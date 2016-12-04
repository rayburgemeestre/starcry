cmake_minimum_required(VERSION 2.6.2)
project(starcry)

include("cmake/gtestmockscript.cmake")

set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake")

# use cmake -DSTATIC=1 to enable static linking..
if (STATIC)
    message(STATUS "Configuring for static...")
    set(Boost_USE_STATIC_LIBS   ON)
else()
    message(STATUS "Configuring for non-static..")
endif()

if (NOT LIB_PREFIX_DIR)
  set (LIB_PREFIX_DIR "${PROJECT_SOURCE_DIR}/")
endif()

if (NOT BOOST_ROOT)
    set(BOOST_ROOT "${LIB_PREFIX_DIR}/boost_1_61_0/")
endif()

if (STATIC)
    find_package(Allegro5static REQUIRED)
else()
    find_package(Allegro5 REQUIRED)
endif()

#find_package(Boost 1.47.0 REQUIRED program_options system)
find_package(CAF REQUIRED core io)
set (V8_DIR "${LIB_PREFIX_DIR}/libs/v8pp/v8/")
find_package(V8 REQUIRED)
find_package(FFMPEG)

#find_library(benchmarklib_LIBRARIES libbenchmarklib REQUIRED)
find_library(benchmarklib_LIBRARIES NAMES libbenchmarklib.a REQUIRED)
find_library(crtmpserver_LIBRARIES NAMES libcrtmpserver.a REQUIRED)

#crow
if (MSVC)
    set(Boost_USE_STATIC_LIBS "On")
    find_package( Boost 1.52 COMPONENTS program_options system date_time filesystem system thread regex REQUIRED )
else()
    #set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -std=c++1y")
    # added chrono as it's needed in case of static libbenchmarklib.a
    find_package( Boost 1.52 COMPONENTS program_options system date_time filesystem thread chrono REQUIRED )
endif()


message(STATUS ${FFMPEG_LIBRARIES})
message(STATUS ${benchmarklib_LIBRARIES})
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
    set (COMPILE_FLAGS "-O3 -Wall -funsigned-char -std=c++14 -fno-omit-frame-pointer -pedantic -mssse3")
else()
    set (COMPILE_FLAGS "-O0 -g -fno-inline -Wall -funsigned-char -std=c++14 -fno-omit-frame-pointer -pedantic")
endif()
add_definitions(${COMPILE_FLAGS})

include_directories("./include/" "${V8_DIR}/include" "${V8_DIR}/../" "${V8_INCLUDE_DIR}" "${LIB_PREFIX_DIR}/libs/allegro5/include/" "${LIB_PREFIX_DIR}/libs/crow/include" "${LIB_PREFIX_DIR}/libs/FastPFor/")
#if (EXISTS "/etc/centos-release")
include_directories(${BOOST_ROOT})
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
target_link_libraries(starcry ${benchmarklib_LIBRARIES})

#target_link_libraries(starcry ${crtmpserver_LIBRARIES})
target_link_libraries(starcry ${LIB_PREFIX_DIR}/libs/crtmpserver/builders/cmake/crtmpserver/libcrtmpserver.a)

#target_link_libraries(starcry ${LIB_PREFIX_DIR}/libs/crtmpserver/builders/cmake/applications/admin/libadmin.a)
#target_link_libraries(starcry ${LIB_PREFIX_DIR}/libs/crtmpserver/builders/cmake/applications/appselector/libappselector.a)
#target_link_libraries(starcry ${LIB_PREFIX_DIR}/libs/crtmpserver/builders/cmake/applications/flvplayback/libflvplayback.a)
#target_link_libraries(starcry ${LIB_PREFIX_DIR}/libs/crtmpserver/builders/cmake/applications/proxypublish/libproxypublish.a)
#target_link_libraries(starcry ${LIB_PREFIX_DIR}/libs/crtmpserver/builders/cmake/applications/samplefactory/libsamplefactory.a)
#target_link_libraries(starcry ${LIB_PREFIX_DIR}/libs/crtmpserver/builders/cmake/applications/stresstest/libstresstest.a)
#target_link_libraries(starcry ${LIB_PREFIX_DIR}/libs/crtmpserver/builders/cmake/applications/vptests/libvptests.a)
target_link_libraries(starcry ${LIB_PREFIX_DIR}/libs/crtmpserver/builders/cmake/applications/appselector/libappselector.a)
target_link_libraries(starcry ${LIB_PREFIX_DIR}/libs/crtmpserver/builders/cmake/applications/flvplayback/libflvplayback.a)

