SHELL:=/bin/bash

uid:=$(shell id -u)
gid:=$(shell id -g)
interactive:=$(shell [ -t 0 ] && echo 1)
ccache_enabled = [[ -f '/usr/bin/ccache' ]]
ccache_env = CXX='ccache g++' CC='ccache gcc' CCACHE_SLOPPINESS=file_macro,locale,time_macros
docker_tty = $$(/bin/sh -c 'if [ "$(interactive)" = "1" ]]; then echo "-t"; else echo ""; fi')
docker_device_card0 = $$(/bin/sh -c 'if [ -e "/dev/dri/card0" ]]; then echo "--device /dev/dri/card0:/dev/dri/card0"; else echo ""; fi')
docker_exe_tmp := $$(/bin/sh -c 'if [ $$(which podman) ]; then echo "podman"; else echo "docker"; fi')
docker_exe:=$(shell if [ $$(which podman) ]; then echo "podman --log-level=debug"; else echo "docker"; fi)
#docker_params = $$(/bin/sh -c 'if [ $$(which podman) ]]; then echo "--storage-opt ignore_chown_errors=true"; else echo ""; fi')
docker_params = $$(/bin/sh -c 'if [ $$(which podman) ]]; then echo ""; else echo ""; fi')
docker_run = $(docker_exe_tmp) $(docker_params) run -i $(docker_tty) --rm \
	                                            -e _UID=$(uid) -e _GID=$(gid) \
	                                            -e container=podman \
	                                            -e DISPLAY=$$DISPLAY \
	                                            -v /tmp/.X11-unix:/tmp/.X11-unix \
	                                            -v $$PWD:$$PWD \
	                                            -v $$PWD/.ccache:/root/.ccache \
	                                            -v $$PWD/.emscripten_cache:/tmp/.emscripten_cache \
	                                            $(docker_device_card0) \
	                                            --entrypoint /bin/bash \
	                                            -w $$PWD docker.io/rayburgemeestre/build-starcry-ubuntu:24.04
inside_docker_container = [[ "$$container" == "podman" ]] # || [[ -f /.dockerenv ]]  # broken
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
	@$(call make-clang, CMAKE_EXE_LINKER_FLAGS=-fuse-ld=gold CXX=$$(which c++) cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=1 -B build && \
	                    cmake --build build --target starcry -j $$(nproc) && \
	                    strip --strip-debug build/starcry)

.PHONY: build-fastmath
build-fastmath:  ## build starcry binary using docker (with clang)
	@$(call make-clang, CMAKE_EXE_LINKER_FLAGS=-fuse-ld=gold CXX=$$(which c++) cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=1 -B build && \
	                    cmake --build build --target starcry-fastmath -j $$(nproc) && \
	                    strip --strip-debug build/starcry-fastmath)

.PHONY: all-binaries
all-binaries:  ## build all binaries in cmake
	@$(call make-clang, CMAKE_EXE_LINKER_FLAGS=-fuse-ld=gold CXX=$$(which c++) cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=1 -B build && \
	                    cmake --build build)

.PHONY: build-gcc
build-gcc:  ## build starcry binary using docker (with gcc)
	@$(call make, CMAKE_EXE_LINKER_FLAGS=-fuse-ld=gold CXX=$$(which g++) cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=1 -B build && \
	              cmake --build build --target starcry -j $$(nproc) && \
	              strip --strip-debug build/starcry)

.PHONY: test
test:  ## execute starcry unit tests using docker (with clang)
	@$(call make-clang, CMAKE_EXE_LINKER_FLAGS=-fuse-ld=gold CXX=$$(which c++) cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=1 -B build && \
	                    cmake --build build --target tests && \
	                    ./build/tests -s -d yes)

