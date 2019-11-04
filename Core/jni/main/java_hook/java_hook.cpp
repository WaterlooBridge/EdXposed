#include <string>
#include <vector>
#include <unistd.h>
#include <mntent.h>
#include <jni.h>
#include <fcntl.h>
#include <dlfcn.h>
#include <inject/config_manager.h>
#include <native_hook/native_hook.h>
#include "java_hook/java_hook.h"
#include "include/logging.h"
#include "include/fd_utils-inl.h"

jobject gInjectDexClassLoader;

static bool isInited = false;

static FileDescriptorTable *gClosedFdTable = nullptr;

void closeFilesBeforeForkNative(JNIEnv *, jclass) {
    gClosedFdTable = FileDescriptorTable::Create();
}

void reopenFilesAfterForkNative(JNIEnv *, jclass) {
    if (!gClosedFdTable) {
        LOGE("gClosedFdTable is null when reopening files");
        return;
    }
    gClosedFdTable->Reopen();
    delete gClosedFdTable;
    gClosedFdTable = nullptr;
}

jobject yahfa_findMethodNative(JNIEnv *env, jclass clazz,
                               jclass targetClass, jstring methodName,
                               jstring methodSig) {
    const char *c_methodName = env->GetStringUTFChars(methodName, nullptr);
    const char *c_methodSig = env->GetStringUTFChars(methodSig, nullptr);
    jobject ret = nullptr;


    //Try both GetMethodID and GetStaticMethodID -- Whatever works :)
    jmethodID method = env->GetMethodID(targetClass, c_methodName, c_methodSig);
    if (!env->ExceptionCheck()) {
        ret = env->ToReflectedMethod(targetClass, method, JNI_FALSE);
    } else {
        env->ExceptionClear();
        method = env->GetStaticMethodID(targetClass, c_methodName, c_methodSig);
        if (!env->ExceptionCheck()) {
            ret = env->ToReflectedMethod(targetClass, method, JNI_TRUE);
        } else {
            env->ExceptionClear();
        }
    }

    env->ReleaseStringUTFChars(methodName, c_methodName);
    env->ReleaseStringUTFChars(methodSig, c_methodSig);
    return ret;
}

jlong suspendAllThreads(JNIEnv *, jclass) {
    if (!suspendAll) {
        return 0;
    }
    ScopedSuspendAll *suspendAllObj = (ScopedSuspendAll *) malloc(sizeof(ScopedSuspendAll));
    suspendAll(suspendAllObj, "edxp_stop_gc", false);
    return reinterpret_cast<jlong>(suspendAllObj);
}

void resumeAllThreads(JNIEnv *, jclass, jlong obj) {
    if (!resumeAll) {
        return;
    }
    resumeAll(reinterpret_cast<ScopedSuspendAll *>(obj));
}

int waitForGcToComplete(JNIEnv *, jclass, jlong thread) {
    // if waitGc succeeded, it should return one of enum collector::GcType
    int gcType = waitGc(0, reinterpret_cast<void *>(thread));
    return gcType;
}

static JNINativeMethod hookMethods[] = {
        {
                "findMethodNative",
                "(Ljava/lang/Class;Ljava/lang/String;Ljava/lang/String;)Ljava/lang/Object;",
                (void *) yahfa_findMethodNative
        },
        {
                "isBlackWhiteListEnabled", "()Z", (void *) is_black_white_list_enabled
        },
        {
                "isDynamicModulesEnabled", "()Z", (void *) is_dynamic_modules_enabled
        },
        {
                "isAppNeedHook", "(Ljava/lang/String;)Z", (void *) is_app_need_hook
        },
        {
                "getInstallerPkgName", "()Ljava/lang/String;", (void *) get_installer_pkg_name
        },
        {
                "closeFilesBeforeForkNative", "()V", (void *) closeFilesBeforeForkNative
        },
        {
                "reopenFilesAfterForkNative", "()V", (void *) reopenFilesAfterForkNative
        },
        {
                "deoptMethodNative", "(Ljava/lang/Object;)V", (void *) deoptimize_method
        },
        {
                "suspendAllThreads", "()J", (void *) suspendAllThreads
        },
        {
                "resumeAllThreads", "(J)V", (void *) resumeAllThreads
        },
        {
                "waitForGcToComplete", "(J)I", (void *) waitForGcToComplete
        }
};