target_link_libraries(starcry ${LIB_PREFIX_DIR}/libs/crtmpserver/builders/cmake/thelib/libthelib.a)
target_link_libraries(starcry ${LIB_PREFIX_DIR}/libs/crtmpserver/builders/cmake/common/libcommon.a)
target_link_libraries(starcry ${LIB_PREFIX_DIR}/libs/crtmpserver/builders/cmake/tinyxml/libtinyxml.a)
target_link_libraries(starcry ${LIB_PREFIX_DIR}/libs/crtmpserver/builders/cmake/lua/liblua.a)
target_link_libraries(starcry -lssl)
target_link_libraries(starcry -lcrypto)
target_link_libraries(starcry -lstdc++fs)
#/usr/lib/x86_64-linux-gnu/libcrypto.so
#/usr/lib/x86_64-linux-gnu/libz.a
#/lib/x86_64-linux-gnu/libdl.so.2

target_link_libraries(starcry ${Boost_LIBRARIES})
target_link_libraries(starcry ${CMAKE_THREAD_LIBS_INIT})
if (STATIC)
    target_link_libraries(starcry "/usr/local/lib/libcaf_io_static.a")
    target_link_libraries(starcry "/usr/local/lib/libcaf_core_static.a")
    target_link_libraries(starcry "/usr/local/lib/libavformat.a")
    target_link_libraries(starcry -lbz2)
    target_link_libraries(starcry "/usr/local/lib/libavcodec.a")
    target_link_libraries(starcry "/usr/local/lib/libavutil.a")
    target_link_libraries(starcry "/usr/local/lib/libavfilter.a")
    target_link_libraries(starcry "/usr/local/lib/libswscale.a")
    target_link_libraries(starcry "/usr/local/lib/libx264.a") # :-)
else()
    target_link_libraries(starcry ${CAF_LIBRARIES})
    target_link_libraries(starcry ${FFMPEG_LIBRARIES})
endif()
target_link_libraries(starcry "${LIB_PREFIX_DIR}/libs/FastPFor/libFastPFor.a")
target_link_libraries(starcry -lz) # NEEDED FOR CENTOS7u2, && Ubuntu it seems?
target_link_libraries(starcry -llzma)

# This order works on my Ubuntu 15.10 box at home *and* box at work.

target_link_libraries(starcry -ldl)


target_link_libraries(starcry "${V8_DIR}/lib/libv8_base.a")
target_link_libraries(starcry -lv8_libbase)
target_link_libraries(starcry "${V8_DIR}/lib/libv8_nosnapshot.a")
#target_link_libraries(starcry "${V8_DIR}/lib/libv8_libplatform.a")
target_link_libraries(starcry -lv8_libplatform) # libv8_libplatform.a seems no longer available?
target_link_libraries(starcry "${V8_DIR}/lib/libv8_libsampler.a")
target_link_libraries(starcry -licui18n)
target_link_libraries(starcry -licuuc)
#message(STATUS "${V8_DIR}/lib/libv8_base.a")
#target_link_libraries(starcry "${V8_DIR}/lib/libv8_libbase.a")

if (STATIC)
    target_link_libraries(starcry ${ALLEGRO_LIBRARIES})
    target_link_libraries(starcry -lX11)
    target_link_libraries(starcry -lXcursor)
    target_link_libraries(starcry -ljpeg)
    target_link_libraries(starcry -lpng)
    target_link_libraries(starcry -lGLU)
    target_link_libraries(starcry -lGL)
    target_link_libraries(starcry -lfreetype)
    if (NOT EXISTS "/etc/centos-release")
        target_link_libraries(starcry -lXrandr)
        target_link_libraries(starcry -lXinerama)
        target_link_libraries(starcry -lXi)
        #xrandr xi xxf86vm gl
    endif()
    #END
    #-lX11 -ltiff -ljpeg -lpng -llapack -lGLU -lGL -ldc1394  -L -lXext -pthread
else()
    target_link_libraries(starcry ${ALLEGRO_LIBRARIES})
endif()

# This order works on my Ubuntu 15.10 box at work only
#target_link_libraries(starcry "${V8_DIR}/lib/libv8_base.a")
#target_link_libraries(starcry "${V8_DIR}/lib/libv8_libbase.a")
#target_link_libraries(starcry "${V8_DIR}/lib/libv8_libplatform.a")
#target_link_libraries(starcry "${V8_DIR}/lib/libv8_nosnapshot.a")
#target_link_libraries(starcry -ldl)
#target_link_libraries(starcry -licui18n)
#target_link_libraries(starcry -licuuc)

install (TARGETS starcry DESTINATION bin)

