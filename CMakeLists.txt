cmake_minimum_required(VERSION 3.10.2)
project(starcry VERSION 0.2 LANGUAGES CXX)

list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

set(CLANGFORMAT_EXECUTABLE clang-format-10)
set(Boost_USE_STATIC_LIBS ON)
set(BOOST_ROOT "/opt/cppse/build/boost/")

include(CheckIPOSupported)
include(ClangFormat)

find_package(V8 REQUIRED)
find_package(Boost 1.52 COMPONENTS program_options system date_time thread chrono REQUIRED )
#find_package(SFML 2.4 COMPONENTS graphics window system)
find_package(OpenMP)
find_package(Threads)

if (OPENMP_FOUND)
    set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
endif()

if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    # don't use libc++ v8's third party libc++ with unstable abi doesn't bode well with caf
    #set (CLANG_FLAGS "-stdlib=libc++")
    set(CMAKE_EXE_LINKER_FLAGS "-Wl,--no-rosegment")
else()
    #set (CLANG_FLAGS "")
endif()

if (SANITIZER)
    add_definitions("-fsanitize=address")
endif()
if (DEBUG)
    # --no-rosegment is important for being able to let valgrind/callgrind read symbols
    add_definitions("-std=c++17 -O0 -g -DV8_COMPRESS_POINTERS -fno-omit-frame-pointer -Wall -mssse3")
else()
    add_definitions("-std=c++17 -O3 -funsigned-char -stdlib=libstdc++ -Wall -fno-omit-frame-pointer -pedantic -pthread -mssse3 -Wno-reserved-user-defined-literal -DV8_COMPRESS_POINTERS")
endif()

include_directories(
    "${CMAKE_CURRENT_SOURCE_DIR}/include/"
    "/opt/cppse/build/v8pp/include/"  # for v8pp
    "/opt/cppse/build/v8pp/include/v8"  # for v8
    "/opt/cppse/build/v8pp/include/v8/include" # for v8
    "/opt/cppse/build/benchmarklib/include/"
    "/opt/cppse/build/boost/include/"
    "/opt/cppse/build/ffmpeg/include/"
    "/opt/cppse/build/fastpfor/include/"
    "/opt/cppse/build/sfml/include/"
    "/opt/cppse/build/seasocks/include/"
    "/opt/cppse/build/png++/"
    "${CMAKE_CURRENT_SOURCE_DIR}/libs/cereal/include/"
    "${CMAKE_CURRENT_SOURCE_DIR}/libs/framer/"
    "${CMAKE_CURRENT_SOURCE_DIR}/libs/piper/include/"
    "/usr/include/SDL2/"
)

