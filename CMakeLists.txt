cmake_minimum_required(VERSION 3.10)
project(networkhook)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Use prebuilt libraries (downloaded in build.yml)
set(CURL_LIB ${CMAKE_SOURCE_DIR}/libs/${ANDROID_ABI}/libcurl.a)
set(SSL_LIB ${CMAKE_SOURCE_DIR}/libs/${ANDROID_ABI}/libssl.a)
set(CRYPTO_LIB ${CMAKE_SOURCE_DIR}/libs/${ANDROID_ABI}/libcrypto.a)

add_library(networkhook SHARED native_hook.cpp)

# Add include directories
target_include_directories(networkhook PRIVATE
    ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(networkhook
    ${CURL_LIB}
    ${SSL_LIB}
    ${CRYPTO_LIB}
    log
    dl
    z
)

target_compile_options(networkhook PRIVATE -fPIC -O2 -std=c++17)