###################################################### Light renderer
# let's always link the worker statically... for convenience.
#target_compile_options(starcry_worker PUBLIC  -DWORKER_ONLY=1)
file(GLOB_RECURSE starcry_worker_SOURCES "src/main.cpp" "src/renderer.cpp" "src/rendering_engine_wrapper.cpp" "src/util/compress_vector.cpp" "src/util/remote_actors.cpp")
# Don't include Main.cpp from src
file(GLOB MAIN_SRC "src/main.cpp")
add_executable(starcry_worker ${starcry_worker_SOURCES})
set_target_properties (starcry_worker PROPERTIES COMPILE_FLAGS "-DWORKER_ONLY=1")
install (TARGETS starcry_worker DESTINATION bin)
target_link_libraries(starcry_worker "${PROJECT_SOURCE_DIR}/libs/FastPFor/libFastPFor.a")
target_link_libraries(starcry_worker ${benchmarklib_LIBRARIES})
target_link_libraries(starcry_worker ${Boost_LIBRARIES})
target_link_libraries(starcry_worker ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries(starcry_worker "/usr/local/lib/libcaf_io_static.a")
target_link_libraries(starcry_worker "/usr/local/lib/libcaf_core_static.a")
if (EXISTS "/etc/centos-release")
    target_link_libraries(starcry_worker -lz) # NEEDED FOR CENTOS7u2
    target_link_libraries(starcry_worker -llzma) # NEEDED FOR CENTOS7u2
endif (EXISTS "/etc/centos-release")
# This order works on my Ubuntu 15.10 box at home *and* box at work.
#target_link_libraries(starcry_worker -lv8)
target_link_libraries(starcry_worker -ldl)
target_link_libraries(starcry_worker -lX11)
target_link_libraries(starcry_worker -lXcursor)
target_link_libraries(starcry_worker -ljpeg)
target_link_libraries(starcry_worker -lpng)
target_link_libraries(starcry_worker -lGLU)
target_link_libraries(starcry_worker -lGL)
target_link_libraries(starcry_worker -lfreetype)
if (NOT EXISTS "/etc/centos-release")
    target_link_libraries(starcry_worker -lXrandr)
    target_link_libraries(starcry_worker -lXinerama)
    target_link_libraries(starcry_worker -lXi)
    #xrandr xi xxf86vm gl
endif()
target_link_libraries(starcry_worker ${ALLEGRO_LIBRARIES})
#######################################################

# Test files, always dynamic.
file(GLOB_RECURSE starcry_test_SOURCES "src/primitives/**.cpp")
file(GLOB TEST_FILES "test/*.cpp" "${LIB_PREFIX_DIR}/libs/v8pp/v8pp/context.cpp" "src/v8_wrapper.cpp")
# Don't include Main.cpp from src
file(GLOB MAIN_SRC "src/main.cpp")
#list(REMOVE_ITEM starcry_SOURCES ${MAIN_SRC})
add_executable(starcry_tests ${TEST_FILES} ${starcry_test_SOURCES})
# Set link libraries (order matters)
target_link_libraries(starcry_tests libgtest)
target_link_libraries(starcry_tests libgmock)
target_link_libraries(starcry_tests ${Boost_LIBRARIES})
target_link_libraries(starcry_tests ${SFML_LIBRARIES})
target_link_libraries(starcry_tests ${ALLEGRO_LIBRARIES})
target_link_libraries(starcry_tests ${benchmarklib_LIBRARIES})
target_link_libraries(starcry_tests ${CMAKE_THREAD_LIBS_INIT})
# V8 dependencies (the one provided by Ubuntu is not new enough for v8pp)
target_link_libraries(starcry_tests -lv8)
target_link_libraries(starcry_tests -licui18n)
target_link_libraries(starcry_tests -licuuc)
target_link_libraries(starcry_tests "${V8_DIR}/lib/libv8_libplatform.a")
target_link_libraries(starcry_tests "${V8_DIR}/lib/libv8_base.a")
#target_link_libraries(starcry_tests "${V8_DIR}/lib/libv8_libbase.a")
target_link_libraries(starcry_tests "${V8_DIR}/lib/libv8_nosnapshot.a")
target_link_libraries(starcry_tests -ldl)

add_test(NAME starcry COMMAND starcry_tests --unittests)

#add_executable(starcry_ubuntu)
#add_custom_target(
#    clean_ubuntu
#    COMMAND docker run -i -t -v ${PROJECT_SOURCE_DIR}:/projects/starcry sc_build_ubuntu:16.04 sh -c \"cmake -DSTATIC=1 -DLIB_PREFIX_DIR=/usr/local/src/starcry . ; make clean\"
#)
#add_custom_target(
#    build_ubuntu
#    COMMAND docker run -i -t -v ${PROJECT_SOURCE_DIR}:/projects/starcry sc_build_ubuntu:16.04 sh -c \"cmake -DSTATIC=1 -DLIB_PREFIX_DIR=/usr/local/src/starcry . ; time make -j8 starcry\"
#)
#add_dependencies(starcry_ubuntu build_ubuntu)
#

