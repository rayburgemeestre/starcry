cmake_minimum_required(VERSION 2.6.2)
project(starcry)

include("cmake/gtestmockscript.cmake")

set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake")

if (EXISTS "/etc/centos-release")
#set(BOOST_ROOT "/projects/boost_1_60_0/")
endif (EXISTS "/etc/centos-release")

find_package(Allegro5 REQUIRED)
find_package(Boost 1.47.0 REQUIRED program_options system)
find_package(CAF REQUIRED core io)
set (V8_DIR "libs/v8pp/v8/")
find_package(V8 REQUIRED)
find_package(FFMPEG)
find_library(benchmarklib_LIBRARIES benchmarklib REQUIRED)

message(STATUS ${benchmarklib_LIBRARIES})

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
add_definitions(-O3 -Wall -funsigned-char -std=c++14 -fno-omit-frame-pointer -pedantic)
#add_definitions(-O0 -g -Wall -funsigned-char -std=c++14)

include_directories("./include/" "./libs/v8pp/v8/include" "./libs/v8pp/" "${V8_INCLUDE_DIR}")
if (EXISTS "/etc/centos-release")
    include_directories("/projects/boost_1_60_0/")
endif (EXISTS "/etc/centos-release")
link_directories("${PROJECT_SOURCE_DIR}/libs/v8pp/v8/lib/")
if (EXISTS "/etc/centos-release")
    link_directories("/projects/boost_1_60_0/stage/lib")
endif (EXISTS "/etc/centos-release")


#include_directories(${CAF_INCLUDE_DIRS})
file(GLOB_RECURSE starcry_SOURCES "src/**.cpp" "include/**.h*" "./libs/v8pp/v8pp/context.cpp")
add_executable(starcry ${starcry_SOURCES})
target_link_libraries(starcry ${Boost_LIBRARIES})
target_link_libraries(starcry ${ALLEGRO_LIBRARIES})
target_link_libraries(starcry ${benchmarklib_LIBRARIES})
target_link_libraries(starcry ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries(starcry ${CAF_LIBRARIES})
target_link_libraries(starcry ${FFMPEG_LIBRARIES})
if (EXISTS "/etc/centos-release")
    target_link_libraries(starcry -lz) # NEEDED FOR CENTOS7u2
    target_link_libraries(starcry -llzma) # NEEDED FOR CENTOS7u2
endif (EXISTS "/etc/centos-release")

# This order works on my Ubuntu 15.10 box at home *and* box at work.
target_link_libraries(starcry -lv8)
target_link_libraries(starcry -ldl)
target_link_libraries(starcry -licui18n)
target_link_libraries(starcry -licuuc)
target_link_libraries(starcry "${PROJECT_SOURCE_DIR}/libs/v8pp/v8/lib/libv8_libplatform.a")
target_link_libraries(starcry "${PROJECT_SOURCE_DIR}/libs/v8pp/v8/lib/libv8_base.a")
target_link_libraries(starcry "${PROJECT_SOURCE_DIR}/libs/v8pp/v8/lib/libv8_libbase.a")
target_link_libraries(starcry "${PROJECT_SOURCE_DIR}/libs/v8pp/v8/lib/libv8_nosnapshot.a")

# This order works on my Ubuntu 15.10 box at work only
#target_link_libraries(starcry "${PROJECT_SOURCE_DIR}/libs/v8pp/v8/lib/libv8_base.a")
#target_link_libraries(starcry "${PROJECT_SOURCE_DIR}/libs/v8pp/v8/lib/libv8_libbase.a")
#target_link_libraries(starcry "${PROJECT_SOURCE_DIR}/libs/v8pp/v8/lib/libv8_libplatform.a")
#target_link_libraries(starcry "${PROJECT_SOURCE_DIR}/libs/v8pp/v8/lib/libv8_nosnapshot.a")
#target_link_libraries(starcry -ldl)
#target_link_libraries(starcry -licui18n)
#target_link_libraries(starcry -licuuc)

install (TARGETS starcry DESTINATION bin)

# Test files
file(GLOB TEST_FILES "test/*.cpp" "./libs/v8pp/v8pp/context.cpp" "src/v8_wrapper.cpp")
# Don't include Main.cpp from src
file(GLOB MAIN_SRC "src/main.cpp")
#list(REMOVE_ITEM starcry_SOURCES ${MAIN_SRC})
add_executable(starcry_tests ${TEST_FILES})
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
target_link_libraries(starcry_tests "${PROJECT_SOURCE_DIR}/libs/v8pp/v8/lib/libv8_libplatform.a")
target_link_libraries(starcry_tests "${PROJECT_SOURCE_DIR}/libs/v8pp/v8/lib/libv8_base.a")
target_link_libraries(starcry_tests "${PROJECT_SOURCE_DIR}/libs/v8pp/v8/lib/libv8_libbase.a")
target_link_libraries(starcry_tests "${PROJECT_SOURCE_DIR}/libs/v8pp/v8/lib/libv8_nosnapshot.a")
target_link_libraries(starcry_tests -ldl)

add_test(NAME starcry COMMAND starcry_tests --unittests)
