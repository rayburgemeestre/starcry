if((NOT BENCHMARKLIB_STATIC_LIBRARY) OR (NOT EXISTS ${BENCHMARKLIB_STATIC_LIBRARY}))
    message("Unable to find benchmarklib, cloning...")
    execute_process(COMMAND git submodule update --init -- libs/benchmarklib
                    WORKING_DIRECTORY ${LIB_PREFIX_DIR})
    set(BENCHMARKLIB_INCLUDE_DIR ${LIB_PREFIX_DIR}/libs/benchmarklib
        CACHE PATH "Benchmarklib include path")
    execute_process(COMMAND bash prepare_linux.sh benchmarklib
                    WORKING_DIRECTORY ${LIB_PREFIX_DIR})
    set(BENCHMARKLIB_STATIC_LIBRARY ${LIB_PREFIX_DIR}/libs/benchmarklib/libbenchmarklib.a
        CACHE PATH "Benchmarklib static library")
else()
    message("Benchmarklib is ready")
endif()

if((NOT CRTMPSERVER_INCLUDE_DIR) OR (NOT EXISTS ${CRTMPSERVER_INCLUDE_DIR}))
    message("Unable to find crtmpserver, cloning...")
    execute_process(COMMAND git submodule update --init -- libs/crtmpserver
                    WORKING_DIRECTORY ${LIB_PREFIX_DIR})
    set(CRTMPSERVER_INCLUDE_DIR ${LIB_PREFIX_DIR}/libs/crtmpserver
        CACHE PATH "Crtmpserver include path")
else()
    message("Crtmpserver is ready")
endif()

set(CRTMPSERVER_STATIC_LIBRARIES "")
foreach(crtmpserver_lib crtmpserver/libcrtmpserver.a
						applications/appselector/libappselector.a
						applications/flvplayback/libflvplayback.a
						thelib/libthelib.a
						common/libcommon.a
						tinyxml/libtinyxml.a
						lua/liblua.a)

	set(CRTMPSERVER_CMAKE_BUILD_DIR ${LIB_PREFIX_DIR}/libs/crtmpserver/builders/cmake
		CACHE PATH "Crtmpserver cmake build path")

	if((NOT CRTMPSERVER_CMAKE_BUILD_DIR) OR (NOT EXISTS "${CRTMPSERVER_CMAKE_BUILD_DIR}/${crtmpserver_lib}"))
		execute_process(COMMAND bash prepare_linux.sh crtmpserver
		                WORKING_DIRECTORY ${CRTMPSERVER_INCLUDE_DIR})
	endif()

	set(CRTMPSERVER_STATIC_LIBRARIES "${CRTMPSERVER_STATIC_LIBRARIES};${CRTMPSERVER_CMAKE_BUILD_DIR}/${crtmpserver_lib}")
	message("Crtmpserver is ready: ${CRTMPSERVER_STATIC_LIBRARIES}")

endforeach(crtmpserver_lib)
set(CRTMPSERVER_STATIC_LIBRARIES ${CRTMPSERVER_STATIC_LIBRARIES}
	CACHE PATH "Crtmpserver static libraries")

