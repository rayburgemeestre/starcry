cmake_minimum_required(VERSION 3.0.0)

project(starcry VERSION 0.1 LANGUAGES CXX)

#** ON HOLD ** include("cmake/gtestmockscript.cmake")

set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake")

include(libs/build_deps.cmake)

#message(STATUS "${v8_LIBRARIES}")
#message(FATAL_ERROR "END")

message(STATUS "Configuring for static...")
set(Boost_USE_STATIC_LIBS   ON)

if (NOT LIB_PREFIX_DIR)
  set (LIB_PREFIX_DIR "${CMAKE_CURRENT_SOURCE_DIR}/")
endif()

if (NOT BOOST_ROOT)
    set(BOOST_ROOT "${LIB_PREFIX_DIR}/boost_1_68_0/")
endif()

find_package(Allegro5static REQUIRED)

#find_package(Boost 1.47.0 REQUIRED program_options system)
find_package(CAF REQUIRED core io)
set (V8_DIR "${LIB_PREFIX_DIR}/libs/v8pp/v8/")
find_package(V8 REQUIRED)
find_package(FFMPEG)

#find_library(benchmarklib_LIBRARIES libbenchmarklib REQUIRED)
#find_library(benchmarklib_LIBRARIES NAMES libbenchmarklib.a REQUIRED)
#find_library(crtmpserver_LIBRARIES NAMES libcrtmpserver.a REQUIRED)

#crow
if (MSVC)
    find_package( Boost 1.52 COMPONENTS program_options system date_time filesystem system thread regex REQUIRED )
else()
    #set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -std=c++1y")
    # added chrono as it's needed in case of static libbenchmarklib.a
    find_package( Boost 1.52 COMPONENTS program_options system date_time filesystem thread chrono REQUIRED )
endif()


message(STATUS ${FFMPEG_LIBRARIES})
#message(STATUS ${BENCHMARKLIB_STATIC_LIBRARY})
message(STATUS ${CAF_LIBRARIES})

find_package(OpenMP)
if (OPENMP_FOUND)
    set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
endif()

if (NOT CAF_FOUND)
    message(FATAL_ERROR "C++ Actor Framework library not found")
endif()

find_package( Threads )

# whole program optimizations screws things over
#add_definitions(-O3 -fwhole-program -Wall -funsigned-char -std=c++14 -fno-omit-frame-pointer -pedantic)
#add_definitions(-O0 -g -Wall -funsigned-char -std=c++14)
if (NOT DEBUG)
    #set (COMPILE_FLAGS "-static-libgcc -static-libstdc++ -Wl,--as-needed -Wl,-static-libgcc -Wl,-static-libstdc++ -O3 -Wall -funsigned-char -std=c++17 -fno-omit-frame-pointer -pedantic -mssse3")
    # -stdlib=libc++  is clang specific
    set (COMPILE_FLAGS "-O3 -Wall -funsigned-char -stdlib=libc++ -std=c++17 -fno-omit-frame-pointer -pedantic -mssse3 -Wl,--strip-all -flto")
    #set (COMPILE_FLAGS "-Os -fdata-sections -ffunction-sections -Wall -funsigned-char -stdlib=libc++ -std=c++17 -fno-omit-frame-pointer -pedantic -mssse3 -Wl,--strip-all -flto -Wl,--gc-sections")
else()
    set (COMPILE_FLAGS "-O0 -g -fno-inline -Wall -stdlib=libc++  -funsigned-char -std=c++17 -fno-omit-frame-pointer -pedantic")
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
	"${BENCHMARKLIB_INCLUDE_DIR}"
)
#if (EXISTS "/etc/centos-release")
include_directories(${BOOST_ROOT})
include_directories("${LIB_PREFIX_DIR}/tmp/ffmpeg/")
#endif (EXISTS "/etc/centos-release")
link_directories("${V8_DIR}/lib/")
if (EXISTS "/etc/centos-release")
    link_directories("${BOOST_ROOT}/stage/lib")
endif (EXISTS "/etc/centos-release")

include_directories("${LIB_PREFIX_DIR}/libs/cereal/include/")

