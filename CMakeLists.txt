cmake_minimum_required(VERSION 3.10)
project(networkhook)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(DOBBY_LIB ${CMAKE_SOURCE_DIR}/libs/${ANDROID_ABI}/libdobby.a)

add_library(networkhook SHARED native_hook.cpp)

target_include_directories(networkhook PRIVATE ${CMAKE_SOURCE_DIR}/include)

target_link_libraries(networkhook
    ${DOBBY_LIB}
    curl
    ssl
    crypto
    log
    dl
    z
)

target_link_options(networkhook PRIVATE
    -Wl,--whole-archive ${DOBBY_LIB} -Wl,--no-whole-archive
    -static-libstdc++ -static-libgcc
)

target_compile_options(networkhook PRIVATE -fPIC -O2 -std=c++14)
