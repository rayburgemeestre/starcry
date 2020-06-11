#cmake_minimum_required(VERSION 3.15.3)
cmake_minimum_required(VERSION 3.10.2)
project(starcry VERSION 0.2 LANGUAGES CXX)
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
    #set (CLANG_FLAGS "-I/home/trigen/projects/build-config/tmp/v8/buildtools/third_party/libc++/trunk/include/ -stdlib=libc++")
    # don't use this for the time being..
    #set (CLANG_FLAGS "-stdlib=libc++")
    #set (CLANG_FLAGS "")
else()
    #set (CLANG_FLAGS "-I/home/trigen/projects/build-config/tmp/v8/buildtools/third_party/libc++/trunk/include/ -stdlib=libc++")
    #set (CLANG_FLAGS "-stdlib=libc++")
endif()

if (NOT DEBUG)
    #add_definitions("-std=c++17 -O3 -funsigned-char -Wall ${CLANG_FLAGS} -fno-omit-frame-pointer -pedantic -mssse3 -Wno-reserved-user-defined-literal")
else()
    #add_definitions("-std=c++17 -O0 -g -fno-inline -Wall ${CLANG_FLAGS} -funsigned-char -fno-omit-frame-pointer -pedantic -mssse3 -Wno-reserved-user-defined-literal")
endif()
#add_definitions("-std=c++17 -O0 -g ${CLANG_FLAGS} -D_LIBCPP_ABI_UNSTABLE=1 -D_LIBCPP_ABI_VERSION=1")
#add_definitions("-std=c++17 -O0 -g ${CLANG_FLAGS} ")
#add_definitions("-std=c++17 -O0 -g -D_GLIBCXX_USE_CXX11_ABI=0 -stdlib=libstdc++")

# works
#add_definitions("-std=c++17 -O0 -g -stdlib=libstdc++ -DV8_COMPRESS_POINTERS")
# now with optimiz
#add_definitions("-std=c++17 -O3 -funsigned-char -Wall -stdlib=libstdc++ -fno-omit-frame-pointer -pedantic -mssse3 -Wno-reserved-user-defined-literal -DV8_COMPRESS_POINTERS")
#
#add_definitions("-std=c++17 -O0 -g -funsigned-char -Wall -stdlib=libstdc++ -fno-omit-frame-pointer -pedantic -mssse3 -Wno-reserved-user-defined-literal -DV8_COMPRESS_POINTERS")
#add_definitions("-std=c++17 -O0 -g -funsigned-char -Wall -fno-omit-frame-pointer -pedantic -mssse3 -Wno-reserved-user-defined-literal -DV8_COMPRESS_POINTERS -Wl,-z,notext")



#add_definitions("-std=c++17 -O0 -g -DV8_COMPRESS_POINTERS")  # --no-rosegment is important for being able to let valgrind/callgrind read symbols
add_definitions("-std=c++17 -O3 -funsigned-char -Wall -stdlib=libstdc++ -fno-omit-frame-pointer -pedantic -mssse3 -Wno-reserved-user-defined-literal -DV8_COMPRESS_POINTERS")
set(CMAKE_EXE_LINKER_FLAGS "-Wl,--no-rosegment")




##add_definitions("-std=c++17 -O0 -g ${CLANG_FLAGS} -D_LIBCPP_ABI_UNSTABLE= -D_LIBCPP_ABI_VERSION=1")
#add_definitions("-std=c++17 -O0 -g ${CLANG_FLAGS}")