#include_directories(${CAF_INCLUDE_DIRS})
file(GLOB_RECURSE starcry_SOURCES "src/**.cpp" "include/**.h*" "${LIB_PREFIX_DIR}/libs/v8pp/v8pp/context.cpp")
#file(GLOB_RECURSE starcry_SOURCES "src/main.cpp")
add_executable(starcry ${starcry_SOURCES})
target_link_libraries(starcry ${BENCHMARKLIB_STATIC_LIBRARY})

foreach(crtmpserver_lib ${CRTMPSERVER_STATIC_LIBRARIES})
target_link_libraries(starcry ${crtmpserver_lib})
endforeach(crtmpserver_lib)

#target_link_libraries(starcry -static -lssl)
#target_link_libraries(starcry -static -lcrypto)
#target_link_libraries(starcry -static -lstdc++fs)
target_link_libraries(starcry -lssl)
target_link_libraries(starcry -lcrypto)



target_link_libraries(starcry ${Boost_LIBRARIES})
target_link_libraries(starcry ${CMAKE_THREAD_LIBS_INIT})
#if (STATIC)
    target_link_libraries(starcry "/usr/local/lib/libcaf_io_static.a")
    target_link_libraries(starcry "/usr/local/lib/libcaf_core_static.a")
    #    target_link_libraries(starcry "${LIB_PREFIX_DIR}/ffmpeg.a") # :-)
    #target_link_libraries(starcry "${LIB_PREFIX_DIR}/tmp/ffmpeg/libavformat/libavformat.a")
    #target_link_libraries(starcry "${LIB_PREFIX_DIR}/tmp/ffmpeg/libavutil/libavutil.a")
    #target_link_libraries(starcry "${LIB_PREFIX_DIR}/tmp/ffmpeg/libavcodec/libavcodec.a")
    #target_link_libraries(starcry "${LIB_PREFIX_DIR}/tmp/ffmpeg/libavfilter/libavfilter.a")
    #target_link_libraries(starcry "${LIB_PREFIX_DIR}/tmp/ffmpeg/libswscale/libswscale.a")

    # order is important :-)
    target_link_libraries(starcry "${LIB_PREFIX_DIR}/tmp/ffmpeg/./libswscale/libswscale.a")
    target_link_libraries(starcry "${LIB_PREFIX_DIR}/tmp/ffmpeg/./libavdevice/libavdevice.a")
    target_link_libraries(starcry "${LIB_PREFIX_DIR}/tmp/ffmpeg/./libavformat/libavformat.a")
    target_link_libraries(starcry "${LIB_PREFIX_DIR}/tmp/ffmpeg/./libavcodec/libavcodec.a")
    target_link_libraries(starcry "${LIB_PREFIX_DIR}/tmp/ffmpeg/./libavutil/libavutil.a")
    #    target_link_libraries(starcry -lva)
    #    target_link_libraries(starcry -lva-drm)
    #target_link_libraries(starcry -lva-x11)
    target_link_libraries(starcry -lXcursor)
    target_link_libraries(starcry -lXrender)  # there is an .a of this
    target_link_libraries(starcry -lxcb) # ./x86_64-linux-gnu/libxcb.a

    #target_link_libraries(starcry "${LIB_PREFIX_DIR}/tmp/ffmpeg/./libavfilter/libavfilter.a")
    #target_link_libraries(starcry "${LIB_PREFIX_DIR}/tmp/ffmpeg/./libpostproc/libpostproc.a")

    target_link_libraries(starcry "${LIB_PREFIX_DIR}/tmp/x264/libx264.a") # :-)

    target_link_libraries(starcry -lbz2)
    # target_link_libraries(starcry -lva)
    #    target_link_libraries(starcry -lx264)
    #else()
    #target_link_libraries(starcry ${CAF_LIBRARIES})
    #target_link_libraries(starcry ${FFMPEG_LIBRARIES})
    #endif()
