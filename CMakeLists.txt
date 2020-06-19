cmake_minimum_required(VERSION 3.10.2)
project(starcry VERSION 0.2 LANGUAGES CXX)

list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

set(CLANGFORMAT_EXECUTABLE clang-format-10)
set(Boost_USE_STATIC_LIBS ON)
set(BOOST_ROOT "/opt/cppse/build/boost/")

include(CheckIPOSupported)
include(ClangFormat)

find_package(CAF REQUIRED core io)
find_package(V8 REQUIRED)
find_package(Boost 1.52 COMPONENTS program_options system date_time thread chrono REQUIRED )
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

if (NOT DEBUG)
    add_definitions("-std=c++17 -O3 -funsigned-char -stdlib=libstdc++ -Wall -fno-omit-frame-pointer -pedantic -pthread -mssse3 -Wno-reserved-user-defined-literal -DV8_COMPRESS_POINTERS")
else()
    add_definitions("-std=c++17 -O0 -g -DV8_COMPRESS_POINTERS")  # --no-rosegment is important for being able to let valgrind/callgrind read symbols
endif()

include_directories(
    "./include/"
    "/opt/cppse/build/v8pp/include/"  # for v8pp
    "/opt/cppse/build/v8pp/include/v8"  # for v8
    "/opt/cppse/build/v8pp/include/v8/include" # for v8
    "/opt/cppse/build/benchmarklib/include/"
    "/opt/cppse/build/caf/include/"
    "/opt/cppse/build/allegro5/include/"
    "/opt/cppse/build/boost/include/"
    "/opt/cppse/build/ffmpeg/include/"
    "/opt/cppse/build/fastpfor/include/"
    "${CMAKE_CURRENT_SOURCE_DIR}/libs/crow/include/"
    "${CMAKE_CURRENT_SOURCE_DIR}/libs/cereal/include/"
    "${CMAKE_CURRENT_SOURCE_DIR}/libs/framer/"
)

file(GLOB_RECURSE starcry_SOURCES "src/**.cpp" "include/**.h*" "/opt/cppse/build/v8pp/include/v8pp/context.cpp")
# (below) useful for testing v8 hello world in main.cpp
#file(GLOB_RECURSE starcry_SOURCES "src/main.cpp" "/opt/cppse/build/v8pp/include/v8pp/context.cpp")
# (below) useful for testing caf hello world in main.cpp
#file(GLOB_RECURSE starcry_SOURCES "src/main.cpp")

add_executable(starcry ${starcry_SOURCES})

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

# allegro 5
target_link_libraries(starcry /opt/cppse/build/allegro5/lib/liballegro-static.a)
target_link_libraries(starcry /opt/cppse/build/allegro5/lib/liballegro_color-static.a)
target_link_libraries(starcry /opt/cppse/build/allegro5/lib/liballegro_memfile-static.a)
target_link_libraries(starcry /opt/cppse/build/allegro5/lib/liballegro_font-static.a)
target_link_libraries(starcry /opt/cppse/build/allegro5/lib/liballegro_ttf-static.a)
target_link_libraries(starcry /opt/cppse/build/allegro5/lib/liballegro_image-static.a)
target_link_libraries(starcry /opt/cppse/build/allegro5/lib/liballegro_primitives-static.a)

# benchmarklib
target_link_libraries(starcry /opt/cppse/build/benchmarklib/lib/libbenchmarklib.a)

# boost
target_link_libraries(starcry ${Boost_LIBRARIES})

# pthreads
#target_link_libraries(starcry ${CMAKE_THREAD_LIBS_INIT})

# caf
target_link_libraries(starcry /opt/cppse/build/caf/lib/libcaf_io.a)
target_link_libraries(starcry /opt/cppse/build/caf/lib/libcaf_core.a)

# ffmpeg
target_link_libraries(starcry /opt/cppse/build/ffmpeg/lib/libswscale.a)
target_link_libraries(starcry /opt/cppse/build/ffmpeg/lib/libavdevice.a)
target_link_libraries(starcry /opt/cppse/build/ffmpeg/lib/libavformat.a)
target_link_libraries(starcry /opt/cppse/build/ffmpeg/lib/libavcodec.a)
target_link_libraries(starcry /opt/cppse/build/ffmpeg/lib/libavutil.a)
target_link_libraries(starcry /opt/cppse/build/ffmpeg/lib/libavfilter.a)