.PHONY: integration-test
integration-test:  ## execute starcry unit tests using docker (with clang)
	@$(call make-clang, CMAKE_EXE_LINKER_FLAGS=-fuse-ld=mold CXX=$$(which c++) cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=1 -B build && \
	                    cmake --build build --target integration_tests -j $$(nproc) && \
	                    ./build/integration_tests -s -d yes --rng-seed 0)

.PHONY: integration-test-gcc
integration-test-gcc:  ## execute starcry unit tests using docker (with gcc)
	@$(call make, CMAKE_EXE_LINKER_FLAGS=-fuse-ld=mold CXX=$$(which c++) cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=1 -B build && \
	              cmake --build build --target integration_tests -j $$(nproc) && \
	              ./build/integration_tests -s -d yes --rng-seed 0)

.PHONY: integration-test-gcc-debug
integration-test-gcc-debug:  ## execute starcry unit tests using docker (with gcc + debug)
	@$(call make, CMAKE_EXE_LINKER_FLAGS=-fuse-ld=mold CXX=$$(which c++) cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=1 -DDEBUG=on -B build && \
	              cmake --build build --target integration_tests -j $$(nproc) && \
	              gdb --args ./build/integration_tests -s -d yes --rng-seed 0)

.PHONY: integration-test-sanitizer
integration-test-sanitizer:
	@$(call make-clang, ASAN_SYMBOLIZER_PATH=/usr/lib/llvm-12/bin/llvm-symbolizer ASAN_OPTIONS=symbolize=1 \
	                    CMAKE_EXE_LINKER_FLAGS=-fuse-ld=mold CXX=$$(which c++) \
	                        cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=1 -DDEBUG=on -B build && \
	                    ASAN_SYMBOLIZER_PATH=/usr/lib/llvm-12/bin/llvm-symbolizer ASAN_OPTIONS=symbolize=1 \
	                        cmake --build build --target integration_tests -j $$(nproc) && \
	                    ASAN_SYMBOLIZER_PATH=/usr/lib/llvm-12/bin/llvm-symbolizer ASAN_OPTIONS=symbolize=1 \
	                        ./build/integration_tests -s -d yes --rng-seed 0)

client:  ## build webassembly javascript file using docker
	# Note, requires you to do 'make build' first for v8pp config.hpp
	@$(call make-clang, /emsdk/upstream/emscripten/em++ --std=c++20 -s WASM=1 -s USE_SDL=2 -O3 --bind \
	                    -o web/public/client.js src/client.cpp src/stb.cpp src/util/logger.cpp \
	                    src/shapes/position.cpp src/shapes/circle.cpp src/shapes/rectangle.cpp \
	                    src/util/noise_mappers.cpp src/rendering_engine/debug.cpp \
	                    -I./src -I./libs/cereal/include -I./libs/perlin_noise/ -Ilibs/stb/ \
	                    -I./libs/json/single_include/ \
	                    -I./libs/zpp_bits/ \
	                    -I/opt/cppse/build/v8/include/v8 -I/opt/cppse/build/v8/include/ \
	                    -I./libs/v8pp/ -I./build/libs/v8pp/ \
	                    -I/opt/cppse/build/vivid/include/ /opt/cppse/build/vivid/lib/libvivid-em.a \
	                    -I/opt/cppse/build/fmt/include /opt/cppse/build/fmt/lib/libfmt-em.a \
	                    -D_M_X64 \
	                    -s TOTAL_MEMORY=2147483648 -s ASSERTIONS=0 -s SAFE_HEAP=0 -s ALLOW_MEMORY_GROWTH=0)
						#-s TOTAL_MEMORY=4294967296 -s MAXIMUM_MEMORY=4294967296 -s ASSERTIONS=2 -s SAFE_HEAP=0 -s ALLOW_MEMORY_GROWTH=1)
	                    #-s TOTAL_MEMORY=2147483648 -s ASSERTIONS=1 -s SAFE_HEAP=1 -s ALLOW_MEMORY_GROWTH=1)
	                    #-s TOTAL_MEMORY=1073741824 -s ASSERTIONS=0 -s SAFE_HEAP=0 -s ALLOW_MEMORY_GROWTH=0)
	@$(call make-clang, sed -i.orig 's/"mouseup"/"no_mouseup"/g' web/public/client.js)

