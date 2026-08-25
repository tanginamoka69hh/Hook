cmake_minimum_required(VERSION 3.10)
project(networkhook)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Dependencies: lahat ng ito ay bubuuin sa workflow
set(LIB_DIR ${CMAKE_SOURCE_DIR}/libs/${ANDROID_ABI})
set(INCLUDE_DIR ${CMAKE_SOURCE_DIR}/include)

add_library(networkhook SHARED native_hook.cpp)

target_include_directories(networkhook PRIVATE
    ${INCLUDE_DIR}
    ${INCLUDE_DIR}/nghttp2
    ${INCLUDE_DIR}/brotli
)

target_link_libraries(networkhook
    ${LIB_DIR}/libcurl.a
    ${LIB_DIR}/libssl.a
    ${LIB_DIR}/libcrypto.a
    ${LIB_DIR}/libnghttp2.a
    ${LIB_DIR}/libbrotlidec.a
    ${LIB_DIR}/libbrotlienc.a
    ${LIB_DIR}/libbrotlicommon.a
    ${LIB_DIR}/libz.a
    log
    dl
    z
)

target_compile_options(networkhook PRIVATE -fPIC -O2 -std=c++17)
