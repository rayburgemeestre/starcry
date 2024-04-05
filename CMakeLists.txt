cmake_minimum_required(VERSION 3.10.2)

find_program(CCACHE_PROGRAM ccache)
if(CCACHE_PROGRAM)
    set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE "${CCACHE_PROGRAM}")
endif()

set(CMAKE_CXX_STANDARD 23)

project(starcry VERSION 0.3 LANGUAGES CXX)

list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

set(CLANGFORMAT_EXECUTABLE clang-format-15)
# clang-tidy makes compilation significantly slower
# set(CMAKE_CXX_CLANG_TIDY clang-tidy-15)
set(Boost_USE_STATIC_LIBS ON)
set(BOOST_ROOT "/opt/cppse/build/boost/")
if(CMAKE_BUILD_TYPE MATCHES Debug)
    set(DEBUG on)
endif()
include(warnings)
# set_warnings()
include(ClangFormat)
include(StarcryIncludes)
include(StarcryLinking)
add_subdirectory(libs/Catch2)

find_package(V8 REQUIRED)
find_package(Boost 1.52 COMPONENTS program_options system date_time thread chrono REQUIRED)
find_package(Threads)

# CLion 2023.3 doesn't like c++23 here?
#set(COMMON_FLAGS "-std=c++23 -fno-omit-frame-pointer -Wall -mssse3 -DV8_COMPRESS_POINTERS -DV8_ENABLE_SANDBOX")
set(COMMON_FLAGS "-std=c++20 -fno-omit-frame-pointer -Wall -mssse3 -DV8_COMPRESS_POINTERS -DV8_ENABLE_SANDBOX")
set(DEBUG_FLAGS "-O0 -g")
set(RELEASE_FLAGS "-O2 -funsigned-char -pedantic -pthread")  # TODO: support --ffast-math

# Determine the build type and compiler
set(BUILD_TYPE "RELEASE")
set(COMPILER_TYPE "GCC")

if (DEBUG)
    set(BUILD_TYPE "DEBUG")
    set(BUILD_TYPE_FLAGS ${DEBUG_FLAGS})
else()
    set(BUILD_TYPE_FLAGS ${RELEASE_FLAGS})
endif()

if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    set(COMPILER_TYPE "Clang")
    if (DEBUG)
        set(COMPILER_SPECIFIC_FLAGS "-stdlib=libstdc++ -gdwarf-4")  # -gdwarf-4 is needed for valgrind, version 5 seems unsupported.
    else()
        set(COMPILER_SPECIFIC_FLAGS "-stdlib=libstdc++ -Wno-reserved-user-defined-literal")
    endif()
else()
    if (DEBUG)
        set(COMPILER_SPECIFIC_FLAGS "-Wno-unknown-pragmas -Wno-literal-suffix")
    else()
        # removed -flto, was a bit too slow for my taste
        set(COMPILER_SPECIFIC_FLAGS "-Wno-unknown-pragmas -Wno-literal-suffix")
    endif()
endif()

message(STATUS "This is a ${BUILD_TYPE} ${COMPILER_TYPE} build.")

add_definitions("${COMMON_FLAGS} ${BUILD_TYPE_FLAGS} ${COMPILER_SPECIFIC_FLAGS}")
if (SANITIZER)
    add_definitions("-fsanitize=address")
endif()

# starcry
file(GLOB_RECURSE starcry_SOURCES "src/**.cpp" "src/**.hpp"  "src/**.h" "/opt/cppse/build/v8pp/include/v8pp/*.cpp" "libs/piper/src/**")
list(REMOVE_ITEM starcry_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/client.cpp")
add_executable(starcry ${starcry_SOURCES})
configure_links(starcry)
if (SANITIZER)
    target_link_libraries(starcry PRIVATE -static-libgcc -static-libstdc++ /usr/lib/llvm-15/lib/clang/15.0.0/lib/linux/libclang_rt.asan-x86_64.a)
else()
    target_link_libraries(starcry PRIVATE -static-libgcc -static-libstdc++)
endif()
install (TARGETS starcry DESTINATION bin)

# sc_client (starcry debug client)
file(GLOB_RECURSE sc_client_SOURCES "src/client.cpp" "src/perlin.cpp" "src/texture.cpp" "src/stb.cpp" "src/util/logger.cpp")
add_executable(sc_client ${sc_client_SOURCES})
target_compile_definitions(sc_client PUBLIC SC_CLIENT)
target_link_libraries(sc_client PRIVATE -lSDL2)
target_link_libraries(sc_client PRIVATE /opt/cppse/build/vivid/lib/libvivid.a)
target_link_libraries(sc_client PRIVATE /opt/cppse/build/fmt/lib/libfmt.a)
configure_links(sc_client)

# unit tests
add_executable(tests test/test_unit.cpp)
target_link_libraries(tests PRIVATE /opt/cppse/build/v8pp/lib/libv8_monolith.a)
target_link_libraries(tests PRIVATE /opt/cppse/build/vivid/lib/libvivid.a)
target_link_libraries(tests PRIVATE Catch2::Catch2WithMain)

# integration tests
file(GLOB_RECURSE integration_tests_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/test/test_integration.cpp")
list(APPEND integration_tests_SOURCES ${starcry_SOURCES})
list(APPEND integration_tests_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/test/test_integration.cpp")
list(REMOVE_ITEM integration_tests_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp")
add_executable(integration_tests ${integration_tests_SOURCES})
target_link_libraries(integration_tests PRIVATE Catch2::Catch2WithMain)
target_link_libraries(integration_tests PRIVATE -static-libgcc -static-libstdc++)
configure_links(integration_tests)

# clang format
set(all_sources ${starcry_SOURCES})
list(APPEND all_sources ${sc_client_SOURCES})
list(APPEND all_sources ${integration_tests_SOURCES})
list(REMOVE_ITEM all_sources /opt/cppse/build/v8pp/include/v8pp/context.cpp)  # causes crash since owned by root
list(REMOVE_DUPLICATES all_sources)
clangformat_setup(${all_sources})