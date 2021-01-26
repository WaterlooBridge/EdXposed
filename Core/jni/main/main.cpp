#include <jni.h>
#include <sys/types.h>
#include <malloc.h>
#include <cstring>

#include "include/riru.h"
#include "include/logging.h"
#include "inject/framework_hook.h"
#include "native_hook/native_hook.h"

static void forkAndSpecializePre(
        JNIEnv *env, jclass clazz, jint *_uid, jint *gid, jintArray *gids, jint *runtimeFlags,
        jobjectArray *rlimits, jint *mountExternal, jstring *seInfo, jstring *niceName,
        jintArray *fdsToClose, jintArray *fdsToIgnore, jboolean *is_child_zygote,
        jstring *instructionSet, jstring *appDataDir, jboolean *isTopApp, jobjectArray *pkgDataInfoList,
        jobjectArray *whitelistedDataInfoList, jboolean *bindMountAppDataDirs, jboolean *bindMountAppStorageDirs) {
    onNativeForkAndSpecializePre(env, clazz, *_uid, *gid, *gids, *runtimeFlags, *rlimits,
                                 *mountExternal, *seInfo, *niceName, *fdsToClose, *fdsToIgnore,
                                 *is_child_zygote, *instructionSet, *appDataDir);
}

static void forkAndSpecializePost(JNIEnv *env, jclass clazz, jint res) {
    onNativeForkAndSpecializePost(env, clazz, res);
}

static void specializeAppProcessPre(
        JNIEnv *env, jclass clazz, jint *_uid, jint *gid, jintArray *gids, jint *runtimeFlags,
        jobjectArray *rlimits, jint *mountExternal, jstring *seInfo, jstring *niceName,
        jboolean *startChildZygote, jstring *instructionSet, jstring *appDataDir,
        jboolean *isTopApp, jobjectArray *pkgDataInfoList, jobjectArray *whitelistedDataInfoList,
        jboolean *bindMountAppDataDirs, jboolean *bindMountAppStorageDirs) {
    // added from Android 10, but disabled at least in Google Pixel devices
    onNativeForkAndSpecializePre(env, clazz, *_uid, *gid, *gids, *runtimeFlags, *rlimits,
                                 *mountExternal, *seInfo, *niceName, nullptr, nullptr,
                                 *startChildZygote, *instructionSet, *appDataDir);
}

static void specializeAppProcessPost(
        JNIEnv *env, jclass clazz) {
    // added from Android 10, but disabled at least in Google Pixel devices
    onNativeForkAndSpecializePost(env, clazz, 0);
}

static void forkSystemServerPre(
        JNIEnv *env, jclass clazz, uid_t *uid, gid_t *gid, jintArray *gids, jint *runtimeFlags,
        jobjectArray *rlimits, jlong *permittedCapabilities, jlong *effectiveCapabilities) {
    onNativeForkSystemServerPre(env, clazz, *uid, *gid, *gids, *runtimeFlags, *rlimits,
                                *permittedCapabilities, *effectiveCapabilities);
}

static void forkSystemServerPost(JNIEnv *env, jclass clazz, jint res) {
    onNativeForkSystemServerPost(env, clazz, res);
}

static int shouldSkipUid(int uid) {
    // by default, Riru only call module functions in "normal app processes" (10000 <= uid % 100000 <= 19999)
    // false = don't skip
    return false;
}

static void onModuleLoaded() {
    // called when the shared library of Riru core is loaded
    LOGI("onModuleLoaded: welcome to EdXposed!");
    install_inline_hooks();
}

extern "C" {

int riru_api_version;
RiruApiV9 *riru_api_v9;

/*
 * Init will be called three times.
 *
 * The first time:
 *   Returns the highest version number supported by both Riru and the module.
 *
 *   arg: (int *) Riru's API version
 *   returns: (int *) the highest possible API version
 *
 * The second time:
 *   Returns the RiruModuleX struct created by the module.
 *   (X is the return of the first call)
 *
 *   arg: (RiruApiVX *) RiruApi strcut, this pointer can be saved for further use
 *   returns: (RiruModuleX *) RiruModule strcut
 *
 * The second time:
 *   Let the module to cleanup (such as RiruModuleX struct created before).
 *
 *   arg: null
 *   returns: (ignored)
 *
 */
void *init(void *arg) {
    static int step = 0;
    step += 1;

    static void *_module;

    switch (step) {
        case 1: {
            auto core_max_api_version = *(int *) arg;
            riru_api_version = core_max_api_version <= RIRU_MODULE_API_VERSION ? core_max_api_version : RIRU_MODULE_API_VERSION;
            return &riru_api_version;
        }
        case 2: {
            switch (riru_api_version) {
                // RiruApiV10 and RiruModuleInfoV10 are equal to V9
                case 10:
                case 9: {
                    riru_api_v9 = (RiruApiV9 *) arg;

                    auto module = (RiruModuleInfoV9 *) malloc(sizeof(RiruModuleInfoV9));
                    memset(module, 0, sizeof(RiruModuleInfoV9));
                    _module = module;

                    module->supportHide = true;

                    module->version = RIRU_MODULE_VERSION;
                    module->versionName = RIRU_MODULE_VERSION_NAME;
                    module->onModuleLoaded = onModuleLoaded;
                    module->shouldSkipUid = shouldSkipUid;
                    module->forkAndSpecializePre = forkAndSpecializePre;
                    module->forkAndSpecializePost = forkAndSpecializePost;
                    module->specializeAppProcessPre = specializeAppProcessPre;
                    module->specializeAppProcessPost = specializeAppProcessPost;
                    module->forkSystemServerPre = forkSystemServerPre;
                    module->forkSystemServerPost = forkSystemServerPost;
                    return module;
                }
                default: {
                    return nullptr;
                }
            }
        }
        case 3: {
            free(_module);
            return nullptr;
        }
        default: {
            return nullptr;
        }
    }
}
}