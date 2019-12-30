cmake_minimum_required(VERSION 3.16.2)
project(starcry VERSION 0.2 LANGUAGES CXX)
include(libs/build_deps.cmake)
include(CheckIPOSupported)

set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake")
set(Boost_USE_STATIC_LIBS ON)
set(BOOST_ROOT "/opt/cppse/build/boost/")
set(V8_DIR "/opt/cppse/build/v8pp/include/v8/")

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
    set (CLANG_FLAGS "-stdlib=libc++")
else()
    set (CLANG_FLAGS "-stdlib=libc++")
endif()

if (NOT DEBUG)
    add_definitions("-std=c++17 -O3 -funsigned-char -Wall ${CLANG_FLAGS} -fno-omit-frame-pointer -pedantic -mssse3 -Wno-reserved-user-defined-literal")
else()
    add_definitions("-std=c++17 -O0 -g -fno-inline -Wall ${CLANG_FLAGS} -funsigned-char -fno-omit-frame-pointer -pedantic -mssse3 -Wno-reserved-user-defined-literal")
endif()

include_directories(
    "./include/"
    "${V8_DIR}/include" "${V8_DIR}/../" "${V8_INCLUDE_DIR}"
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

add_executable(starcry ${starcry_SOURCES})

# Optional IPO. Do not use IPO if it's not supported by compiler.
check_ipo_supported(RESULT result OUTPUT output)
if(result)
    set_property(TARGET starcry PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
else()
    message(WARNING "IPO is not supported: ${output}")
endif()

#target_link_libraries(starcry -error-limit=0)
target_link_libraries(starcry /opt/cppse/build/allegro5/lib/liballegro-static.a)
target_link_libraries(starcry /opt/cppse/build/allegro5/lib/liballegro_color-static.a)
target_link_libraries(starcry /opt/cppse/build/allegro5/lib/liballegro_memfile-static.a)
target_link_libraries(starcry /opt/cppse/build/allegro5/lib/liballegro_font-static.a)
target_link_libraries(starcry /opt/cppse/build/allegro5/lib/liballegro_ttf-static.a)
target_link_libraries(starcry /opt/cppse/build/allegro5/lib/liballegro_image-static.a)
target_link_libraries(starcry /opt/cppse/build/allegro5/lib/liballegro_primitives-static.a)
target_link_libraries(starcry /opt/cppse/build/benchmarklib/lib/libbenchmarklib.a)
target_link_libraries(starcry /opt/cppse/build/crtmpserver/lib/libcrtmpserver.a)
target_link_libraries(starcry /opt/cppse/build/crtmpserver/lib/libappselector.a)
target_link_libraries(starcry /opt/cppse/build/crtmpserver/lib/libflvplayback.a)
target_link_libraries(starcry /opt/cppse/build/crtmpserver/lib/libthelib.a)
target_link_libraries(starcry /opt/cppse/build/crtmpserver/lib/libcommon.a)
target_link_libraries(starcry /opt/cppse/build/crtmpserver/lib/libtinyxml.a)
target_link_libraries(starcry /opt/cppse/build/crtmpserver/lib/liblua.a)
target_link_libraries(starcry -lssl)
target_link_libraries(starcry -lcrypto)
target_link_libraries(starcry ${Boost_LIBRARIES})
target_link_libraries(starcry ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries(starcry /opt/cppse/build/caf/lib/libcaf_io_static.a)
target_link_libraries(starcry /opt/cppse/build/caf/lib/libcaf_core_static.a)
target_link_libraries(starcry /opt/cppse/build/ffmpeg/lib/libswscale.a)
target_link_libraries(starcry /opt/cppse/build/ffmpeg/lib/libavdevice.a)
target_link_libraries(starcry /opt/cppse/build/ffmpeg/lib/libavformat.a)
target_link_libraries(starcry /opt/cppse/build/ffmpeg/lib/libavcodec.a)
target_link_libraries(starcry /opt/cppse/build/ffmpeg/lib/libavutil.a)
target_link_libraries(starcry /opt/cppse/build/ffmpeg/lib/libavfilter.a)
target_link_libraries(starcry -lXcursor)
target_link_libraries(starcry -lXrender)
target_link_libraries(starcry -lxcb)
target_link_libraries(starcry /opt/cppse/build/x264/lib/libx264.a)
target_link_libraries(starcry -lbz2)
target_link_libraries(starcry /opt/cppse/build/fastpfor/lib/libFastPFor.a)
target_link_libraries(starcry -lz)
target_link_libraries(starcry -llzma)
target_link_libraries(starcry -ldl)
target_link_libraries(starcry -lswresample -lm -lz)
target_link_libraries(starcry /opt/cppse/build/v8pp/lib/v8.a)
target_link_libraries(starcry /usr/lib/llvm-9/lib/libc++.a)
target_link_libraries(starcry ${ALLEGRO_LIBRARIES})
target_link_libraries(starcry -lX11)
target_link_libraries(starcry -lXcursor)
target_link_libraries(starcry -ljpeg)
target_link_libraries(starcry -lpng)
target_link_libraries(starcry -lGLU)
target_link_libraries(starcry -lGL)
target_link_libraries(starcry -lfreetype)
target_link_libraries(starcry -lXrandr)
target_link_libraries(starcry -lXinerama)
target_link_libraries(starcry -lXi)

install (TARGETS starcry DESTINATION bin)