void loadDexAndInit(JNIEnv *env, const char *dexPath) {
    if (isInited) {
        return;
    }
//    install_inline_hooks();
    jclass clzClassLoader = env->FindClass("java/lang/ClassLoader");
    LOGD("java/lang/ClassLoader: %p", clzClassLoader);
    jmethodID mdgetSystemClassLoader = env->GetStaticMethodID(clzClassLoader,
                                                              "getSystemClassLoader",
                                                              "()Ljava/lang/ClassLoader;");
    LOGD("java/lang/ClassLoader.getSystemClassLoader method: %p", mdgetSystemClassLoader);
    jobject systemClassLoader = env->CallStaticObjectMethod(clzClassLoader, mdgetSystemClassLoader);
    LOGD("java/lang/ClassLoader.getSystemClassLoader method result: %p", systemClassLoader);
    if (NULL == systemClassLoader) {
        LOGE("getSystemClassLoader failed!!!");
        return;
    }
    jclass clzPathClassLoader = env->FindClass("dalvik/system/PathClassLoader");
    LOGD("dalvik/system/PathClassLoader: %p", clzClassLoader);
    jmethodID mdinitPathCL = env->GetMethodID(clzPathClassLoader, "<init>",
                                              "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/ClassLoader;)V");
    LOGD("dalvik/system/PathClassLoader.<init>: %p", clzClassLoader);
    jstring jarpath_str = env->NewStringUTF(dexPath);
    jobject myClassLoader = env->NewObject(clzPathClassLoader, mdinitPathCL,
                                           jarpath_str, NULL, systemClassLoader);

    env->DeleteLocalRef(clzClassLoader);
    env->DeleteLocalRef(systemClassLoader);
    env->DeleteLocalRef(clzPathClassLoader);

    if (NULL == myClassLoader) {
        LOGE("PathClassLoader creation failed!!!");
        return;
    }
    gInjectDexClassLoader = env->NewGlobalRef(myClassLoader);
    LOGD("PathClassLoader created: %p", myClassLoader);
    LOGD("PathClassLoader loading dexPath[%s]\n", dexPath);
    jclass entry_class = findClassFromLoader(env, myClassLoader, ENTRY_CLASS_NAME);
    if (NULL != entry_class) {
        LOGD("HookEntry Class %p", entry_class);
        env->RegisterNatives(entry_class, hookMethods, 11);
        isInited = true;
        LOGD("RegisterNatives succeed for HookEntry.");
    } else {
        LOGE("HookEntry class is null. %d", getpid());
    }
}

jstring getThrowableMessage(JNIEnv *env, jobject throwable) {
    if (!throwable) {
        LOGE("throwable is null.");
        return NULL;
    }
    jclass jthrowableClass = env->GetObjectClass(throwable);
    jmethodID getMsgMid = env->GetMethodID(jthrowableClass, "getMessage", "()Ljava/lang/String;");
    if (getMsgMid == 0) {
        LOGE("get Throwable.getMessage method id failed.");
        return NULL;
    }
    return (jstring) env->CallObjectMethod(throwable, getMsgMid);
}

jclass findClassFromLoader(JNIEnv *env, jobject classLoader, const char *className) {
    jclass clz = env->GetObjectClass(classLoader);
    jmethodID mid = env->GetMethodID(clz, "loadClass", "(Ljava/lang/String;)Ljava/lang/Class;");
    if (env->ExceptionOccurred()) {
        LOGE("loadClass method not found");
        env->ExceptionClear();
    } else {
        LOGD("loadClass method %p", mid);
    }
    jclass ret = NULL;
    if (!mid) {
        mid = env->GetMethodID(clz, "findClass", "(Ljava/lang/String;)Ljava/lang/Class;");
        if (env->ExceptionOccurred()) {
            LOGE("findClass method not found");
            env->ExceptionClear();
        } else {
            LOGD("findClass method %p", mid);
        }
    }
    if (mid) {
        jstring className_str = env->NewStringUTF(className);
        jobject tmp = env->CallObjectMethod(classLoader, mid, className_str);
        jthrowable exception = env->ExceptionOccurred();
        if (exception) {
            jstring message = getThrowableMessage(env, exception);
            const char *message_char = env->GetStringUTFChars(message, JNI_FALSE);
            LOGE("Error when findClass %s: %s", className, message_char);
            env->ReleaseStringUTFChars(message, message_char);
            env->ExceptionClear();
        }
        if (NULL != tmp) {
            LOGD("findClassFromLoader %p", tmp);
            ret = (jclass) tmp;
        }
    } else {
        LOGE("no method found");
    }
    if (ret == NULL) {
        LOGE("class %s not found.", className);
    }
    return ret;
}
