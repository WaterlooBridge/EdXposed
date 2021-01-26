//
// Created by lin on 1/25/21.
//

#include <vector>
#include <dlfcn.h>
#include <sys/system_properties.h>

typedef void *soinfo_t;
typedef uintptr_t addr_t;

static int get_android_system_version() {
    char os_version_str[PROP_VALUE_MAX + 1];
    __system_property_get("ro.build.version.release", os_version_str);
    int os_version_int = atoi(os_version_str);
    return os_version_int;
}

static char *get_android_linker_path() {
#if __LP64__
    if (get_android_system_version() >= 10) {
    return "/apex/com.android.runtime/bin/linker64";
  } else {
    return "/system/bin/linker64";
  }
#else
    if (get_android_system_version() >= 10) {
        return "/apex/com.android.runtime/bin/linker";
    } else {
        return "/system/bin/linker";
    }
#endif
}

void *resolve_elf_internal_symbol(const char *library_name, const char *symbol_name) {
    static void *whaleHandle = NULL;
    static void *libOpenSym = NULL;
    static void *libSymbolSym = NULL;
    static void *libCloseSym = NULL;
    if (!whaleHandle) {
#if __LP64__
        const char *kLibWhalePath = "/system/lib64/libwhale.so";
#else
        const char *kLibWhalePath = "/system/lib/libwhale.so";
#endif
        whaleHandle = dlopen(kLibWhalePath, RTLD_LAZY | RTLD_GLOBAL);
        libOpenSym = dlsym(whaleHandle, "WDynamicLibOpen");
        libSymbolSym = dlsym(whaleHandle, "WDynamicLibSymbol");
        libCloseSym = dlsym(whaleHandle, "WDynamicLibClose");
    }
    void *dlHandle = reinterpret_cast<void *(*)(const char *)>(libOpenSym)(library_name);
    void *symbol = reinterpret_cast<void *(*)(void *, const char *)>(libSymbolSym)(dlHandle,
                                                                                   symbol_name);
    reinterpret_cast<void (*)(void *)>(libCloseSym)(dlHandle);
    return symbol;
}

std::vector<soinfo_t> linker_solist;

std::vector<soinfo_t> linker_get_solist() {
    if (!linker_solist.empty()) {
        linker_solist.clear();
    }

    static soinfo_t (*solist_get_head)() = NULL;
    if (!solist_get_head)
        solist_get_head =
                (soinfo_t(*)()) resolve_elf_internal_symbol(get_android_linker_path(),
                                                            "__dl__Z15solist_get_headv");

    static soinfo_t (*solist_get_somain)() = NULL;
    if (!solist_get_somain)
        solist_get_somain =
                (soinfo_t(*)()) resolve_elf_internal_symbol(get_android_linker_path(),
                                                            "__dl__Z17solist_get_somainv");

    static addr_t *solist_head = NULL;
    if (!solist_head)
        solist_head = (addr_t *) solist_get_head();

    static addr_t somain = 0;
    if (!somain)
        somain = (addr_t) solist_get_somain();

    // Generate the name for an offset.
#define PARAM_OFFSET(type_, member_) __##type_##__##member_##__offset_
#define STRUCT_OFFSET                PARAM_OFFSET
    int STRUCT_OFFSET (solist, next) = 0;
    for (size_t i = 0; i < 1024 / sizeof(void *); i++) {
        if (*(addr_t *) ((addr_t) solist_head + i * sizeof(void *)) == somain) {
            STRUCT_OFFSET(solist, next) = i * sizeof(void *);
            break;
        }
    }

    linker_solist.push_back(solist_head);

    addr_t sonext = 0;
    sonext = *(addr_t *) ((addr_t) solist_head + STRUCT_OFFSET(solist, next));
    while (sonext) {
        linker_solist.push_back((void *) sonext);
        sonext = *(addr_t *) ((addr_t) sonext + STRUCT_OFFSET(solist, next));
    }

    return linker_solist;
}

char *linker_soinfo_get_realpath(soinfo_t soinfo) {
    static char *(*_get_realpath)(soinfo_t) = NULL;
    if (!_get_realpath)
        _get_realpath =
                (char *(*)(soinfo_t)) resolve_elf_internal_symbol(get_android_linker_path(),
                                                                  "__dl__ZNK6soinfo12get_realpathEv");
    return _get_realpath(soinfo);
}