client-desktop:  ## build webassembly client for desktop for testing purposes
	@$(call make-clang, CMAKE_EXE_LINKER_FLAGS=-fuse-ld=gold CXX=$$(which c++) cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=1 -B build && \
	                    cmake --build build --target sc_client)

client-debug:  ## build webassembly javascript file using docker with debug
	@$(call make-clang, /emsdk/upstream/emscripten/em++ --std=c++20 -s WASM=1 -s USE_SDL=2 -O0 -g --bind \
	                    -o web/public/client.js src/client.cpp src/stb.cpp src/util/logger.cpp \
	                    src/shapes/position.cpp src/shapes/circle.cpp src/shapes/rectangle.cpp \
	                    src/util/noise_mappers.cpp src/rendering_engine/debug.cpp \
	                    -I./src -I./libs/cereal/include -I./libs/perlin_noise/ -Ilibs/stb/ \
	                    -I./libs/json/single_include/ \
	                    -I./libs/zpp_bits/ \
	                    -I/opt/cppse/build/v8/include/v8 -I/opt/cppse/build/v8/include/ \
	                    -I./libs/v8pp/ -I./build/libs/v8pp/ \
	                    -I/opt/cppse/build/vivid/include/ /opt/cppse/build/vivid/lib/libvivid-em.a \
	                    -I/opt/cppse/build/fmt/include /opt/cppse/build/fmt/lib/libfmt-em.a \
	                    -D_M_X64 \
	                    -s TOTAL_MEMORY=1073741824 -s ASSERTIONS=1 -s ALLOW_MEMORY_GROWTH=1)
	                    #-s TOTAL_MEMORY=2147483648 -s ASSERTIONS=0 -s SAFE_HEAP=0 -s ALLOW_MEMORY_GROWTH=0)
	                    #-s TOTAL_MEMORY=2147483648 -s ASSERTIONS=1 -s SAFE_HEAP=1 -s ALLOW_MEMORY_GROWTH=1)
	                    #-s TOTAL_MEMORY=1073741824 -s ASSERTIONS=0 -s SAFE_HEAP=0 -s ALLOW_MEMORY_GROWTH=0)
	@$(call make-clang, sed -i.orig 's/"mouseup"/"no_mouseup"/g' web/public/client.js)

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
	@$(call make-clang, find ./input -name '*.js' -type f | xargs -I{} -n 1 -P 8 clang-format-19 -i {} && \
	                    cmake --build build --target clangformat && \
						pushd web && npm run format && popd)

pull:  ## pull the starcry docker build image
	$(docker_exe) pull docker.io/rayburgemeestre/build-starcry-ubuntu:24.04

build-image:  ## build the starcry build image using podman
	$(docker_exe) pull docker.io/rayburgemeestre/build-ubuntu:24.04 && \
	$(docker_exe) build --no-cache -t docker.io/rayburgemeestre/build-starcry-ubuntu:24.04 -f Dockerfile .
	#$(docker_exe) build -t docker.io/rayburgemeestre/build-starcry-ubuntu:24.04 -f Dockerfile .

runtime_deps:  ## install run-time dependencies
	./docs/install_runtime_deps.sh

build-web-deps:
	pushd web && sudo apt install nodejs npm -y
	pushd web && npm ci
	pushd web && sudo npm i -g @quasar/cli

deps: ## install dependencies required for running and building starcry
	./docs/install_deps.sh

.PHONY: shell
shell:  ## start a shell in the starcry build image
	xhost +local:
	@$(call make, bash)

