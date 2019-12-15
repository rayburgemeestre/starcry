cmake_minimum_required(VERSION 3.0.0)

project(starcry VERSION 0.1 LANGUAGES CXX)

include(libs/build_deps.cmake)

set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake")
set(Boost_USE_STATIC_LIBS   ON)
if (NOT LIB_PREFIX_DIR)
    set (LIB_PREFIX_DIR "${CMAKE_CURRENT_SOURCE_DIR}/")
endif()
if (NOT BOOST_ROOT)
    set(BOOST_ROOT "${LIB_PREFIX_DIR}/boost_1_68_0/")
endif()
set (V8_DIR "${LIB_PREFIX_DIR}/libs/v8pp/v8/")

find_package(Allegro5static REQUIRED)
find_package(CAF REQUIRED core io)
find_package(V8 REQUIRED)
find_package(FFMPEG)
find_package(Boost 1.52 COMPONENTS program_options system date_time filesystem thread chrono REQUIRED )
find_package(OpenMP)

if (OPENMP_FOUND)
    set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
endif()

if (NOT CAF_FOUND)
    message(FATAL_ERROR "C++ Actor Framework library not found")
endif()

find_package( Threads )
if (NOT DEBUG)
    set (COMPILE_FLAGS "-O3 -Wall -funsigned-char -stdlib=libc++ -std=c++17 -fno-omit-frame-pointer -pedantic -mssse3 -Wl,--strip-all -flto -Wno-reserved-user-defined-literal")
else()
    set (COMPILE_FLAGS "-O0 -g -fno-inline -Wall -stdlib=libc++  -funsigned-char -std=c++17 -fno-omit-frame-pointer -pedantic -mssse3 -Wno-reserved-user-defined-literal")
endif()

add_definitions(${COMPILE_FLAGS})
include_directories(
    "./include/"
	"${V8_DIR}/include"
	"${V8_DIR}/../"
	"${V8_INCLUDE_DIR}"
	"${LIB_PREFIX_DIR}/libs/allegro5/include/"
	"${LIB_PREFIX_DIR}/libs/crow/include"
	"${LIB_PREFIX_DIR}/libs/FastPFor/"
	"${CMAKE_CURRENT_SOURCE_DIR}/libs/framer/"
	"${BENCHMARKLIB_INCLUDE_DIR}"
)
include_directories(${BOOST_ROOT})
include_directories("${LIB_PREFIX_DIR}/tmp/ffmpeg/")
link_directories("${V8_DIR}/lib/")
include_directories("${LIB_PREFIX_DIR}/libs/cereal/include/")
file(GLOB_RECURSE starcry_SOURCES "src/**.cpp" "include/**.h*" "${LIB_PREFIX_DIR}/libs/v8pp/v8pp/context.cpp")
add_executable(starcry ${starcry_SOURCES})
target_link_libraries(starcry ${BENCHMARKLIB_STATIC_LIBRARY})

foreach(crtmpserver_lib ${CRTMPSERVER_STATIC_LIBRARIES})
target_link_libraries(starcry ${crtmpserver_lib})
endforeach(crtmpserver_lib)
target_link_libraries(starcry -lssl)
target_link_libraries(starcry -lcrypto)
target_link_libraries(starcry ${Boost_LIBRARIES})
target_link_libraries(starcry ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries(starcry "/usr/local/lib/libcaf_io_static.a")
target_link_libraries(starcry "/usr/local/lib/libcaf_core_static.a")
target_link_libraries(starcry "${LIB_PREFIX_DIR}/tmp/ffmpeg/libswscale/libswscale.a")
target_link_libraries(starcry "${LIB_PREFIX_DIR}/tmp/ffmpeg/libavdevice/libavdevice.a")
target_link_libraries(starcry "${LIB_PREFIX_DIR}/tmp/ffmpeg/libavformat/libavformat.a")
target_link_libraries(starcry "${LIB_PREFIX_DIR}/tmp/ffmpeg/libavcodec/libavcodec.a")
target_link_libraries(starcry "${LIB_PREFIX_DIR}/tmp/ffmpeg/libavutil/libavutil.a")
target_link_libraries(starcry "${LIB_PREFIX_DIR}/tmp/ffmpeg/libavfilter/libavfilter.a")
target_link_libraries(starcry -lXcursor)
target_link_libraries(starcry -lXrender)
target_link_libraries(starcry -lxcb)
target_link_libraries(starcry "${LIB_PREFIX_DIR}/tmp/x264/libx264.a") # :-)
target_link_libraries(starcry -lbz2)
target_link_libraries(starcry "${LIB_PREFIX_DIR}/libs/FastPFor/libFastPFor.a")
target_link_libraries(starcry -lz)
target_link_libraries(starcry -llzma)
target_link_libraries(starcry -ldl)
target_link_libraries(starcry -lswresample -lm -lz)
target_link_libraries(starcry "${LIB_PREFIX_DIR}/v8.a")
target_link_libraries(starcry -lc++experimental)
target_link_libraries(starcry /usr/lib/llvm-7/lib/libc++fs.a)
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