# X11
target_link_libraries(starcry /usr/lib/x86_64-linux-gnu/libXinerama.a)
target_link_libraries(starcry /usr/lib/x86_64-linux-gnu/libXi.a)
target_link_libraries(starcry /usr/lib/x86_64-linux-gnu/libXrandr.a)
target_link_libraries(starcry /usr/lib/x86_64-linux-gnu/libXext.a)
target_link_libraries(starcry /usr/lib/x86_64-linux-gnu/libXrender.a)
target_link_libraries(starcry /usr/lib/x86_64-linux-gnu/libXcursor.a)
target_link_libraries(starcry /usr/lib/x86_64-linux-gnu/libXfixes.a)
target_link_libraries(starcry /usr/lib/x86_64-linux-gnu/libX11.a)
target_link_libraries(starcry /usr/lib/x86_64-linux-gnu/libxcb.a)
target_link_libraries(starcry /usr/lib/x86_64-linux-gnu/libXau.a)
target_link_libraries(starcry /usr/lib/x86_64-linux-gnu/libXdmcp.a)

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
target_link_libraries(starcry -lGL)
target_link_libraries(starcry /usr/lib/x86_64-linux-gnu/libfreetype.a)

# fastpfor
target_link_libraries(starcry /opt/cppse/build/fastpfor/lib/libFastPFor.a)

# v8
target_link_libraries(starcry /opt/cppse/build/v8pp/lib/libv8_monolith.a)
target_link_libraries(starcry /usr/lib/gcc/x86_64-linux-gnu/7/libstdc++fs.a)

# generic
target_link_libraries(starcry /usr/lib/x86_64-linux-gnu/libssl.a)
target_link_libraries(starcry /usr/lib/x86_64-linux-gnu/libcrypto.a)

# crtmpserver
target_link_libraries(starcry /opt/cppse/build/crtmpserver/lib/libcrtmpserver.a)
target_link_libraries(starcry /opt/cppse/build/crtmpserver/lib/libappselector.a)
target_link_libraries(starcry /opt/cppse/build/crtmpserver/lib/libflvplayback.a)
target_link_libraries(starcry /opt/cppse/build/crtmpserver/lib/libthelib.a)
target_link_libraries(starcry /opt/cppse/build/crtmpserver/lib/libcommon.a)
target_link_libraries(starcry /opt/cppse/build/crtmpserver/lib/libtinyxml.a)
target_link_libraries(starcry /opt/cppse/build/crtmpserver/lib/liblua.a)


# trying to include more statically will eventually result in
# starcry: symbol lookup error: starcry: undefined symbol: pthread_create
# at runtime..
#target_link_libraries(starcry /usr/lib/x86_64-linux-gnu/librt.a)
#target_link_libraries(starcry /usr/lib/x86_64-linux-gnu/libm.a)
#target_link_libraries(starcry -Wl,--whole-archive /usr/lib/x86_64-linux-gnu/libpthread.a -Wl,--no-whole-archive)
#target_link_libraries(starcry /usr/lib/x86_64-linux-gnu/libc.a)
#target_link_libraries(starcry /usr/lib/x86_64-linux-gnu/libmvec.a)
#target_link_libraries(starcry /usr/lib/x86_64-linux-gnu/libpthread.a)

install (TARGETS starcry DESTINATION bin)

target_clangformat_setup(starcry)

# CAF example
file(GLOB_RECURSE caf_hello_SOURCES "docs/caf_hello_world.cpp")
add_executable(caf_hello ${caf_hello_SOURCES})
check_ipo_supported(RESULT result OUTPUT output)
if(result)
set_property(TARGET caf_hello PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
endif()
target_link_libraries(caf_hello /opt/cppse/build/caf/lib/libcaf_io.a)
target_link_libraries(caf_hello /opt/cppse/build/caf/lib/libcaf_core.a)
target_link_libraries(caf_hello /usr/lib/x86_64-linux-gnu/libssl.a)
target_link_libraries(caf_hello /usr/lib/x86_64-linux-gnu/libcrypto.a)
install (TARGETS caf_hello DESTINATION bin)

# CAF client / server
file(GLOB_RECURSE caf_server_SOURCES "docs/caf_server.cpp")
add_executable(caf_server ${caf_server_SOURCES})
check_ipo_supported(RESULT result OUTPUT output)
if(result)
    set_property(TARGET caf_server PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
endif()
target_link_libraries(caf_server /opt/cppse/build/caf/lib/libcaf_io.a)
target_link_libraries(caf_server /opt/cppse/build/caf/lib/libcaf_core.a)
target_link_libraries(caf_server /usr/lib/x86_64-linux-gnu/libssl.a)
target_link_libraries(caf_server /usr/lib/x86_64-linux-gnu/libcrypto.a)
install (TARGETS caf_server DESTINATION bin)

file(GLOB_RECURSE caf_client_SOURCES "docs/caf_client.cpp")
add_executable(caf_client ${caf_client_SOURCES})
check_ipo_supported(RESULT result OUTPUT output)
if(result)
    set_property(TARGET caf_client PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
endif()
target_link_libraries(caf_client /opt/cppse/build/caf/lib/libcaf_io.a)
target_link_libraries(caf_client /opt/cppse/build/caf/lib/libcaf_core.a)
target_link_libraries(caf_client /usr/lib/x86_64-linux-gnu/libssl.a)
target_link_libraries(caf_client /usr/lib/x86_64-linux-gnu/libcrypto.a)
install (TARGETS caf_client DESTINATION bin)
