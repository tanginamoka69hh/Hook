cmake_minimum_required(VERSION 3.22)

project(networkhook LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

if(NOT ANDROID)
    message(FATAL_ERROR "This project must be built with the Android NDK toolchain.")
endif()

if(NOT ANDROID_ABI)
    set(ANDROID_ABI arm64-v8a)
endif()

set(ROOT_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
set(LIB_DIR  "${ROOT_DIR}/libs/${ANDROID_ABI}")
set(INC_DIR  "${ROOT_DIR}/include")

add_library(networkhook SHARED
    native_hook.cpp
)

target_include_directories(networkhook PRIVATE
    "${INC_DIR}"
)

# IMPORTANT:
# Do not add /usr/include or /usr/include/x86_64-linux-gnu.
# Android NDK provides the correct Android headers.

target_compile_options(networkhook PRIVATE
    -O2
    -fPIC
    -ffunction-sections
    -fdata-sections
)

target_link_options(networkhook PRIVATE
    -Wl,--gc-sections
)

target_link_libraries(networkhook
    "${LIB_DIR}/libcurl.a"
    "${LIB_DIR}/libssl.a"
    "${LIB_DIR}/libcrypto.a"
    "${LIB_DIR}/libdobby.a"

    log
    dl
)

set_target_properties(networkhook PROPERTIES
    OUTPUT_NAME "networkhook"
    PREFIX "lib"
)
