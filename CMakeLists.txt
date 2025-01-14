cmake_minimum_required(VERSION 3.27)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_COLOR_DIAGNOSTICS ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_EXTENSIONS OFF)

if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release")
endif()

project(adtlib-test VERSION 0.4 LANGUAGES CXX)

option(OPT_SSE4_2 "compile with SSE4_2 support" ON)

include_directories(BEFORE "src")

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

if (CMAKE_BUILD_TYPE MATCHES "Asan")
    set(CMAKE_BUILD_TYPE "Debug")
    add_compile_options(-O0 -g3 -fno-omit-frame-pointer -fsanitize=undefined -fsanitize=address)
    add_link_options(-fsanitize=undefined -fsanitize=address)
endif()

if (CMAKE_BUILD_TYPE MATCHES "Debug")
    add_compile_definitions("-DDEBUG")
    add_compile_definitions("-DADT_DBG_MEMORY")

    if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
        add_compile_options(/wd4267 /wd4101 /wd4200 /wd4244)
    else()
        add_compile_options(-Wmissing-field-initializers -Wno-unused-parameter -Wno-unused-variable -Wno-unused-function)
    endif()
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
)

add_executable(
    adt-print
    src/printTest.cc
)

add_executable(
    adt-vec
    src/VecTest.cc
)

add_executable(
    adt-heap
    src/HeapTest.cc
)

add_executable(
    adt-string
    src/StringTest.cc
)

add_executable(
    adt-pool
    src/PoolTest.cc
)

add_executable(
    adt-map
    src/MapTest.cc
)

add_executable(
    adt-threadpool
    src/ThreadPoolTest.cc
)

add_executable(
    adt-queue
    src/QueueTest.cc
)

add_executable(
    adt-list
    src/ListTest.cc
)

add_executable(
    adt-rbtree
    src/RBTreeTest.cc
)

add_executable(
    adt-arr
    src/ArrTest.cc
)

add_executable(
    adt-json
    src/JSONTest.cc
    src/json/Parser.cc
    src/json/Lexer.cc
)