#add_definitions("-std=c++17 -O0 -g -stdlib=libc++")
#this compioled the hello world correctly
#cd /home/trigen/projects/build-config/caf/actor-framework/build/examples && /usr/bin/c++   -I/home/trigen/projects/build-config/caf/actor-framework/libcaf_opencl -I/home/trigen/projects/build-config/caf/actor-framework/libcaf_core -I/home/trigen/projects/build-config/caf/actor-framework/libcaf_io -I/home/trigen/projects/build-config/caf/actor-framework/libcaf_test -I/home/trigen/projects/build-config/caf/actor-framework/libcaf_openssl -I/home/trigen/projects/build-config/caf/actor-framework/build/libcaf_core  -std=c++11 -Wextra -Wall -pedantic -ftemplate-depth=512 -ftemplate-backtrace-limit=0 -Wno-missing-field-initializers -pthread -fPIC -DGLIBCXX_USE_CXX11_ABI=1 -D_GLIBCXX_USE_CXX11_ABI=1 -O2 -g -DNDEBUG   -o CMakeFiles/hello_world.dir/hello_world.cpp.o -c /home/trigen/projects/build-config/caf/actor-framework/examples/hello_world.cpp
# replicate attempt
#add_definitions("-std=c++17 -Wextra -Wall -pedantic -ftemplate-depth=512 -ftemplate-backtrace-limit=0 -Wno-missing-field-initializers -pthread -fPIC -DGLIBCXX_USE_CXX11_ABI=1 -D_GLIBCXX_USE_CXX11_ABI=1 -O2 -g -DNDEBUG")
#
# the linking bit
#/usr/bin/c++  -std=c++11 -Wextra -Wall -pedantic -ftemplate-depth=512 -ftemplate-backtrace-limit=0 -Wno-missing-field-initializers -pthread -fPIC -DGLIBCXX_USE_CXX11_ABI=1 -D_GLIBCXX_USE_CXX11_ABI=1 -O2 -g -DNDEBUG  -rdynamic CMakeFiles/hello_world.dir/hello_world.cpp.o  -o ../bin/hello_world  ../lib/libcaf_core_static.a ../lib/libcaf_io_static.a ../lib/libcaf_openssl_static.a ../lib/libcaf_io_static.a ../lib/libcaf_core_static.a -lssl -lcrypto
# turns out the fucker doesn't use libc++ ???? even though the output says so



#directly from caf.
#-std=c++11 -Wextra -Wall -pedantic -ftemplate-depth=512 -ftemplate-backtrace-limit=0 -stdlib=libc++ -pthread -fPIC -DGLIBCXX_USE_CXX11_ABI=1 -D_GLIBCXX_USE_CXX11_ABI=1 -O2 -g -DNDEBUG

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

link_directories("/usr/lib/llvm-10/lib/")

file(GLOB_RECURSE starcry_SOURCES "src/**.cpp" "include/**.h*" "/opt/cppse/build/v8pp/include/v8pp/context.cpp")
# (below) useful for testing v8 only
#file(GLOB_RECURSE starcry_SOURCES "src/main.cpp" "/opt/cppse/build/v8pp/include/v8pp/context.cpp")
# (below) useful for testing caf only
#file(GLOB_RECURSE starcry_SOURCES "src/main.cpp")

add_executable(starcry ${starcry_SOURCES})

# Optional IPO. Do not use IPO if it's not supported by compiler.
#check_ipo_supported(RESULT result OUTPUT output)
#if(result)
#    set_property(TARGET starcry PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
#else()
#    message(WARNING "IPO is not supported: ${output}")
#endif()

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

# crtmpserver
target_link_libraries(starcry /opt/cppse/build/crtmpserver/lib/libcrtmpserver.a)
target_link_libraries(starcry /opt/cppse/build/crtmpserver/lib/libappselector.a)
target_link_libraries(starcry /opt/cppse/build/crtmpserver/lib/libflvplayback.a)
target_link_libraries(starcry /opt/cppse/build/crtmpserver/lib/libthelib.a)
target_link_libraries(starcry /opt/cppse/build/crtmpserver/lib/libcommon.a)
target_link_libraries(starcry /opt/cppse/build/crtmpserver/lib/libtinyxml.a)
target_link_libraries(starcry /opt/cppse/build/crtmpserver/lib/liblua.a)

# boost
target_link_libraries(starcry ${Boost_LIBRARIES})

# pthreads
target_link_libraries(starcry ${CMAKE_THREAD_LIBS_INIT})

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
target_link_libraries(starcry -lXcursor)
target_link_libraries(starcry -lXrender)
target_link_libraries(starcry -lxcb)
target_link_libraries(starcry /opt/cppse/build/x264/lib/libx264.a)
target_link_libraries(starcry -lbz2)
target_link_libraries(starcry -lz)
target_link_libraries(starcry -llzma)
target_link_libraries(starcry -ldl)
target_link_libraries(starcry -lswresample -lm -lz)
target_link_libraries(starcry -lssl)
target_link_libraries(starcry -lcrypto)

