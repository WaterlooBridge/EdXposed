//
// Created by lin on 20-11-6.
//

#ifndef EDXPOSED_OAT_FILE_MANAGER_H
#define EDXPOSED_OAT_FILE_MANAGER_H

#include <dlfcn.h>

#include "../base/object.h"
#include "../android_build.h"

namespace art {

    namespace oat_file_manager {

        CREATE_HOOK_STUB_ENTRIES(void, SetOnlyUseSystemOatFiles) {
            return;
        }

        static void
        DisableOnlyUseSystemOatFiles(void *handle, void (*hookFun)(void *, void *, void **)) {
            const int api_level = GetAndroidApiLevel();
            if (api_level >= ANDROID_R) {
                void *symbol = dlsym(handle, "_ZN3art14OatFileManager24SetOnlyUseSystemOatFilesEv");
                HOOK_FUNC(SetOnlyUseSystemOatFiles, symbol)
            } else if (api_level >= ANDROID_Q) {
                void *symbol = dlsym(handle,
                                     "_ZN3art14OatFileManager24SetOnlyUseSystemOatFilesEbb");
                HOOK_FUNC(SetOnlyUseSystemOatFiles, symbol)
            } else if (api_level >= ANDROID_P) {
                void *symbol = dlsym(handle, "_ZN3art14OatFileManager24SetOnlyUseSystemOatFilesEv");
                HOOK_FUNC(SetOnlyUseSystemOatFiles, symbol)
            }
        }

    }

}

#endif //EDXPOSED_OAT_FILE_MANAGER_H
