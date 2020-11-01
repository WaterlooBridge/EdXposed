cmake_minimum_required(VERSION 3.6)

include_directories(${CMAKE_CURRENT_LIST_DIR})

set(BUILD_SOURCES
        main.cpp
        native_hook/native_hook.cpp
        include/misc.cpp
        include/riru.c
        java_hook/java_hook.cpp
        inject/framework_hook.cpp
        inject/config_manager.cpp
        )

set(LIB_NAME riru_edxposed)

add_library(${LIB_NAME} SHARED ${BUILD_SOURCES})

target_link_libraries(${LIB_NAME} log)