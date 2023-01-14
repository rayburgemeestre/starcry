SHELL:=/bin/bash

uid:=$(shell id -u)
gid:=$(shell id -g)
interactive:=$(shell [ -t 0 ] && echo 1)
ccache_enabled = [[ -f '/usr/bin/ccache' ]]
ccache_env = CXX='ccache g++' CC='ccache gcc' CCACHE_SLOPPINESS=file_macro,locale,time_macros
docker_tty = $$(/bin/sh -c 'if [ "$(interactive)" = "1" ]]; then echo "-t"; else echo ""; fi')
docker_exe_tmp := $$(/bin/sh -c 'if [ $$(which podman) ]; then echo "podman"; else echo "docker"; fi')
docker_exe:=$(shell if [ $$(which podman) ]; then echo "podman"; else echo "docker"; fi)
docker_params = $$(/bin/sh -c 'if [ $$(which podman) ]]; then echo "--storage-opt ignore_chown_errors=true"; else echo ""; fi')
docker_run = $(docker_exe_tmp) $(docker_params) run -i $(docker_tty) --rm \
	                                            -e _UID=$(uid) -e _GID=$(gid) \
	                                            -e container=podman \
	                                            -e DISPLAY=$$DISPLAY \
	                                            -v /tmp/.X11-unix:/tmp/.X11-unix \
	                                            -v $$PWD:$$PWD \
	                                            -v $$PWD/.ccache:/root/.ccache \
	                                            -v $$PWD/.emscripten_cache:/tmp/.emscripten_cache \
	                                            --entrypoint /bin/bash \
	                                            -w $$PWD docker.io/rayburgemeestre/build-starcry-ubuntu:22.04
inside_docker_container = [[ "$$container" == "podman" ]]
run_in_container = ($(docker_run) -c "if $(ccache_enabled); then $(ccache_env) $1; else $1; fi")
run_locally = (if $(ccache_enabled); then $(ccache_env) $1; else $1; fi)
run_in_container_clang = ($(docker_run) -c "sudo switch-to-latest-clang; if $(ccache_enabled); then $(ccache_env) $1; else $1; fi")
run_locally_clang = (if $(ccache_enabled); then sudo switch-to-latest-clang; $(ccache_env) $1; else sudo switch-to-latest-clang; $1; fi)
make = if ! $(inside_docker_container); then $(run_in_container); \
	   else $(run_locally); fi
make-clang = if ! $(inside_docker_container); then $(run_in_container_clang); \
	         else $(run_locally_clang); fi

.PHONY: help
help: # with thanks to Ben Rady
	@grep -E '^[0-9a-zA-Z_-]+:.*?## .*$$' $(MAKEFILE_LIST) | sort | awk 'BEGIN {FS = ":.*?## "}; {printf "\033[36m%-20s\033[0m %s\n", $$1, $$2}'

.PHONY: build
build:  ## build starcry binary using docker (with clang)
	@$(call make-clang, CMAKE_EXE_LINKER_FLAGS=-fuse-ld=gold CXX=$$(which c++) cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=1 -GNinja -B build && \
	                    cmake --build build --target starcry -j 4 && \
	                    strip --strip-debug build/starcry)

.PHONY: all
all:  ## build all binaries in cmake
	@$(call make-clang, CMAKE_EXE_LINKER_FLAGS=-fuse-ld=gold CXX=$$(which c++) cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=1 -GNinja -B build && \
	                    cmake --build build)

.PHONY: build-gcc
build-gcc:  ## build starcry binary using docker (with gcc)
	@$(call make, CMAKE_EXE_LINKER_FLAGS=-fuse-ld=gold CXX=$$(which g++) cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=1 -GNinja -B build && \
	              cmake --build build --target starcry -j 4 && \
	              strip --strip-debug build/starcry)

.PHONY: test
test:  ## execute starcry unit tests using docker (with clang)
	@$(call make-clang, CMAKE_EXE_LINKER_FLAGS=-fuse-ld=gold CXX=$$(which c++) cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=1 -GNinja -B build && \
	                    cmake --build build --target tests && \
	                    ./build/tests)

.PHONY: integration-test
integration-test:  ## execute starcry unit tests using docker (with clang)
	@$(call make-clang, CMAKE_EXE_LINKER_FLAGS=-fuse-ld=gold CXX=$$(which c++) cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=1 -GNinja -B build && \
	                    cmake --build build --target integration_tests && \
	                    ./build/integration_tests -s)

.PHONY: integration-test-sanitizer
integration-test-sanitizer:
	@$(call make-clang, ASAN_SYMBOLIZER_PATH=/usr/lib/llvm-12/bin/llvm-symbolizer ASAN_OPTIONS=symbolize=1 \
	                    CMAKE_EXE_LINKER_FLAGS=-fuse-ld=gold CXX=$$(which c++) \
	                        cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=1 -DDEBUG=on -GNinja -B build && \
	                    ASAN_SYMBOLIZER_PATH=/usr/lib/llvm-12/bin/llvm-symbolizer ASAN_OPTIONS=symbolize=1 \
	                        cmake --build build --target integration_tests && \
	                    ASAN_SYMBOLIZER_PATH=/usr/lib/llvm-12/bin/llvm-symbolizer ASAN_OPTIONS=symbolize=1 \
	                        ./build/integration_tests)