.PHONY: starcry
clean:  ## clean build artifacts
	sudo rm -f CMakeCache.txt
	sudo rm -rf build
	sudo rm -rf out
	sudo rm -f callgrind.out.*
	sudo rm -f actor_log*.log
	sudo rm -f web/webroot/stream/stream.m3u8*
	sudo rm -rf docs/doxygen/*
	sudo rm -rf docs/output/*
	sudo rm -rf cmake-build-debug

.PHONEY: clean2
clean2:
	sudo rm -rfv .ccache/*
	make clean

.PHONY: dockerize
dockerize:  ## dockerize starcry executable in stripped down docker image
	make build
	@$(call make-clang, mkdir -p out/workdir/web && \
						echo disabled: cp -prv $$PWD/web/webroot out/workdir/web/webroot && \
	                    cp -prv $$PWD/web/dist/spa out/workdir/web/webroot && \
	                    (cp -prv $$PWD/output/report.html out/workdir/web/webroot/report.html || true) && \
	                    cp -prv $$PWD/input out/workdir/input && \
	                    strip --strip-debug $$PWD/build/starcry && \
						cp -prv $$PWD/build/starcry ./out/ && \
						cp -prv $$PWD/docs/Dockerfile ./out/ && \
						cp -prv $$PWD/docs/starcry-dev-wrapper.sh ./out/workdir/ && \
						cp -prv $$PWD/docs/fonts/monaco.ttf ./out/workdir/ && \
						cp -prv $$PWD/docs/fonts/monogram.ttf ./out/workdir/ && \
						cp -prv $$PWD/docs/output ./out/workdir/web/webroot/docs)

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
	@$(call make-clang, pushd web && npm ci)
	@$(call make-clang, pushd web && ./node_modules/.bin/quasar build)  # doesn't require build-web-deps

run-web:  ## run web in development hot-swappable mode
	pushd web && quasar dev

attach:
	docker exec -it starcry /bin/bash

release:  # alias for make build..
	make build

all:
	make build
	make build-web
	make client

publish:  ## build from scratch starcry, web, client, docker image, push dockerhub, deploy k8s
	make clean
	make all
	make docs || true  # first time tends to fail for some reason
	make docs || true  # second time hopefully better...
	make dockerize
	make push
	make kube
	echo done

publish-fast:  ## publish but assume builds are done manually
	make dockerize
	make push
	make kube
	echo done

.PHONY: kube
kube:
	kubectl apply -f kube/starcry.yaml
	kubectl rollout restart deployment -n starcry starcry
	kubectl rollout restart deployment -n starcry starcry-workers
	kubectl rollout restart deployment -n starcry redis

push:
	$(docker_exe) push docker.io/rayburgemeestre/starcry:v`cat .version` || true

#------------

clion:
	xhost +
	# doesn't seem to work well with latest Linux mainline kernel (6.12.3 at time of writing)
	sudo sysctl kernel.unprivileged_userns_clone=0 || true
	mkdir -p /tmp/ccache-root
	$(docker_exe) stop starcry_clion || true
	$(docker_exe) rm starcry_clion || true
	$(docker_exe) run --rm --name starcry_clion $(docker_device_card0) --network host -i --privileged -t -v /tmp/ccache-root:/root/.ccache -v $$PWD:/projects/starcry -v $$HOME:$$HOME -v $$HOME:/root -w /projects/starcry -e DISPLAY=$$DISPLAY -v /tmp/.X11-unix:/tmp/.X11-unix docker.io/rayburgemeestre/build-starcry-ubuntu:24.04 -c "switch-to-latest-clang; ${HOME}/system/superprofile/dot-files/.bin/clion ${HOME}"

clion-gcc:
	xhost +
	sudo sysctl kernel.unprivileged_userns_clone=0
	sudo rm -rf /home/trigen/.config/JetBrains/CLion2023.3/.lock
	mkdir -p /tmp/ccache-root
	$(docker_exe) stop starcry_clion || true
	$(docker_exe) rm starcry_clion || true
	$(docker_exe) run --rm --name starcry_clion $(docker_device_card0) --network host -i --privileged -t -v /tmp/ccache-root:/root/.ccache -v $$PWD:/projects/starcry -v $$HOME:$$HOME -v $$HOME:/root -w /projects/starcry -e DISPLAY=$$DISPLAY -v /tmp/.X11-unix:/tmp/.X11-unix docker.io/rayburgemeestre/build-starcry-ubuntu:24.04 -c "switch-to-latest-clang; ${HOME}/system/superprofile/dot-files/.bin/clion ${HOME}"

ide_shell:
	xhost +
	sudo sysctl kernel.unprivileged_userns_clone=0
	mkdir -p /tmp/ccache-root
	$(docker_exe) stop starcry_clion || true
	$(docker_exe) rm starcry_clion || true
	$(docker_exe) run --rm --name starcry_clion --network host -i --privileged -t -v /tmp/ccache-root:/root/.ccache -v $$PWD:/projects/starcry -v $$HOME:$$HOME -v $$HOME:/root -w /projects/starcry -e DISPLAY=$$DISPLAY -v /tmp/.X11-unix:/tmp/.X11-unix docker.io/rayburgemeestre/build-starcry-ubuntu:24.04 -c "switch-to-latest-clang; bash"

webstorm:
	xhost +
	mkdir -p /tmp/ccache-root
	$(docker_exe) stop starcry_webstorm || true
	$(docker_exe) rm starcry_webstorm || true
	$(docker_exe) run --rm --name starcry_webstorm --network host -i --privileged -t -v /tmp/ccache-root:/root/.ccache -v $$PWD:/projects/starcry -v $$HOME:$$HOME -v $$HOME:/root -w /projects/starcry -e DISPLAY=$$DISPLAY -v /tmp/.X11-unix:/tmp/.X11-unix docker.io/rayburgemeestre/build-starcry-ubuntu:24.04 -c "switch-to-latest-clang; ${HOME}/system/superprofile/dot-files/.bin/webstorm ${HOME}"

profile:  ## run starcry with valgrind's callgrind for profiling
	#valgrind --tool=callgrind ./build/starcry -c 1 input/broken/wave.js -f 1 --stdout --debug --verbose
	#valgrind --tool=callgrind ./build/starcry --tui input/spiral2.js -f 1 --single --no-output
	valgrind --tool=callgrind ./build/starcry input/blank.js --benchmark --no-output
	# valgrind --tool=callgrind ./build/starcry --no-render --stdout input/test.js
	ls -althrst | tail -n 1
	echo invoke kcachegrind on this file

valgrind:
	#valgrind --leak-check=full ./build/starcry input/motion.js  --no-output
	valgrind --leak-check=full ./build/starcry

profile-memory:
	#valgrind --tool=massif --massif-out-file=massif.out.starcry ./build/starcry input/sampling.js --gui
	valgrind --tool=massif --time-unit=B --detailed-freq=1 --max-snapshots=50  --massif-out-file=massif.out.starcry ./build/starcry input/motion.js --no-output
	massif-visualizer massif.out.starcry

download-integration-test-reference:
	wget https://cppse.nl/reference.tar.gz
	tar -zxvf reference.tar.gz

upload-integration-test-reference:
	upload_integration_test_results.sh

debug-last:
	rm -rf dbg
	apport-unpack /var/crash/_home_trigen_projects_starcry_build_starcry.1144.crash dbg
	gdb ./build/starcry dbg/CoreDump

.PHONY: docs
docs:  ## build starcry docs
	@$(call make-clang, pushd docs && doxygen && sphinx-build source output && echo DONE)

# Tilt helpers
up:
	# curl -fsSL https://raw.githubusercontent.com/tilt-dev/tilt/master/scripts/install.sh | bash
	KUBECONFIG=/etc/rancher/k3s/k3s.yaml tilt up

down:
	KUBECONFIG=/etc/rancher/k3s/k3s.yaml tilt down

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
