cmake_minimum_required(VERSION 2.6.2)
 
project(starcry)

## begin gtest/gmock magic from http://www.kaizou.org/2014/11/gtest-cmake/ ##
find_package(Threads REQUIRED)
include(ExternalProject)
ExternalProject_Add(
        gtest
        URL https://googletest.googlecode.com/files/gtest-1.7.0.zip
        PREFIX ${CMAKE_CURRENT_BINARY_DIR}/gtest
        # Disable install step
        INSTALL_COMMAND ""
)
add_library(libgtest IMPORTED STATIC GLOBAL)
add_dependencies(libgtest gtest)
ExternalProject_Get_Property(gtest source_dir binary_dir)
set_target_properties(libgtest PROPERTIES
        "IMPORTED_LOCATION" "${binary_dir}/libgtest.a"
        "IMPORTED_LINK_INTERFACE_LIBRARIES" "${CMAKE_THREAD_LIBS_INIT}"
        #    "INTERFACE_INCLUDE_DIRECTORIES" "${source_dir}/include"
        )
include_directories("${source_dir}/include")
ExternalProject_Add(
        gmock
        URL https://googlemock.googlecode.com/files/gmock-1.7.0.zip
        PREFIX ${CMAKE_CURRENT_BINARY_DIR}/gmock
        # Disable install step
        INSTALL_COMMAND ""
)
add_library(libgmock IMPORTED STATIC GLOBAL)
add_dependencies(libgmock gmock)
ExternalProject_Get_Property(gmock source_dir binary_dir)
set_target_properties(libgmock PROPERTIES
        "IMPORTED_LOCATION" "${binary_dir}/libgmock.a"
        "IMPORTED_LINK_INTERFACE_LIBRARIES" "${CMAKE_THREAD_LIBS_INIT}"
        )
include_directories("${source_dir}/include")
## end gtest/gmock magic ##

set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake")

find_package(Allegro5 REQUIRED)
find_package(Boost 1.47.0 REQUIRED program_options system)
find_package(SFML 1 COMPONENTS system window graphics network audio REQUIRED)
find_package(CAF REQUIRED core)
#find_package(ZMQ)
#find_package(FFMPEG)
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

include_directories("./include/")
#include_directories(${CAF_INCLUDE_DIRS})
file(GLOB_RECURSE starcry_SOURCES "src/**.cpp")
add_executable(starcry ${starcry_SOURCES})
target_link_libraries(starcry ${Boost_LIBRARIES})
target_link_libraries(starcry ${SFML_LIBRARIES})
target_link_libraries(starcry ${ALLEGRO_LIBRARIES})
#target_link_libraries(starcry ${ZMQ_LIBRARIES})
#target_link_libraries(starcry ${FFMPEG_LIBRARIES})
target_link_libraries(starcry ${benchmarklib_LIBRARIES})
target_link_libraries(starcry ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries(starcry ${CAF_LIBRARIES})

install (TARGETS starcry DESTINATION bin)

# Test files
file(GLOB TEST_FILES "test/*.cpp")
# Don't include Main.cpp from src
file(GLOB MAIN_SRC "src/main.cpp")
list(REMOVE_ITEM starcry_SOURCES ${MAIN_SRC})
add_executable(starcry_tests ${TEST_FILES} ${SRC_FILES})
# Set link libraries (order matters)
target_link_libraries(starcry_tests libgtest)
target_link_libraries(starcry_tests libgmock)
target_link_libraries(starcry_tests ${Boost_LIBRARIES})
target_link_libraries(starcry_tests ${SFML_LIBRARIES})
target_link_libraries(starcry_tests ${ALLEGRO_LIBRARIES})
#target_link_libraries(starcry ${ZMQ_LIBRARIES})
#target_link_libraries(starcry ${FFMPEG_LIBRARIES})
target_link_libraries(starcry_tests ${benchmarklib_LIBRARIES})
target_link_libraries(starcry_tests)
target_link_libraries(starcry_tests ${CMAKE_THREAD_LIBS_INIT})

add_test(NAME starcry COMMAND starcry_tests --unittests)
