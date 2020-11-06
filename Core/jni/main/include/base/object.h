//
// Created by lin on 20-11-6.
//

#ifndef EDXPOSED_OBJECT_H
#define EDXPOSED_OBJECT_H

#define HOOK_FUNC(func, symbol) \
        if (symbol) { \
            hookFun(symbol, \
                reinterpret_cast<void *>(func##Replace), \
                reinterpret_cast<void **>(&func##Backup)); \
        }

#define CREATE_HOOK_STUB_ENTRIES(ret, func, ...) \
        inline static ret (*func##Backup)(__VA_ARGS__); \
        static ret func##Replace(__VA_ARGS__)

#endif //EDXPOSED_OBJECT_H
