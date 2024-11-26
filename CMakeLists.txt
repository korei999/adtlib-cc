cmake_minimum_required(VERSION 3.8)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_COLOR_DIAGNOSTICS ON)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_EXTENSIONS OFF)

if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release")
endif()

project(adtlib-test VERSION 0.2 LANGUAGES CXX)

# if (NOT CMAKE_BUILD_TYPE MATCHES "Asan")
#     set(BUILD_SHARED_LIBS OFF)
#     set(CMAKE_EXE_LINKER_FLAGS "-static")
# endif()

add_executable(
    ${CMAKE_PROJECT_NAME}
    src/main.cc
    src/json/Lexer.cc
    src/json/Parser.cc
)

add_definitions("-DADT_LOGS")
add_definitions("-DADT_DEFER_LESS_TYPING")
add_definitions("-DADT_LOGS_LESS_TYPING")
# add_definitions("-DADT_STD_TYPES")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-exceptions")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wswitch -Wimplicit-fallthrough")
add_definitions("-D_DEFAULT_SOURCE")
add_definitions("-DADTLIB_CC_VERSION=${CMAKE_PROJECT_VERSION}")
message(STATUS "CMAKE_PROJECT_VERSION: '${CMAKE_PROJECT_VERSION}'")
include_directories(BEFORE "src")

if (CMAKE_BUILD_TYPE MATCHES "Asan")
    set(CMAKE_BUILD_TYPE "Debug")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=undefined -fsanitize=address")
    add_link_options(-fsanitize=undefined -fsanitize=address)
endif()

if (CMAKE_BUILD_TYPE MATCHES "Debug")
    add_compile_definitions("-DDEBUG")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wmissing-field-initializers -Wno-unused-parameter -Wno-unused-variable -Wno-unused-function")
endif()

cmake_host_system_information(RESULT OS_NAME QUERY OS_NAME)
message(STATUS "OS_NAME: '${OS_NAME}'")

message(STATUS "CMAKE_BUILD_TYPE: '${CMAKE_BUILD_TYPE}'")

# find_package(PkgConfig REQUIRED)
# pkg_check_modules(LINUX_PKGS REQUIRED mimalloc)
# include_directories(BEFORE ${LINUX_PKGS_INCLUDE_DIRS})
# target_link_libraries(
#     ${CMAKE_PROJECT_NAME} PRIVATE
#     ${LINUX_PKGS_LIBRARIES}
# )
