SHELL:=/bin/bash

uid:=$(shell id -u)
gid:=$(shell id -g)

interactive:=$(shell [ -t 0 ] && echo 1)

ccache_enabled = [[ -f '/usr/bin/ccache' ]]
ccache_env = CXX='ccache g++' CC='ccache gcc' CCACHE_SLOPPINESS=file_macro,locale,time_macros

docker_tty = $$(/bin/sh -c 'if [ "$(interactive)" = "1" ]]; then echo "-t"; else echo ""; fi')
docker_run = docker run -i $(docker_tty) --init --rm \
	    	    	    -e _UID=$(uid) -e _GID=$(gid) \
	                    -v $$PWD:$$PWD \
	    	            -v $$PWD/.ccache:/tmp/.ccache \
	    	    	    -v $$PWD/.emscripten_cache:/tmp/.emscripten_cache \
	                    -w $$PWD rayburgemeestre/build-starcry-ubuntu:20.04

inside_docker_container = [[ -f /.dockerenv ]]

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
	@$(call make-clang, CMAKE_EXE_LINKER_FLAGS=-fuse-ld=gold CXX=$$(which c++) cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=1 -GNinja -B build)
	@$(call make-clang, cmake --build build --target starcry)
	@$(call make-clang, strip --strip-debug build/starcry)

.PHONY: build-gcc
build-gcc:  ## build starcry binary using docker (with gcc)
	@$(call make, CMAKE_EXE_LINKER_FLAGS=-fuse-ld=gold CXX=$$(which g++) cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=1 -GNinja -B build)
	@$(call make, cmake --build build --target starcry)
	@$(call make, strip --strip-debug build/starcry)

.PHONY: test
test:  ## execute starcry unit tests using docker (with clang)
	@$(call make-clang, CMAKE_EXE_LINKER_FLAGS=-fuse-ld=gold CXX=$$(which c++) cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=1 -GNinja -B build)
	@$(call make-clang, cmake --build build --target tests)
	@$(call make-clang, ./build/tests)

client:  ## build webassembly javascript file using docker
	@$(call make-clang, /emsdk/upstream/emscripten/em++ -s WASM=1 -s USE_SDL=2 -O3 --bind \
	                    -o webroot/client.js src/client.cpp src/stb.cpp \
	    	    	    -I./src -I./libs/cereal/include -I./libs/perlin_noise/ -Ilibs/stb/ \
	    	    	    -s TOTAL_MEMORY=1073741824 -s ASSERTIONS=0 -s ALLOW_MEMORY_GROWTH=0)

client_desktop:  ## build webassembly client for desktop for testing purposes
	# TODO: seems broken
	@$(call make-clang, c++ -DSC_CLIENT -O3 -o client_desktop src/client.cpp \
	                    -I src -I./libs/cereal/include -I./libs/perlin_noise/ -I/usr/include/SDL2 -lSDL2)

client_debug:  ## build webassembly javascript file using docker with debug
	@$(call make-clang, /emsdk/upstream/emscripten/em++ -s WASM=1 -s USE_SDL=2 -O3 -g --bind \
	                    -o webroot/client.js src/client.cpp src/stb.cpp \
	    	    	    -I./src -I./libs/cereal/include -I./libs/perlin_noise/ -Ilibs/stb/ \
	    	    	    -s TOTAL_MEMORY=1073741824 -s ASSERTIONS=1 -s ALLOW_MEMORY_GROWTH=1)

debug:  ## build starcry binary using docker with debug
	make debug-clean
	@$(call make-clang, pushd build && CMAKE_EXE_LINKER_FLAGS=-fuse-ld=gold CXX=$$(which c++) \
	                    cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=1 -DDEBUG=on ..)
	@$(call make-clang, pushd build && make VERBOSE=1 -j $$(nproc) starcry)
	@$(call make-clang, pushd build && ln -fs $$PWD/build/compile_commands.json $$PWD/compile_commands.json)

debug-sanitizer:  ## build starcry binary using docker with debug + address sanitizer
	make debug-clean
	@$(call make-clang, pushd build && ASAN_SYMBOLIZER_PATH=/usr/lib/llvm-12/bin/llvm-symbolizer ASAN_OPTIONS=symbolize=1 \
	                                   CXX=$$(which c++) cmake -DSANITIZER=1 -DDEBUG=on .. \
	                    cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=1 -DDEBUG=on ..)
	@$(call make-clang, pushd build && ASAN_SYMBOLIZER_PATH=/usr/lib/llvm-12/bin/llvm-symbolizer ASAN_OPTIONS=symbolize=1 \
	                    make VERBOSE=1 -j $$(nproc) starcry)
	@$(call make-clang, pushd build && ln -fs $$PWD/build/compile_commands.json $$PWD/compile_commands.json)