target_link_libraries(starcry "${LIB_PREFIX_DIR}/libs/FastPFor/libFastPFor.a")
#target_link_libraries(starcry -static -lz) # NEEDED FOR CENTOS7u2, && Ubuntu it seems?
#target_link_libraries(starcry -static -llzma)
target_link_libraries(starcry -lz) # NEEDED FOR CENTOS7u2, && Ubuntu it seems?
target_link_libraries(starcry -llzma)

# This order works on my Ubuntu 15.10 box at home *and* box at work.

target_link_libraries(starcry -ldl)


#target_link_libraries(starcry "${V8_DIR}/lib/libv8_libbase.a")
#target_link_libraries(starcry -lv8_libbase)
#target_link_libraries(starcry "${V8_DIR}/lib/libv8_nosnapshot.a")
#target_link_libraries(starcry "${V8_DIR}/lib/libv8_libplatform.a")
#target_link_libraries(starcry "${LIB_PREFIX_DIR}/libs/v8pp/v8/out/x64.release/obj/third_party/icu/libicui18n.a")
#target_link_libraries(starcry "${LIB_PREFIX_DIR}/libs/v8pp/v8/out/x64.release/obj/third_party/icu/libicuuc.a")
#target_link_libraries(starcry "${LIB_PREFIX_DIR}/libs/v8pp/v8/out/x64.release/obj/libv8_libplatform.a")
#target_link_libraries(starcry "${LIB_PREFIX_DIR}/libs/v8pp/v8/out/x64.release/obj/libv8_libbase.a")
#target_link_libraries(starcry "${LIB_PREFIX_DIR}/libs/v8pp/v8/out/x64.release/obj/libv8_libplatform.a")
#target_link_libraries(starcry "${LIB_PREFIX_DIR}/libs/v8pp/v8/out/x64.release/obj/libv8_libbase.a")
target_link_libraries(starcry "${LIB_PREFIX_DIR}/v8.a")

#target_link_libraries(starcry "${LIB_PREFIX_DIR}/v8.a")

#target_link_libraries(starcry -lv8_libplatform) # libv8_libplatform.a seems no longer available?
#target_link_libraries(starcry "${V8_DIR}/lib/libv8_libsampler.a")
#target_link_libraries(starcry "${LIB_PREFIX_DIR}/libs/v8pp/v8/out/x64.release/obj/third_party/icu/libicuuc.a")
#target_link_libraries(starcry "${LIB_PREFIX_DIR}/libs/v8pp/v8/out/x64.release/obj/third_party/icu/libicui18n.a")
#message(STATUS "${V8_DIR}/lib/libv8_libbase.a")
#target_link_libraries(starcry "${V8_DIR}/lib/libv8_libbase.a")

# GCC? target_link_libraries(starcry -lstdc++fs)
# CLANG?
 target_link_libraries(starcry -lc++experimental)
target_link_libraries(starcry /usr/lib/llvm-7/lib/libc++fs.a)

#if (STATIC)
    target_link_libraries(starcry ${ALLEGRO_LIBRARIES})
    target_link_libraries(starcry -lX11)
    #target_link_libraries(starcry -static -lXcursor)
    #target_link_libraries(starcry -static -ljpeg)
    #target_link_libraries(starcry -static -lpng)
    #target_link_libraries(starcry -static -lGLU)
    target_link_libraries(starcry -lXcursor)
    target_link_libraries(starcry -ljpeg)
    target_link_libraries(starcry -lpng)
    target_link_libraries(starcry -lGLU)
    target_link_libraries(starcry -lGL)
    #target_link_libraries(starcry /usr/lib/libGLC.a)
    #target_link_libraries(starcry -static -lfreetype)
    target_link_libraries(starcry -lfreetype)
    if (NOT EXISTS "/etc/centos-release")
        #target_link_libraries(starcry -static -lXrandr)
        #target_link_libraries(starcry -static -lXinerama)
        #target_link_libraries(starcry -static -lXi)
        target_link_libraries(starcry -lXrandr)
        target_link_libraries(starcry -lXinerama)
        target_link_libraries(starcry -lXi)
        #xrandr xi xxf86vm gl
    endif()
    #END
    #-lX11 -ltiff -ljpeg -lpng -llapack -lGLU -lGL -ldc1394  -L -lXext -pthread
