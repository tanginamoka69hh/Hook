cmake_minimum_required(VERSION 3.10)
project(networkhook)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Prebuilt libraries will be in libs/arm64-v8a/
set(LIB_DIR ${CMAKE_SOURCE_DIR}/libs/${ANDROID_ABI})
set(INC_DIR ${CMAKE_SOURCE_DIR}/include)

add_library(networkhook SHARED native_hook.cpp)

target_include_directories(networkhook PRIVATE ${INC_DIR})

target_link_libraries(networkhook
    ${LIB_DIR}/libdobby.a
    ${LIB_DIR}/libcurl.a
    ${LIB_DIR}/libssl.a
    ${LIB_DIR}/libcrypto.a
    ${LIB_DIR}/libnghttp2.a
    ${LIB_DIR}/libbrotli.a
    ${LIB_DIR}/libz.a
    log
    dl
    z
)

target_compile_options(networkhook PRIVATE -fPIC -O2 -std=c++17)