debug-clean:
	rm -rf /var/crash/_home_trigen_projects_starcry_build_starcry.1144.crash
	rm -rf gdb.txt

format:  ## format source code (build at least once first)
	@$(call make-clang, stat -c 'chown %u:%g . -R' CMakeLists.txt | sudo sh -)
	@$(call make-clang, mv -v webpack*.js input/)
	@$(call make-clang, find ./input -name '*.js' -type f -exec clang-format-12 -i {} \;)
	@$(call make-clang, mv -v input/webpack*.js ./)
	@$(call make-clang, cmake --build build --target clangformat)

pull:  ## pull the starcry docker build image
	docker pull rayburgemeestre/build-starcry-ubuntu:20.04

docker:  ## build the starcry docker build image
	docker pull rayburgemeestre/build-ubuntu:20.04 && \
	docker build -t rayburgemeestre/build-starcry-ubuntu:20.04 -f Dockerfile .

runtime_deps:  ## install run-time dependencies
	./docs/install_runtime_deps.sh

deps: ## install dependencies required for running and building starcry
	./docs/install_deps.sh

client_deps:  ## install dependencies for building webassembly client
	./docs/install_client_deps.sh

dockerize_deps:
	./docs/install_dockerize_deps.sh

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
	rm -f webroot/stream/stream.m3u8*

.PHONY: dockerize
dockerize:  ## dockerize starcry executable in stripped down docker image
	make build

	@$(call make-clang, mkdir -p out/workdir)
	@$(call make-clang, cp -prv $$PWD/webroot out/workdir/webroot)
	@$(call make-clang, cp -prv $$PWD/output/report.html out/workdir/webroot/report.html || true)
	@$(call make-clang, cp -prv $$PWD/input out/workdir/input)
	@$(call make-clang, strip --strip-debug $$PWD/build/starcry)
	@$(call make-clang, dockerize --verbose --debug -n -o out --add-file $$PWD/build/starcry /starcry)
	@$(call make-clang, mkdir -p out/usr/share/terminfo/x)
	@$(call make-clang, cp -prv /usr/share/terminfo/x/xterm-16color out/usr/share/terminfo/x/)
	@$(call make-clang, sed -i.bak '2 a ENV TERM=xterm-16color' out/Dockerfile)

	cd out && docker build . -t rayburgemeestre/starcry:v`cat ../.version`
	if ! [[ -z "$$DOCKER_PASSWORD" ]]; then \
	    echo "$$DOCKER_PASSWORD" | docker login -u "$$DOCKER_USERNAME" --password-stdin; \
	    docker push rayburgemeestre/starcry:v`cat .version`; \
	else \
	    echo not executing docker push rayburgemeestre/starcry:v`cat .version`; \
	fi
	# quick test
	docker run -it rayburgemeestre/starcry:v`cat .version` /starcry --help || true

build_web:  ## build web static files
	npm ci
	npm run build

run_web:  ## run web in development hot-swappable mode
	npm run dev

attach:
	docker exec -it starcry /bin/bash

release:  # alias for make build..
	make build

publish:
	make clean
	make build
	make build_web
	make client
	make dockerize
	echo docker push rayburgemeestre/starcry:v`cat .version`
	echo kubectl apply -f kube/starcry.yaml

#------------

clion:
	xhost +
	docker run --rm --name starcry -p 18081:18080 -i --privileged -t -v /tmp/ccache-root:/root/.ccache -v $$PWD:/projects/starcry -v $$HOME:/home/trigen -w /projects/starcry -e DISPLAY=$$DISPLAY -v /etc/passwd:/etc/passwd -v /etc/shadow:/etc/shadow -v /etc/sudoers:/etc/sudoers -v /tmp/.X11-unix:/tmp/.X11-unix -u 1144 rayburgemeestre/build-starcry-ubuntu:20.04 -c "sudo switch-to-latest-clang; /home/trigen/system/superprofile/dot-files/.bin/clion"

profile:  ## run starcry with valgrind's callgrind for profiling
	valgrind --tool=callgrind ./build/starcry input/contrast.js -f 1
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