#else()
#    target_link_libraries(starcry ${ALLEGRO_LIBRARIES})
#endif()

# This order works on my Ubuntu 15.10 box at work only
#target_link_libraries(starcry "${V8_DIR}/lib/libv8_libbase.a")
#target_link_libraries(starcry "${V8_DIR}/lib/libv8_libbase.a")
#target_link_libraries(starcry "${V8_DIR}/lib/libv8_libplatform.a")
#target_link_libraries(starcry "${V8_DIR}/lib/libv8_nosnapshot.a")
#target_link_libraries(starcry -ldl)
#target_link_libraries(starcry -licui18n)
#target_link_libraries(starcry -licuuc)

install (TARGETS starcry DESTINATION bin)

# ** ON HOLD ** ###################################################### Light renderer
# ** ON HOLD ** # let's always link the worker statically... for convenience.
# ** ON HOLD ** #target_compile_options(starcry_worker PUBLIC  -DWORKER_ONLY=1)
# ** ON HOLD ** file(GLOB_RECURSE starcry_worker_SOURCES "src/main.cpp" "src/renderer.cpp" "src/rendering_engine_wrapper.cpp" "src/util/compress_vector.cpp" "src/util/remote_actors.cpp")
# ** ON HOLD ** # Don't include Main.cpp from src
# ** ON HOLD ** file(GLOB MAIN_SRC "src/main.cpp")
# ** ON HOLD ** add_executable(starcry_worker ${starcry_worker_SOURCES})
# ** ON HOLD ** set_target_properties (starcry_worker PROPERTIES COMPILE_FLAGS "-DWORKER_ONLY=1")
# ** ON HOLD ** install (TARGETS starcry_worker DESTINATION bin)
# ** ON HOLD ** target_link_libraries(starcry_worker "${PROJECT_SOURCE_DIR}/libs/FastPFor/libFastPFor.a")
# ** ON HOLD ** target_link_libraries(starcry_worker ${BENCHMARKLIB_STATIC_LIBRARY})
# ** ON HOLD ** target_link_libraries(starcry_worker ${Boost_LIBRARIES})
# ** ON HOLD ** target_link_libraries(starcry_worker ${CMAKE_THREAD_LIBS_INIT})
# ** ON HOLD ** target_link_libraries(starcry_worker "/usr/local/lib/libcaf_io_static.a")
# ** ON HOLD ** target_link_libraries(starcry_worker "/usr/local/lib/libcaf_core_static.a")
# ** ON HOLD ** if (EXISTS "/etc/centos-release")
# ** ON HOLD **     target_link_libraries(starcry_worker -lz) # NEEDED FOR CENTOS7u2
# ** ON HOLD **     target_link_libraries(starcry_worker -llzma) # NEEDED FOR CENTOS7u2
# ** ON HOLD ** endif (EXISTS "/etc/centos-release")
# ** ON HOLD ** # This order works on my Ubuntu 15.10 box at home *and* box at work.
# ** ON HOLD ** #target_link_libraries(starcry_worker -lv8)
# ** ON HOLD ** target_link_libraries(starcry_worker -ldl)
# ** ON HOLD ** target_link_libraries(starcry_worker -lX11)
# ** ON HOLD ** target_link_libraries(starcry_worker -lXcursor)
# ** ON HOLD ** target_link_libraries(starcry_worker -ljpeg)
# ** ON HOLD ** target_link_libraries(starcry_worker -lpng)
# ** ON HOLD ** target_link_libraries(starcry_worker -lGLU)
# ** ON HOLD ** target_link_libraries(starcry_worker -lGL)
# ** ON HOLD ** target_link_libraries(starcry_worker -lfreetype)
# ** ON HOLD ** if (NOT EXISTS "/etc/centos-release")
# ** ON HOLD **     target_link_libraries(starcry_worker -lXrandr)
# ** ON HOLD **     target_link_libraries(starcry_worker -lXinerama)
# ** ON HOLD **     target_link_libraries(starcry_worker -lXi)
# ** ON HOLD **     #xrandr xi xxf86vm gl
# ** ON HOLD ** endif()
# ** ON HOLD ** target_link_libraries(starcry_worker ${ALLEGRO_LIBRARIES})
# ** ON HOLD ** #######################################################
# ** ON HOLD ** 
# ** ON HOLD ** # Test files, always dynamic.
# ** ON HOLD ** file(GLOB_RECURSE starcry_test_SOURCES "src/primitives/**.cpp")
# ** ON HOLD ** file(GLOB TEST_FILES "test/*.cpp" "${LIB_PREFIX_DIR}/libs/v8pp/v8pp/context.cpp" "src/v8_wrapper.cpp")
# ** ON HOLD ** # Don't include Main.cpp from src
# ** ON HOLD ** file(GLOB MAIN_SRC "src/main.cpp")
# ** ON HOLD ** #list(REMOVE_ITEM starcry_SOURCES ${MAIN_SRC})
# ** ON HOLD ** add_executable(starcry_tests ${TEST_FILES} ${starcry_test_SOURCES})
# ** ON HOLD ** # Set link libraries (order matters)
# ** ON HOLD ** target_link_libraries(starcry_tests libgtest)
# ** ON HOLD ** target_link_libraries(starcry_tests libgmock)
# ** ON HOLD ** target_link_libraries(starcry_tests ${Boost_LIBRARIES})
# ** ON HOLD ** target_link_libraries(starcry_tests ${SFML_LIBRARIES})
# ** ON HOLD ** target_link_libraries(starcry_tests ${ALLEGRO_LIBRARIES})
# ** ON HOLD ** target_link_libraries(starcry_tests ${BENCHMARKLIB_STATIC_LIBRARY})
# ** ON HOLD ** target_link_libraries(starcry_tests ${CMAKE_THREAD_LIBS_INIT})
# ** ON HOLD ** # V8 dependencies (the one provided by Ubuntu is not new enough for v8pp)
# ** ON HOLD ** target_link_libraries(starcry_tests -lv8)
# ** ON HOLD ** target_link_libraries(starcry_tests -licui18n)
# ** ON HOLD ** target_link_libraries(starcry_tests -licuuc)
# ** ON HOLD ** target_link_libraries(starcry_tests "${V8_DIR}/lib/libv8_libplatform.a")
# ** ON HOLD ** target_link_libraries(starcry_tests "${V8_DIR}/lib/libv8_libbase.a")
# ** ON HOLD ** #target_link_libraries(starcry_tests "${V8_DIR}/lib/libv8_libbase.a")
# ** ON HOLD ** #target_link_libraries(starcry_tests "${V8_DIR}/lib/libv8_nosnapshot.a")
# ** ON HOLD ** target_link_libraries(starcry_tests -ldl)
# ** ON HOLD ** 
# ** ON HOLD ** add_test(NAME starcry COMMAND starcry_tests --unittests)
# ** ON HOLD ** 
# ** ON HOLD ** #add_executable(starcry_ubuntu)
# ** ON HOLD ** #add_custom_target(
# ** ON HOLD ** #    clean_ubuntu
# ** ON HOLD ** #    COMMAND docker run -i -t -v ${PROJECT_SOURCE_DIR}:/projects/starcry sc_build_ubuntu:16.04 sh -c \"cmake -DSTATIC=1 -DLIB_PREFIX_DIR=/usr/local/src/starcry . ; make clean\"
# ** ON HOLD ** #)
# ** ON HOLD ** #add_custom_target(
# ** ON HOLD ** #    build_ubuntu
# ** ON HOLD ** #    COMMAND docker run -i -t -v ${PROJECT_SOURCE_DIR}:/projects/starcry sc_build_ubuntu:16.04 sh -c \"cmake -DSTATIC=1 -DLIB_PREFIX_DIR=/usr/local/src/starcry . ; time make -j8 starcry\"
# ** ON HOLD ** #)
# ** ON HOLD ** #add_dependencies(starcry_ubuntu build_ubuntu)
# ** ON HOLD ** #
# ** ON HOLD ** 
