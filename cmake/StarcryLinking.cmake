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
  # The gcc version in this path changes with every Ubuntu release (7 on 18.04, 9 on
  # 20.04, 11 on 22.04, 13 on 24.04, 15/16 on 26.04). Hardcoding it meant one edit per
  # release and a tree that only built on that one release, so pick the newest that is
  # actually installed instead.
  file(GLOB _stdcxxfs_candidates /usr/lib/gcc/x86_64-linux-gnu/*/libstdc++fs.a)
  if(NOT _stdcxxfs_candidates)
    message(FATAL_ERROR "no libstdc++fs.a found under /usr/lib/gcc/x86_64-linux-gnu/*/")
  endif()
  list(SORT _stdcxxfs_candidates COMPARE NATURAL ORDER DESCENDING)
  list(GET _stdcxxfs_candidates 0 _stdcxxfs)
  message(STATUS "Using ${_stdcxxfs}")
  target_link_libraries(${target} PRIVATE ${_stdcxxfs})

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