client:  ## build webassembly javascript file using docker
	@$(call make-clang, /emsdk/upstream/emscripten/em++ --std=c++20 -s WASM=1 -s USE_SDL=2 -O3 --bind \
	                    -o web/webroot/client.js src/client.cpp src/stb.cpp src/util/logger.cpp \
	                    -I./src -I./libs/cereal/include -I./libs/perlin_noise/ -Ilibs/stb/ \
	                    -I/opt/cppse/build/v8pp/include/v8 -I/opt/cppse/build/v8pp/include/ \
	                    -I/opt/cppse/build/fmt/include /opt/cppse/build/fmt/lib/libfmt-em.a \
	                    -s TOTAL_MEMORY=1073741824 -s ASSERTIONS=0 -s ALLOW_MEMORY_GROWTH=0)

client_desktop:  ## build webassembly client for desktop for testing purposes
	@$(call make-clang, CMAKE_EXE_LINKER_FLAGS=-fuse-ld=gold CXX=$$(which c++) cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=1 -GNinja -B build && \
	                    cmake --build build --target sc_client)

client_debug:  ## build webassembly javascript file using docker with debug
	@$(call make-clang, /emsdk/upstream/emscripten/em++ --std=c++20 -s WASM=1 -s USE_SDL=2 -O3 -g --bind \
	                    -o web/webroot/client.js src/client.cpp src/stb.cpp \
	                    -I./src -I./libs/cereal/include -I./libs/perlin_noise/ -Ilibs/stb/ \
	                    -s TOTAL_MEMORY=1073741824 -s ASSERTIONS=1 -s ALLOW_MEMORY_GROWTH=1)

debug:  ## build starcry binary using docker with debug
	make debug-clean
	@$(call make-clang, mkdir -p build && pushd build && CMAKE_EXE_LINKER_FLAGS=-fuse-ld=gold CXX=$$(which c++) \
	                    cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=1 -DDEBUG=on .. && \
	                    make VERBOSE=1 -j $$(nproc) starcry && \
	                    ln -fs $$PWD/build/compile_commands.json $$PWD/compile_commands.json)

debug-gcc:  ## build starcry binary using docker with debug
	make debug-clean
	@$(call make, mkdir -p build && pushd build && CMAKE_EXE_LINKER_FLAGS=-fuse-ld=gold CXX=$$(which c++) \
	              cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=1 -DDEBUG=on .. && \
	              make VERBOSE=1 -j $$(nproc) starcry && \
	              ln -fs $$PWD/build/compile_commands.json $$PWD/compile_commands.json)

debug-sanitizer:  ## build starcry binary using docker with debug + address sanitizer
	make debug-clean
	@$(call make-clang, mkdir -p build && pushd build && ASAN_SYMBOLIZER_PATH=/usr/lib/llvm-12/bin/llvm-symbolizer ASAN_OPTIONS=symbolize=1 \
	                                   CXX=$$(which c++) cmake -DSANITIZER=1 -DDEBUG=on .. \
	                                                     cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=1 -DDEBUG=on .. && \
	                                   ASAN_SYMBOLIZER_PATH=/usr/lib/llvm-12/bin/llvm-symbolizer ASAN_OPTIONS=symbolize=1 \
	                                   make VERBOSE=1 -j $$(nproc) starcry && \
	                                   ln -fs $$PWD/build/compile_commands.json $$PWD/compile_commands.json)

debug-clean:
	rm -rf /var/crash/_home_trigen_projects_starcry_build_starcry.1144.crash
	rm -rf gdb.txt

format:  ## format source code (build at least once first)
	@$(call make-clang, stat -c 'chown %u:%g . -R' CMakeLists.txt | sh - && \
	                    mv -v web/webpack*.js input/ && \
	                    find ./input -name '*.js' -type f | xargs -I{} -n 1 -P 8 clang-format-14 -i {} && \
	                    mv -v input/webpack*.js ./web/ && \
	                    cmake --build build --target clangformat)

pull:  ## pull the starcry docker build image
	$(docker_exe) pull docker.io/rayburgemeestre/build-starcry-ubuntu:22.04

build-image:  ## build the starcry build image using podman
	$(docker_exe) pull -q docker.io/rayburgemeestre/build-ubuntu:22.04 && \
	$(docker_exe) build --no-cache -t docker.io/rayburgemeestre/build-starcry-ubuntu:22.04 -f Dockerfile .

runtime_deps:  ## install run-time dependencies
	./docs/install_runtime_deps.sh

deps: ## install dependencies required for running and building starcry
	./docs/install_deps.sh

.PHONY: shell
shell:  ## start a shell in the starcry build image
	xhost +
	@$(call make, bash)

.PHONY: starcry
clean:  ## clean build artifacts
	rm -f CMakeCache.txt
	rm -rf build
	rm -rf out
	rm -f callgrind.out.*
	rm -f actor_log*.log
	rm -f web/webroot/stream/stream.m3u8*

