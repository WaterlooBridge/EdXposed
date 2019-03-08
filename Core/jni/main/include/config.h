#include <jni.h>
#include <sys/types.h>

#ifndef CONFIG_H
#define CONFIG_H

//#define LOG_DISABLED
//#define DEBUG

#define INJECT_DEX_PATH \
"/system/framework/edxposed.dex"

#define ENTRY_CLASS_NAME "com.zhenl.riru.xposed.Main"

#endif //CONFIG_H