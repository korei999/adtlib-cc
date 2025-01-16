cmake_minimum_required(VERSION 3.27)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_COLOR_DIAGNOSTICS ON)
set(CMAKE_C_STANDARD 17)
set(CMAKE_C_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_EXTENSIONS OFF)

if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release")
endif()

project(adtlib-test VERSION 0.4 LANGUAGES CXX C)

option(OPT_SSE4_2 "" OFF)
option(OPT_MARCH_NATIVE "" OFF)

include_directories(BEFORE "src")
include_directories(BEFORE "mimalloc/include")

add_definitions("-DADT_LOGS")
add_definitions("-DADT_DEFER_LESS_TYPING")
add_definitions("-DADT_LOGS_LESS_TYPING")
# add_definitions("-DADT_STD_TYPES")

add_definitions("-D_DEFAULT_SOURCE")
add_definitions("-DADTLIB_CC_VERSION=${CMAKE_PROJECT_VERSION}")
message(STATUS "CMAKE_PROJECT_VERSION: '${CMAKE_PROJECT_VERSION}'")

if (CMAKE_SYSTEM_NAME MATCHES "Windows")
    add_definitions("-D_CRT_SECURE_NO_WARNINGS")
    add_definitions("-DADT_STD_TYPES")
endif()

if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    add_compile_options(/utf-8)
else()
    add_compile_options(-Wall -Wextra -Wswitch -Wimplicit-fallthrough)
endif()

if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    add_compile_options(-Wno-class-memaccess)
endif()

if (OPT_SSE4_2)
    add_definitions("-DADT_SSE4_2")

    if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
        add_compile_options(/arch:SSE4.2)
    else()
        add_compile_options(-msse4.2)
    endif()
endif()

if (OPT_MARCH_NATIVE)
    add_definitions("-DADT_MARCH_NATIVE")
    add_compile_options(-march=native)
endif()

if (CMAKE_BUILD_TYPE MATCHES "Asan")
    set(CMAKE_BUILD_TYPE "Debug")
    add_compile_options(-O0 -g3 -fno-omit-frame-pointer -fsanitize=undefined -fsanitize=address)
    add_link_options(-fsanitize=undefined -fsanitize=address)
    add_definitions("-DMI_TRACK_ASAN=ON")
    add_definitions("-DMI_TRACK_VALGRIND=ON")
endif()

if (CMAKE_BUILD_TYPE MATCHES "Debug")
    add_compile_definitions("-DDEBUG")
    add_compile_definitions("-DADT_DBG_MEMORY")

    if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
        add_compile_options(/wd4267 /wd4101 /wd4200 /wd4244)
    else()
        add_compile_options(-Wmissing-field-initializers -Wno-unused-parameter -Wno-unused-variable -Wno-unused-function)
    endif()
else()
    if (NOT CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
        add_compile_options(-flto)
        add_link_options(-flto)
    else()
    endif()

    add_definitions("-DADT_USE_MIMALLOC")
endif()

cmake_host_system_information(RESULT OS_NAME QUERY OS_NAME)

message(STATUS "OS_NAME: '${OS_NAME}'")
message(STATUS "CMAKE_CXX_COMPILER_ID: '${CMAKE_CXX_COMPILER_ID}'")
message(STATUS "CMAKE_BUILD_TYPE: '${CMAKE_BUILD_TYPE}'")

# find_package(PkgConfig REQUIRED)
# pkg_check_modules(LINUX_PKGS REQUIRED mimalloc)
# include_directories(BEFORE ${LINUX_PKGS_INCLUDE_DIRS})
# target_link_libraries(
#     adt-vec PRIVATE
#     ${LINUX_PKGS_LIBRARIES}
# )

add_executable(
    adt-alloc
    src/allocTest.cc
    mimalloc/src/static.c
)

add_executable(
    adt-print
    src/printTest.cc
    mimalloc/src/static.c
)

add_executable(
    adt-vec
    src/VecTest.cc
    mimalloc/src/static.c
)

add_executable(
    adt-heap
    src/HeapTest.cc
    mimalloc/src/static.c
)

add_executable(
    adt-string
    src/StringTest.cc
    mimalloc/src/static.c
)

add_executable(
    adt-pool
    src/PoolTest.cc
    mimalloc/src/static.c
)

add_executable(
    adt-map
    src/MapTest.cc
    mimalloc/src/static.c
)

add_executable(
    adt-threadpool
    src/ThreadPoolTest.cc
    mimalloc/src/static.c
)

add_executable(
    adt-queue
    src/QueueTest.cc
    mimalloc/src/static.c
)

add_executable(
    adt-list
    src/ListTest.cc
    mimalloc/src/static.c
)

add_executable(
    adt-rbtree
    src/RBTreeTest.cc
    mimalloc/src/static.c
)

add_executable(
    adt-arr
    src/ArrTest.cc
    mimalloc/src/static.c
)

add_executable(
    adt-json
    src/JSONTest.cc
    src/json/Parser.cc
    src/json/Lexer.cc
    mimalloc/src/static.c
)
