cmake_minimum_required(VERSION 3.21)

# Set modern CMake policies
cmake_policy(SET CMP0077 NEW)  # Use absolute paths in target_sources
cmake_policy(SET CMP0135 NEW)  # Use file(GLOB) with relative paths
cmake_policy(SET CMP0144 NEW)  # Use upper-case <PACKAGENAME>_ROOT variables
#cmake_policy(SET CMP0167 NEW)  # Use Boost::boost instead of FindBoost

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

find_program(CCACHE_PROGRAM ccache)
if(CCACHE_PROGRAM)
    set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE "${CCACHE_PROGRAM}")
endif()

project(starcry VERSION 0.3 LANGUAGES CXX)

list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

set(CLANGFORMAT_EXECUTABLE clang-format-19)

set(Boost_USE_STATIC_LIBS ON)
set(BOOST_ROOT "/opt/cppse/build/boost/")

include(warnings)
include(ClangFormat)
include(StarcryIncludes)
include(StarcryLinking)

add_subdirectory(libs/Catch2)

find_package(V8 REQUIRED)
find_package(Boost 1.52 COMPONENTS program_options system date_time thread chrono REQUIRED)
find_package(Threads REQUIRED)

# Set common compiler flags
set(COMMON_FLAGS -fno-omit-frame-pointer -Wall -mssse3 -DV8_COMPRESS_POINTERS -DV8_ENABLE_SANDBOX)
set(DEBUG_FLAGS -O0 -g)
set(RELEASE_FLAGS -O2 -funsigned-char -pedantic -pthread)

# Determine build type and compiler
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
        set(COMPILER_SPECIFIC_FLAGS -stdlib=libstdc++ -gdwarf-4)
    else()
        set(COMPILER_SPECIFIC_FLAGS -stdlib=libstdc++ -Wno-reserved-user-defined-literal)
    endif()
else()
    if (DEBUG)
        set(COMPILER_SPECIFIC_FLAGS -Wno-unknown-pragmas -Wno-literal-suffix)
    else()
        set(COMPILER_SPECIFIC_FLAGS -flto -Wno-unknown-pragmas -Wno-literal-suffix)
    endif()
endif()

message(STATUS "This is a ${BUILD_TYPE} ${COMPILER_TYPE} build.")

set(V8PP_HEADER_ONLY 1)
set(V8PP_ISOLATE_DATA_SLOT 0)
# set(V8PP_VERSION "2.1.1")
# set(V8PP_VERSION_MAJOR 2)
# set(V8PP_VERSION_MINOR 1)
# set(V8PP_VERSION_PATCH 1)
set(PROJECT_VERSION "2.1.1")
set(PROJECT_VERSION_MAJOR 2)
set(PROJECT_VERSION_MINOR 1)
set(PROJECT_VERSION_PATCH 1)

set(V8PP_CONFIG_IN "${CMAKE_CURRENT_SOURCE_DIR}/libs/v8pp/v8pp/config.hpp.in")
set(V8PP_CONFIG_OUT "${CMAKE_CURRENT_BINARY_DIR}/libs/v8pp/v8pp/config.hpp")
configure_file(${V8PP_CONFIG_IN} ${V8PP_CONFIG_OUT} @ONLY)

# Add the generated config directory to include paths
include_directories(${CMAKE_CURRENT_BINARY_DIR}/libs/v8pp)

file(GLOB_RECURSE starcry_SOURCES
    "src/**.cpp"
    "src/**.hpp"
    "src/**.h"
    "libs/v8pp/v8pp/context.cpp"
    "libs/piper/src/**"
)
list(REMOVE_ITEM starcry_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/client.cpp")

add_executable(starcry ${starcry_SOURCES})
target_compile_features(starcry PRIVATE cxx_std_23)
target_compile_options(starcry PRIVATE
    ${COMMON_FLAGS}
    ${BUILD_TYPE_FLAGS}
    ${COMPILER_SPECIFIC_FLAGS}
    -isystem "${CMAKE_CURRENT_SOURCE_DIR}/libs/v8pp"
)


if (SANITIZER)
    target_compile_options(starcry PRIVATE -fsanitize=address)
    target_link_libraries(starcry PRIVATE
        -static-libgcc
        -static-libstdc++
        /usr/lib/llvm-15/lib/clang/15.0.7/lib/linux/libclang_rt.asan-x86_64.a
    )
else()
    target_link_libraries(starcry PRIVATE -static-libgcc -static-libstdc++)
endif()

configure_links(starcry)
set_target_properties(starcry PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

file(GLOB_RECURSE sc_client_SOURCES
    "src/client.cpp"
    "src/perlin.cpp"
    "src/texture.cpp"
    "src/stb.cpp"
    "src/util/logger.cpp"
    "src/rendering_engine/debug.cpp"
    "src/shapes/*.cpp"
    "src/util/noise_mappers.cpp"
)

add_executable(sc_client ${sc_client_SOURCES})
target_compile_features(sc_client PRIVATE cxx_std_23)
target_compile_definitions(sc_client PUBLIC SC_CLIENT)
target_compile_options(sc_client PRIVATE -isystem "${CMAKE_CURRENT_SOURCE_DIR}/libs/v8pp")
target_link_libraries(sc_client PRIVATE
    -lSDL2
    /opt/cppse/build/vivid/lib/libvivid.a
    /opt/cppse/build/fmt/lib/libfmt.a
)
configure_links(sc_client)

add_executable(unit_tests test/test_unit.cpp)
set_target_properties(unit_tests PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
target_compile_features(unit_tests PRIVATE cxx_std_23)
target_compile_options(unit_tests PRIVATE -isystem "${CMAKE_CURRENT_SOURCE_DIR}/libs/v8pp")
target_link_libraries(unit_tests PRIVATE
    /opt/cppse/build/v8/lib/libv8_monolith.a
    /opt/cppse/build/vivid/lib/libvivid.a
    Catch2::Catch2WithMain
)

file(GLOB_RECURSE integration_tests_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/test/test_integration.cpp")
list(APPEND integration_tests_SOURCES ${starcry_SOURCES})
list(APPEND integration_tests_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/test/test_integration.cpp")
list(REMOVE_ITEM integration_tests_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp")

add_executable(integration_tests ${integration_tests_SOURCES})
set_target_properties(integration_tests PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
target_compile_options(integration_tests PRIVATE ${COMMON_FLAGS} -isystem "${CMAKE_CURRENT_SOURCE_DIR}/libs/v8pp")
target_compile_features(integration_tests PRIVATE cxx_std_23)
target_link_libraries(integration_tests PRIVATE
    Catch2::Catch2WithMain
    -static-libgcc
    -static-libstdc++
)
configure_links(integration_tests)

set(all_sources ${starcry_SOURCES})
list(APPEND all_sources ${sc_client_SOURCES})
list(APPEND all_sources ${integration_tests_SOURCES})
file(GLOB_RECURSE extra_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/test/*.cpp" "${CMAKE_CURRENT_SOURCE_DIR}/test/*.hpp")
list(APPEND all_sources ${extra_SOURCES})
list(REMOVE_ITEM all_sources libs/v8pp/v8pp/context.cpp)
list(REMOVE_DUPLICATES all_sources)
clangformat_setup(${all_sources})
