cmake_minimum_required(VERSION 3.10.2)

find_program(CCACHE_PROGRAM ccache)
if(CCACHE_PROGRAM)
    set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE "${CCACHE_PROGRAM}")
endif()

set(CMAKE_CXX_STANDARD 20)

project(starcry VERSION 0.2 LANGUAGES CXX)

if(CMAKE_BUILD_TYPE MATCHES Debug)
    set(DEBUG on)  # this is the define that we set for debug
endif()

list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

set(CLANGFORMAT_EXECUTABLE clang-format-14)
set(Boost_USE_STATIC_LIBS ON)
set(BOOST_ROOT "/opt/cppse/build/boost/")

# include(warnings)
# set_warnings()

include(ClangFormat)
include(StarcryLinking)

find_package(V8 REQUIRED)
find_package(Boost 1.52 COMPONENTS program_options system date_time thread chrono REQUIRED )
find_package(Threads)

add_subdirectory(libs/Catch2)

# unittests
add_executable(tests test/test_unit.cpp)
target_link_libraries(tests PRIVATE /opt/cppse/build/v8pp/lib/libv8_monolith.a)
target_link_libraries(tests PRIVATE Catch2::Catch2WithMain)

if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fuse-ld=gold")
else()
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-z,notext -Wno-unknown-pragmas")
endif()

if (SANITIZER)
    add_definitions("-fsanitize=address")
endif()

if (DEVELOP)
    add_definitions("-DDEVELOP")
endif()

if (DEBUG)
    message(STATUS "This is a DEBUG build.")
    if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
        # -gdwarf-4 is needed for valgrind, version 5 seens unsupported.
        add_definitions("-std=c++20 -O0 -g -stdlib=libstdc++ -DV8_COMPRESS_POINTERS -fno-omit-frame-pointer -Wall -mssse3 -gdwarf-4")
    else()
        add_definitions("-std=c++2a -O0 -g                   -DV8_COMPRESS_POINTERS -fno-omit-frame-pointer -Wall -mssse3 -Wno-unknown-pragmas -Wno-literal-suffix -fmudflap -lmudflap")
    endif()
else()
    message(STATUS "This is a RELEASE build.")
    if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
        add_definitions("-std=c++20 -O2 -funsigned-char -stdlib=libstdc++ -Wall -fno-omit-frame-pointer -pedantic -pthread -mssse3 -Wno-reserved-user-defined-literal -DV8_COMPRESS_POINTERS")
    else()
        add_definitions("-std=c++2a -O2 -funsigned-char                   -Wall -fno-omit-frame-pointer -pedantic -pthread -mssse3 -Wno-reserved-user-defined-literal -DV8_COMPRESS_POINTERS -Wno-unknown-pragmas -Wno-literal-suffix")
    endif()
endif()

include_directories(
    "${CMAKE_CURRENT_SOURCE_DIR}/src/"
    "/opt/cppse/build/v8pp/include/"  # for v8pp
    "/opt/cppse/build/v8pp/include/v8"  # for v8
    "/opt/cppse/build/v8pp/include/v8/include" # for v8
    "/opt/cppse/build/benchmarklib/include/"
    "/opt/cppse/build/boost/include/"
    "/opt/cppse/build/ffmpeg/include/"
    "/opt/cppse/build/fastpfor/include/"
    "/opt/cppse/build/sfml/include/"
    "/opt/cppse/build/seasocks/include/"
    "/opt/cppse/build/fmt/include/"
    "/opt/cppse/build/png++/"
    "/opt/cppse/build/openexr/include/Imath/"
    "/opt/cppse/build/openexr/include/OpenEXR/"
    "/opt/cppse/build/tvision/include/"
    "/opt/cppse/build/inotify-cpp/include/"
    "/opt/cppse/build/redis-plus-plus/include/"
    "/opt/cppse/build/hiredis/include/"
    "${CMAKE_CURRENT_SOURCE_DIR}/libs/cereal/include/"
    "${CMAKE_CURRENT_SOURCE_DIR}/libs/framer/"
    "${CMAKE_CURRENT_SOURCE_DIR}/libs/json/include/"
    "${CMAKE_CURRENT_SOURCE_DIR}/libs/piper/include/"
    "${CMAKE_CURRENT_SOURCE_DIR}/libs/perlin_noise/"
    "${CMAKE_CURRENT_SOURCE_DIR}/libs/stb/"
    "/usr/include/SDL2/"
)

file(GLOB_RECURSE starcry_SOURCES "src/**.cpp" "src/**.hpp"  "src/**.h" "/opt/cppse/build/v8pp/include/v8pp/*.cpp" "libs/piper/src/**")
list(REMOVE_ITEM starcry_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/client.cpp")
list(REMOVE_ITEM starcry_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/deprecated/generator.cpp")
list(REMOVE_ITEM starcry_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/deprecated/scripting.cpp")

file(GLOB_RECURSE sc_client_SOURCES "src/client.cpp" "src/perlin.cpp" "src/texture.cpp" "src/stb.cpp" "src/util/logger.cpp")

add_executable(starcry ${starcry_SOURCES})
add_executable(sc_client ${sc_client_SOURCES})

target_compile_definitions(sc_client PUBLIC SC_CLIENT)

# integration tests
file(GLOB_RECURSE integration_tests_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/test/test_integration.cpp")
list(APPEND integration_tests_SOURCES ${starcry_SOURCES})
list(APPEND integration_tests_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/test/test_integration.cpp")
list(REMOVE_ITEM integration_tests_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp")
add_executable(integration_tests ${integration_tests_SOURCES})
target_link_libraries(integration_tests PRIVATE Catch2::Catch2WithMain)
configure_links(integration_tests)

target_link_libraries(integration_tests PRIVATE -static-libgcc -static-libstdc++)

if (SANITIZER)
    # ASAN

    #target_link_libraries(starcry PRIVATE -static-libgcc -static-libstdc++ /usr/lib/llvm-12/lib/clang/12.0.1/lib/linux/libclang_rt.asan-x86_64.a)
    target_link_libraries(starcry PRIVATE -static-libgcc -static-libstdc++ /usr/lib/llvm-14/lib/clang/14.0.0/lib/linux/libclang_rt.asan-x86_64.a)

    # target_link_libraries(starcry -lclang_rt.asan-x86_64)
else()
    target_link_libraries(starcry PRIVATE -static-libgcc -static-libstdc++)
endif()

# SDL
target_link_libraries(sc_client PRIVATE -lSDL2)
target_link_libraries(sc_client PRIVATE /opt/cppse/build/fmt/lib/libfmt.a)
# configure_links(sc_client)

configure_links(starcry)

install (TARGETS starcry DESTINATION bin)

set(all_sources ${starcry_SOURCES})
list(APPEND all_sources ${sc_client_SOURCES})
list(APPEND all_sources ${integration_tests_SOURCES})
list(REMOVE_ITEM all_sources /opt/cppse/build/v8pp/include/v8pp/context.cpp)  # causes crash since owned by root
list(REMOVE_DUPLICATES all_sources)
clangformat_setup(${all_sources})
