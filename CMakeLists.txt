cmake_minimum_required(VERSION 3.10)
project(networkhook)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Use system libraries (installed via apt)
find_package(CURL REQUIRED)
find_package(OpenSSL REQUIRED)

add_library(networkhook SHARED native_hook.cpp)

target_include_directories(networkhook PRIVATE
    ${CURL_INCLUDE_DIRS}
    ${OPENSSL_INCLUDE_DIR}
)

target_link_libraries(networkhook
    ${CURL_LIBRARIES}
    ${OPENSSL_LIBRARIES}
    log
    dl
    z
)

target_compile_options(networkhook PRIVATE -fPIC -O2 -std=c++17)
