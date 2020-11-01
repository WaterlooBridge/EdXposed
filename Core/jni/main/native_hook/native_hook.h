#ifndef HOOK_H
#define HOOK_H

#if defined(__LP64__)
static constexpr const char *kLibArtPath = "/system/lib64/libart.so";
static constexpr const char *kLibWhalePath = "/system/lib64/libwhale.so";
static constexpr const char *kLibDlPath = "/system/lib64/libdl.so";
static constexpr const char *kLinkerPath = "/apex/com.android.runtime/bin/linker64";
#else
static constexpr const char *kLibArtPath = "/system/lib/libart.so";
static constexpr const char *kLibWhalePath = "/system/lib/libwhale.so";
static constexpr const char *kLibDlPath = "/system/lib/libdl.so";
static constexpr const char *kLinkerPath = "/apex/com.android.runtime/bin/linker";
#endif

class ScopedSuspendAll {
};

extern void (*suspendAll)(ScopedSuspendAll *, const char *, bool);

extern void (*resumeAll)(ScopedSuspendAll *);

extern int waitGc(int, void *);

void install_inline_hooks();

void deoptimize_method(JNIEnv *env, jclass clazz, jobject method);

#endif // HOOK_H
