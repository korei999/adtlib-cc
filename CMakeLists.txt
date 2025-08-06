cmake_minimum_required(VERSION 3.20)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_COLOR_DIAGNOSTICS ON)
set(CMAKE_C_STANDARD 17)
set(CMAKE_C_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_EXTENSIONS OFF)

if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release")
endif()

project(adtlib-test VERSION 0.7 LANGUAGES CXX C)

option(OPT_SSE4_2 "" ON)
option(OPT_AVX2 "" OFF)
option(OPT_MARCH_NATIVE "" OFF)

include_directories(BEFORE "src")
include_directories(BEFORE "include")

add_definitions("-DADT_LOGS")
add_definitions("-DADT_LOGS_LESS_TYPING")
add_definitions("-DADT_DEFER_LESS_TYPING")
# add_definitions("-DADT_STD_TYPES")

add_definitions("-D_DEFAULT_SOURCE")
add_definitions("-DADTLIB_CC_VERSION=${CMAKE_PROJECT_VERSION}")
message(STATUS "CMAKE_PROJECT_VERSION: '${CMAKE_PROJECT_VERSION}'")

if (CMAKE_BUILD_TYPE MATCHES "DebugMingw" OR CMAKE_BUILD_TYPE MATCHES "ReleaseMingw")
    set(TOOLCHAIN_PREFIX x86_64-w64-mingw32)
    set(CMAKE_C_COMPILER ${TOOLCHAIN_PREFIX}-gcc)
    set(CMAKE_CXX_COMPILER ${TOOLCHAIN_PREFIX}-g++)

    set(CMAKE_FIND_ROOT_PATH /usr/${TOOLCHAIN_PREFIX})
    set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
    set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
    set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

    set(CMAKE_SYSTEM_NAME "Windows")

    if (CMAKE_BUILD_TYPE MATCHES "DebugMingw")
        set(CMAKE_BUILD_TYPE "Debug")
    else()
        set(CMAKE_BUILD_TYPE "Release")
    endif()
endif()

if (CMAKE_SYSTEM_NAME MATCHES "Windows")
    add_definitions("-D_CRT_SECURE_NO_WARNINGS")
    add_definitions("-DADT_STD_TYPES")
    add_definitions("-D_USE_MATH_DEFINES")
endif()

if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    add_compile_options(/Zc:preprocessor /EHsc /utf-8)
else()
    add_compile_options(-Wall -Wextra -Wswitch -Wimplicit-fallthrough -Wno-missing-braces)
endif()

if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-cast-function-type")
endif()

if (OPT_SSE4_2)
    add_definitions("-DADT_SSE4_2")

    if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
        add_compile_options(/arch:SSE4.2)
    else()
        add_compile_options(-msse4.2)
    endif()
endif()

if (OPT_AVX2)
    add_definitions("-DADT_AVX2")

    if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
        add_compile_options(/arch:AVX2)
    else()
        add_compile_options(-mavx2 -mfma)
    endif()
endif()

if (OPT_MARCH_NATIVE)
    add_definitions("-DADT_MARCH_NATIVE")
    if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    else()
        add_compile_options(-march=native)
    endif()
endif()

if (CMAKE_BUILD_TYPE MATCHES "Asan")
    set(CMAKE_BUILD_TYPE "Debug")
    add_compile_options(-O0 -g3 -fno-omit-frame-pointer -fsanitize=undefined -fsanitize=address)
    add_link_options(-fsanitize=undefined -fsanitize=address)

    add_definitions("-DMI_TRACK_ASAN=ON")
    add_definitions("-DMI_TRACK_VALGRIND=ON")
endif()

if (CMAKE_BUILD_TYPE MATCHES "Debug")
    add_compile_definitions("-DADT_DBG_MEMORY")

    if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
        add_compile_options(/wd4267 /wd4101 /wd4200 /wd4244)
    else()
        add_compile_options(-Wmissing-field-initializers -Wno-unused-parameter -Wno-unused-variable -Wno-unused-function -Wno-unused-but-set-variable)
    endif()
else()
    if (NOT CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
        add_compile_options(-flto)
        add_link_options(-flto)
    else()
    endif()

    # add_definitions("-DADT_USE_MIMALLOC")
endif()

cmake_host_system_information(RESULT OS_NAME QUERY OS_NAME)

message(STATUS "OS_NAME: '${OS_NAME}'")
message(STATUS "CMAKE_CXX_COMPILER_ID: '${CMAKE_CXX_COMPILER_ID}'")
message(STATUS "CMAKE_BUILD_TYPE: '${CMAKE_BUILD_TYPE}'")

# find_package(PkgConfig REQUIRED)
#
# pkg_check_modules(TBB REQUIRED tbb)
# include_directories(BEFORE ${TBB_INCLUDE_DIRS})
# link_directories(${TBB_LIBRARY_DIRS})
# link_libraries(${TBB_LIBRARIES})

# add_link_options(-lfmt)
# add_link_options(-lmimalloc)
# include(FetchContent)
# 
# FetchContent_Declare(
#     MIMALLOC
#     GIT_REPOSITORY https://github.com/microsoft/mimalloc.git
#     GIT_TAG master
# )
# FetchContent_MakeAvailable(MIMALLOC)
# FetchContent_GetProperties(MIMALLOC
#     SOURCE_DIR MIMALLOC_SOURCE_DIR
# )
# 
# include_directories(${MIMALLOC_SOURCE_DIR}/include)
# message(STATUS "MIMALLOC_SOURCE_DIR: '${MIMALLOC_SOURCE_DIR}'")

add_subdirectory(src)