file(GLOB_RECURSE starcry_SOURCES "src/**.cpp" "include/**.hpp"  "include/**.h" "/opt/cppse/build/v8pp/include/v8pp/context.cpp" "libs/piper/src/**")
#file(GLOB_RECURSE starcry_SOURCES "src/main.cpp")
list(REMOVE_ITEM starcry_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/client.cpp")

file(GLOB_RECURSE sc_client_SOURCES "src/client.cpp")
# (below) useful for testing v8 hello world in main.cpp
#file(GLOB_RECURSE starcry_SOURCES "src/main.cpp" "/opt/cppse/build/v8pp/include/v8pp/context.cpp")

add_executable(starcry ${starcry_SOURCES})
add_executable(sc_client ${sc_client_SOURCES})

# Optional IPO. Do not use IPO if it's not supported by compiler.
if (NOT DEBUG)
    check_ipo_supported(RESULT result OUTPUT output)
    if(result)
        set_property(TARGET starcry PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
    else()
        message(WARNING "IPO is not supported: ${output}")
    endif()
endif()

target_link_libraries(starcry -static-libgcc -static-libstdc++)

# boost
target_link_libraries(starcry ${Boost_LIBRARIES})

# pthreads
#target_link_libraries(starcry ${CMAKE_THREAD_LIBS_INIT})

# piper
#target_link_libraries(starcry ${CMAKE_CURRENT_SOURCE_DIR}/libs/piper/build/libpiper.a)

# SDL
target_link_libraries(sc_client -lSDL2)

# SFML (seems broken for some reason now)
#target_link_libraries(starcry ${SFML_LIBRARIES})
target_link_libraries(starcry /opt/cppse/build/sfml/lib/libsfml-graphics-s.a)
target_link_libraries(starcry /opt/cppse/build/sfml/lib/libsfml-window-s.a)
target_link_libraries(starcry /opt/cppse/build/sfml/lib/libsfml-system-s.a)
# below can be made obsolete if we can get rid of some Joystick related stuff in libsfml-window-s.a
target_link_libraries(starcry /lib/x86_64-linux-gnu/libudev.so)


# ffmpeg
target_link_libraries(starcry /opt/cppse/build/ffmpeg/lib/libswscale.a)
target_link_libraries(starcry /opt/cppse/build/ffmpeg/lib/libavdevice.a)
target_link_libraries(starcry /opt/cppse/build/ffmpeg/lib/libavformat.a)
target_link_libraries(starcry /opt/cppse/build/ffmpeg/lib/libavcodec.a)
target_link_libraries(starcry /opt/cppse/build/ffmpeg/lib/libavutil.a)
target_link_libraries(starcry /opt/cppse/build/ffmpeg/lib/libavfilter.a)

# X11 (statically linking causes SFML to segfault)
# We could retry with SDL at some point, but for now let's use dynamic linking
target_link_libraries(starcry -lXrandr)
target_link_libraries(starcry -lXext)
target_link_libraries(starcry -lXrender)
target_link_libraries(starcry -lX11)
target_link_libraries(starcry -lxcb)
target_link_libraries(starcry -lXau)
target_link_libraries(starcry -lXdmcp)
target_link_libraries(starcry -lGL)

# x264
target_link_libraries(starcry /opt/cppse/build/x264/lib/libx264.a)
target_link_libraries(starcry /usr/lib/x86_64-linux-gnu/libbz2.a)
target_link_libraries(starcry /usr/lib/x86_64-linux-gnu/libz.a)
target_link_libraries(starcry /usr/lib/x86_64-linux-gnu/liblzma.a)

target_link_libraries(starcry -ldl)

# More allegro/X11 stuff
target_link_libraries(starcry /usr/lib/x86_64-linux-gnu/libjpeg.a)
target_link_libraries(starcry /usr/lib/x86_64-linux-gnu/libpng16.a)
target_link_libraries(starcry /usr/lib/x86_64-linux-gnu/libGLU.a)
target_link_libraries(starcry /usr/lib/x86_64-linux-gnu/libfreetype.a)

# fastpfor
target_link_libraries(starcry /opt/cppse/build/fastpfor/lib/libFastPFor.a)

# v8
target_link_libraries(starcry /opt/cppse/build/v8pp/lib/libv8_monolith.a)
#target_link_libraries(starcry /usr/lib/gcc/x86_64-linux-gnu/7/libstdc++fs.a) # ubuntu 18.04
target_link_libraries(starcry /usr/lib/gcc/x86_64-linux-gnu/9/libstdc++fs.a)  # ubuntu 20.04

# generic
target_link_libraries(starcry /usr/lib/x86_64-linux-gnu/libssl.a)
target_link_libraries(starcry /usr/lib/x86_64-linux-gnu/libcrypto.a)

# seasocks
target_link_libraries(starcry /opt/cppse/build/seasocks/lib/libseasocks.a)

if (SANITIZER)
# ASAN
target_link_libraries(starcry /usr/lib/clang/10.0.1/lib/linux/libclang_rt.asan-x86_64.a)
endif()

install (TARGETS starcry DESTINATION bin)

#install (TARGETS sc_client DESTINATION bin)

clangformat_setup(${starcry_SOURCES} ${sc_client_SOURCES})