.PHONY: dockerize
dockerize:  ## dockerize starcry executable in stripped down docker image
	make build
	@$(call make-clang, mkdir -p out/workdir/web && \
	                    cp -prv $$PWD/web/webroot out/workdir/web/webroot && \
	                    (cp -prv $$PWD/output/report.html out/workdir/web/webroot/report.html || true) && \
	                    cp -prv $$PWD/input out/workdir/input && \
	                    strip --strip-debug $$PWD/build/starcry && \
						cp -prv $$PWD/build/starcry ./out/ && \
						cp -prv $$PWD/docs/Dockerfile ./out/ && \
						cp -prv $$PWD/docs/fonts/monaco.ttf ./out/workdir/ && \
						cp -prv $$PWD/docs/fonts/monogram.ttf ./out/workdir/)

	(cd out && $(docker_exe) build . -t docker.io/rayburgemeestre/starcry:v`cat ../.version`); \
	if ! [[ -z "$$DOCKER_PASSWORD" ]]; then \
	    echo "$$DOCKER_PASSWORD" | $(docker_exe) login -u "$$DOCKER_USERNAME" --password-stdin; \
	    $(docker_exe) push docker.io/rayburgemeestre/starcry:v`cat .version`; \
	else \
	    echo not executing $(docker_exe) push docker.io/rayburgemeestre/starcry:v`cat .version`; \
	fi
	# quick test
	$(docker_exe) run -it docker.io/rayburgemeestre/starcry:v`cat .version` /starcry --help || true

docker-finalize:
	@$(call make-clang, chown $$_UID:$$_GID . -R)  # docker specific workaround for CI


build-web:  ## build web static files
	pushd web && npm ci
	pushd web && npm run build

run_web:  ## run web in development hot-swappable mode
	pushd web && npm run dev

attach:
	docker exec -it starcry /bin/bash

release:  # alias for make build..
	make build

publish:  ## build from scratch starcry, web, client, docker image. (does not push to dockerhub)
	make clean
	make build
	make build-web
	make client
	make dockerize
	$(docker_exe) push docker.io/rayburgemeestre/starcry:v`cat .version` || true
	# $(docker_exe) push docker.io/rayburgemeestre/starcry-no-gui:v`cat .version` || true
	echo kubectl apply -f kube/starcry.yaml

#------------

clion:
	xhost +
	mkdir -p /tmp/ccache-root
	$(docker_exe) rm starcry_clion || true
	$(docker_exe) run --rm --name starcry_clion -p 18081:18080 -p 10001:10000 -p 16379:6379 -i --privileged -t -v /tmp/ccache-root:/root/.ccache -v $$PWD:/projects/starcry -v $$HOME:$$HOME -v $$HOME:/root -w /projects/starcry -e DISPLAY=$$DISPLAY -v /tmp/.X11-unix:/tmp/.X11-unix docker.io/rayburgemeestre/build-starcry-ubuntu:22.04 -c "switch-to-latest-clang; ${HOME}/system/superprofile/dot-files/.bin/clion ${HOME}"

ide_shell:
	xhost +
	mkdir -p /tmp/ccache-root
	$(docker_exe) rm starcry_clion || true
	$(docker_exe) run --rm --name starcry_clion -p 18081:18080 -p 10001:10000 -i --privileged -t -v /tmp/ccache-root:/root/.ccache -v $$PWD:/projects/starcry -v $$HOME:$$HOME -v $$HOME:/root -w /projects/starcry -e DISPLAY=$$DISPLAY -v /tmp/.X11-unix:/tmp/.X11-unix docker.io/rayburgemeestre/build-starcry-ubuntu:22.04 -c "switch-to-latest-clang; bash"

webstorm:
	xhost +
	mkdir -p /tmp/ccache-root
	podman run --rm --name starcry_webstorm -p 8081:8080 -i --privileged -t -v /tmp/ccache-root:/root/.ccache -v $$PWD:/projects/starcry -v $$HOME:$$HOME -v $$HOME:/root -w /projects/starcry -e DISPLAY=$$DISPLAY -v /tmp/.X11-unix:/tmp/.X11-unix docker.io/rayburgemeestre/build-starcry-ubuntu:22.04 -c "switch-to-latest-clang; ${HOME}/system/superprofile/dot-files/.bin/webstorm ${HOME}"

profile:  ## run starcry with valgrind's callgrind for profiling
	valgrind --tool=callgrind ./build/starcry -c 1 input/script.js
	# valgrind --tool=callgrind ./build/starcry --no-render --stdout input/test.js
	ls -althrst | tail -n 1

debug-last:
	rm -rf dbg
	apport-unpack /var/crash/_home_trigen_projects_starcry_build_starcry.1144.crash dbg
	gdb ./build/starcry dbg/CoreDump

#------------

a:
	make debug
	./build/starcry input/linetest2.js # -f 1

b:
	make build
	./build/starcry input/greenlines.js --raw

c:
	make debug
	gdb --args ./build/starcry --no-gui input/script.js

log:
	tail -f sc.log | ~/projects/metalogmon/metalogmon ~/projects/metalogmon/scripts/starcry.js
