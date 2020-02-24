#include <cstdio>
#include <unistd.h>
#include <fcntl.h>
#include <jni.h>
#include <cstring>
#include <cstdlib>
#include <sys/mman.h>
#include <array>
#include <thread>
#include <vector>
#include <utility>
#include <string>
#include <inject/framework_hook.h>
#include <native_hook/native_hook.h>

#include "include/logging.h"
#include "include/misc.h"

#include "include/config.h"

extern "C" {

#define EXPORT __attribute__((visibility("default"))) __attribute__((used))

EXPORT void onModuleLoaded() {
    LOGI("onModuleLoaded: welcome to EdXposed!");
    install_inline_hooks();
}

EXPORT int shouldSkipUid(int uid) {
    return 0;
}

EXPORT void nativeForkAndSpecializePre(
        JNIEnv *env, jclass clazz, jint *_uid, jint *gid, jintArray *gids, jint *runtimeFlags,
        jobjectArray *rlimits, jint *mountExternal, jstring *seInfo, jstring *niceName,
        jintArray *fdsToClose, jintArray *fdsToIgnore, jboolean *is_child_zygote,
        jstring *instructionSet, jstring *appDataDir, jstring *packageName,
        jobjectArray *packagesForUID, jstring *sandboxId) {
    // packageName, packagesForUID, sandboxId are added from Android Q beta 2, removed from beta 5
    onNativeForkAndSpecializePre(env, clazz, *_uid, *gid, *gids, *runtimeFlags, *rlimits,
                                 *mountExternal, *seInfo, *niceName, *fdsToClose, *fdsToIgnore,
                                 *is_child_zygote, *instructionSet, *appDataDir);
}

EXPORT int nativeForkAndSpecializePost(JNIEnv *env, jclass clazz, jint res) {
    return onNativeForkAndSpecializePost(env, clazz, res);
}

EXPORT void nativeForkSystemServerPre(
        JNIEnv *env, jclass clazz, uid_t *uid, gid_t *gid, jintArray *gids, jint *runtimeFlags,
        jobjectArray *rlimits, jlong *permittedCapabilities, jlong *effectiveCapabilities) {
    onNativeForkSystemServerPre(env, clazz, *uid, *gid, *gids, *runtimeFlags, *rlimits,
                                *permittedCapabilities, *effectiveCapabilities);
}

EXPORT int nativeForkSystemServerPost(JNIEnv *env, jclass clazz, jint res) {
    return onNativeForkSystemServerPost(env, clazz, res);
}

// this is added from Android Q beta, but seems Google disabled this in following updates
EXPORT void specializeAppProcessPre(JNIEnv *env, jclass clazz, jint *uid, jint *gid,
                                    jintArray *gids, jint *runtime_flags, jobjectArray *rlimits,
                                    jint *mount_external, jstring *se_info, jstring *nice_name,
                                    jboolean *start_child_zygote, jstring *instruction_set,
                                    jstring *app_data_dir, jstring *package_name,
                                    jobjectArray *packages_for_uid, jstring *sandbox_id) {
    // packageName, packagesForUID, sandboxId are added from Android Q beta 2, removed from beta 5
    onNativeForkAndSpecializePre(env, clazz, *uid, *gid, *gids, *runtime_flags, *rlimits,
                                 *mount_external, *se_info, *nice_name, nullptr, nullptr,
                                 *start_child_zygote, *instruction_set, *app_data_dir);
}

EXPORT int specializeAppProcessPost(JNIEnv *env, jclass clazz) {
    return onNativeForkAndSpecializePost(env, clazz, 0);
}

}