# allegro
# target_link_libraries(starcry ${ALLEGRO_LIBRARIES})
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
target_link_libraries(starcry /opt/cppse/build/fastpfor/lib/libFastPFor.a)

#target_link_libraries(starcry /opt/cppse/build/v8pp/lib/libv8_monolith.a)
#target_link_libraries(starcry /opt/cppse/build/v8pp/lib/v8.a)

# v8
target_link_libraries(starcry /opt/cppse/build/v8pp/lib/libv8_monolith.a)
target_link_libraries(starcry -ldl)
target_link_libraries(starcry /usr/lib/gcc/x86_64-linux-gnu/8/libstdc++fs.a)

#target_link_libraries(starcry /opt/cppse/build/v8pp/lib/libv8_monolith.a)
# NO: target_link_libraries(starcry /opt/cppse/build/v8pp/lib/libc++.so)
#target_link_libraries(starcry /opt/cppse/build/v8pp/lib/v8.a)
#target_link_libraries(starcry /opt/cppse/build/v8pp/lib/libv8_libbase.so)
#target_link_libraries(starcry /opt/cppse/build/v8pp/lib/libv8_libplatform.so)
#target_link_libraries(starcry /opt/cppse/build/v8pp/lib/libicuuc.so)
#target_link_libraries(starcry /opt/cppse/build/v8pp/lib/libicui18n.so)
#target_link_libraries(starcry /opt/cppse/build/v8pp/lib/libv8.so)
#target_link_libraries(starcry /usr/lib/llvm-9/lib/libc++.a)
#target_link_libraries(starcry /usr/lib/llvm-9/lib/libc++abi.so.1.0)
#target_link_libraries(starcry /opt/cppse/build/v8pp/lib/v8.a)
#target_link_libraries(starcry /opt/cppse/build/v8pp/lib/libc++.so)
#target_link_libraries(starcry /home/trigen/projects/build-config/v8pp/v8/build/linux/debian_sid_amd64-sysroot/usr/lib/gcc/x86_64-linux-gnu/6/libstdc++fs.a)
#target_link_libraries(starcry /opt/cppse/build/v8pp/lib/libv8_libplatform.so)
#target_link_libraries(starcry /opt/cppse/build/v8pp/lib/libv8_libbase.so)
#target_link_libraries(starcry /opt/cppse/build/v8pp/lib/libicuuc.so)
#target_link_libraries(starcry /opt/cppse/build/v8pp/lib/libicui18n.so)
#target_link_libraries(starcry /opt/cppse/build/v8pp/lib/libv8.so)
#target_link_libraries(starcry /usr/lib/llvm-9/lib/libc++.a)
#target_link_libraries(starcry /usr/lib/llvm-9/lib/libc++abi.so.1.0)
#target_link_libraries(starcry /home/trigen/projects/starcry/libstdc++fs.a)
#target_link_libraries(starcry -lstdc++fs)

# caf?
#target_link_libraries(starcry /usr/lib/llvm-9/lib/libc++.a)
#target_link_libraries(starcry /usr/lib/llvm-9/lib/libc++.so.1)
#target_link_libraries(starcry /usr/lib/llvm-9/lib/libc++abi.so.1.0)
#target_link_libraries(starcry /opt/cppse/build/v8pp/lib/libc++.so)
#target_link_libraries(starcry -lc++)
#target_link_libraries(starcry -lc++abi)
#target_link_libraries(starcry /usr/lib/llvm-9/lib/libc++abi.so.1.0)
#target_link_libraries(starcry /usr/lib/gcc/x86_64-linux-gnu/7/libstdc++.a)

#/usr/lib/x86_64-linux-gnu/liblldb-9.so
#/usr/lib/x86_64-linux-gnu/libLLVM-9.so
#/usr/lib/gcc/x86_64-linux-gnu/7/libstdc++.so
#/usr/lib/llvm-9/lib/libLLVM.so
#/usr/lib/llvm-9/lib/libLLVM-9.0.0.so
#/usr/lib/llvm-9/lib/LLVMPolly.so
#/usr/lib/llvm-9/lib/libLLVM-9.so
#/usr/lib/llvm-9/lib/python2.7/site-packages/lldb/_lldb.so

install (TARGETS starcry DESTINATION bin)
