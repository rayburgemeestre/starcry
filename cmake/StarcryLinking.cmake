if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  # set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fuse-ld=gold")
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fuse-ld=mold")
else()
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-z,notext -Wno-unknown-pragmas")
endif()

function(configure_links target)
  # boost
  target_link_libraries(${target} PRIVATE ${Boost_LIBRARIES})

  # redis
  target_link_libraries(${target} PRIVATE /opt/cppse/build/redis-plus-plus/lib/libredis++.a)
  target_link_libraries(${target} PRIVATE /opt/cppse/build/hiredis/lib/libhiredis.a)

  # vivid
  target_link_libraries(${target} PRIVATE /opt/cppse/build/vivid/lib/libvivid.a)

  # ffmpeg
  target_link_libraries(${target} PRIVATE /opt/cppse/build/ffmpeg/lib/libswscale.a)
  target_link_libraries(${target} PRIVATE /opt/cppse/build/ffmpeg/lib/libavdevice.a)
  target_link_libraries(${target} PRIVATE /opt/cppse/build/ffmpeg/lib/libavformat.a)
  target_link_libraries(${target} PRIVATE /opt/cppse/build/ffmpeg/lib/libavcodec.a)
  target_link_libraries(${target} PRIVATE /opt/cppse/build/ffmpeg/lib/libavutil.a)
  target_link_libraries(${target} PRIVATE /opt/cppse/build/ffmpeg/lib/libavfilter.a)
  target_link_libraries(${target} PRIVATE /opt/cppse/build/ffmpeg/lib/libswresample.a)

  # X11 (statically linking causes SFML to segfault, no longer using SFML though)
  target_link_libraries(${target} PRIVATE /usr/lib/x86_64-linux-gnu/libX11.a)
  target_link_libraries(${target} PRIVATE /usr/lib/x86_64-linux-gnu/libxcb.a)
  target_link_libraries(${target} PRIVATE /usr/lib/x86_64-linux-gnu/libXau.a)
  target_link_libraries(${target} PRIVATE /usr/lib/x86_64-linux-gnu/libXdmcp.a)

  # x264
  target_link_libraries(${target} PRIVATE /opt/cppse/build/x264/lib/libx264.a)
  target_link_libraries(${target} PRIVATE /usr/lib/x86_64-linux-gnu/libbz2.a)
  target_link_libraries(${target} PRIVATE /usr/lib/x86_64-linux-gnu/liblzma.a)

  # target_link_libraries(${target} PRIVATE -ldl)
  target_link_libraries(${target} PRIVATE /usr/lib/x86_64-linux-gnu/libdl.a)

  target_link_libraries(${target} PRIVATE /usr/lib/x86_64-linux-gnu/libpng16.a)

  # fastpfor
  target_link_libraries(${target} PRIVATE /opt/cppse/build/fastpfor/lib/libFastPFOR.a)

  # v8
  target_link_libraries(${target} PRIVATE /opt/cppse/build/v8/lib/libv8_monolith.a)
  #target_link_libraries(${target} PRIVATE /usr/lib/gcc/x86_64-linux-gnu/7/libstdc++fs.a) # ubuntu 18.04
  #target_link_libraries(${target} PRIVATE /usr/lib/gcc/x86_64-linux-gnu/9/libstdc++fs.a)  # ubuntu 20.04
  #target_link_libraries(${target} PRIVATE /usr/lib/gcc/x86_64-linux-gnu/11/libstdc++fs.a)  # ubuntu 22.04
  target_link_libraries(${target} PRIVATE /usr/lib/gcc/x86_64-linux-gnu/13/libstdc++fs.a)  # ubuntu 24.04

  # generic
  target_link_libraries(${target} PRIVATE /usr/lib/x86_64-linux-gnu/libssl.a)
  target_link_libraries(${target} PRIVATE /usr/lib/x86_64-linux-gnu/libcrypto.a)

  # seasocks
  target_link_libraries(${target} PRIVATE /opt/cppse/build/seasocks/lib/libseasocks.a)

  # openexr
  target_link_libraries(${target} PRIVATE /opt/cppse/build/openexr/lib/libOpenEXR-3_3.a)
  target_link_libraries(${target} PRIVATE /opt/cppse/build/openexr/lib/libOpenEXRCore-3_3.a)
  target_link_libraries(${target} PRIVATE /opt/cppse/build/openexr/lib/libOpenEXRUtil-3_3.a)
  target_link_libraries(${target} PRIVATE /opt/cppse/build/openexr/lib/libIlmThread-3_3.a)
  target_link_libraries(${target} PRIVATE /opt/cppse/build/openexr/lib/libImath-3_1.a)
  target_link_libraries(${target} PRIVATE /opt/cppse/build/openexr/lib/libIex-3_3.a)
  target_link_libraries(${target} PRIVATE /usr/lib/x86_64-linux-gnu/libz.a)

  # curses
  target_link_libraries(${target} PRIVATE /usr/lib/x86_64-linux-gnu/libcurses.a)
  target_link_libraries(${target} PRIVATE /usr/lib/x86_64-linux-gnu/libtinfo.a)

  # fmt
  target_link_libraries(${target} PRIVATE /opt/cppse/build/fmt/lib/libfmt.a)

  # tvision
  target_link_libraries(${target} PRIVATE /opt/cppse/build/tvision/lib/libtvision.a)

  # ncursesw
  target_link_libraries(${target} PRIVATE /usr/lib/x86_64-linux-gnu/libncurses.a)
  target_link_libraries(${target} PRIVATE /usr/lib/x86_64-linux-gnu/libncursesw.a)
  target_link_libraries(${target} PRIVATE /usr/lib/x86_64-linux-gnu/libtermcap.a)

  # inotify
  target_link_libraries(${target} PRIVATE /opt/cppse/build/inotify-cpp/lib/libinotify-cpp.a)
endfunction()
