SHELL:=/bin/bash

DATE_STR:=$(shell date +"%Y%m%d")
TIME_STR:=$(shell date +'%H%M')

.PHONY: help
help: # with thanks to Ben Rady
	@grep -E '^[0-9a-zA-Z_-]+:.*?## .*$$' $(MAKEFILE_LIST) | sort | awk 'BEGIN {FS = ":.*?## "}; {printf "\033[36m%-20s\033[0m %s\n", $$1, $$2}'

fast-docker-build:  ## build starcry binary using docker
	# build starcry with tailored image so we can invoke the make command straight away
	mkdir -p /tmp/ccache-root
	docker run -t -v /tmp/ccache-root:/root/.ccache -v $$PWD:$$PWD --workdir $$PWD rayburgemeestre/build-starcry-ubuntu:20.04 sh -c "make prepare && make core_"

client:  ## build webassembly javascript file using docker
	mkdir -p /tmp/ccache-root
	docker run -it -v /tmp/ccache-root:/root/.ccache -v $$PWD:$$PWD --workdir $$PWD rayburgemeestre/build-starcry-ubuntu:20.04 sh -c "/emsdk/upstream/emscripten/em++ -s WASM=1 -s USE_SDL=2 -O3 --bind -o webroot/client.js src/client.cpp -I./src -I./libs/cereal/include -I./libs/perlin_noise/ -s TOTAL_MEMORY=1073741824 -s ASSERTIONS=0 -s ALLOW_MEMORY_GROWTH=0"
	# -I/usr/include -I/emsdk/upstream/emscripten/system/include/ -I/usr/include/x86_64-linux-gnu/

client_desktop:  ## build webassembly client for desktop for testing purposes
	mkdir -p /tmp/ccache-root
	docker run -it -v /tmp/ccache-root:/root/.ccache -v $$PWD:$$PWD --workdir $$PWD rayburgemeestre/build-starcry-ubuntu:20.04 sh -c "c++ -DSC_CLIENT -O3 -o client_desktop src/client.cpp -I src -I./libs/cereal/include -I./libs/perlin_noise/ -lSDL2 "


client_debug:  ## build webassembly javascript file using docker with debug
	mkdir -p /tmp/ccache-root
	docker run -it -v /tmp/ccache-root:/root/.ccache -v $$PWD:$$PWD --workdir $$PWD rayburgemeestre/build-starcry-ubuntu:20.04 sh -c "/emsdk/upstream/emscripten/em++ -s WASM=1 -s USE_SDL=2 -O3 -g --bind -o webroot/client.js src/client.cpp -I./src -I./libs/cereal/include -I./libs/perlin_noise/ -s TOTAL_MEMORY=1073741824 -s ASSERTIONS=1 -s ALLOW_MEMORY_GROWTH=1"

debug:  ## build starcry binary using docker with debug
	# build starcry with tailored image so we can invoke the make command straight away
	mkdir -p /tmp/ccache-root
	docker run -t -v /tmp/ccache-root:/root/.ccache -v $$PWD:$$PWD --workdir $$PWD rayburgemeestre/build-starcry-ubuntu:20.04 sh -c "make prepare && make core_debug"

format:  ## format source code (build at least once first)
	# build starcry with tailored image so we can invoke the make command straight away
	docker run -t -v $$PWD:$$PWD --workdir $$PWD rayburgemeestre/build-starcry-ubuntu:20.04 sh -c "make prepare && make core_format"
	stat -c 'chown %u:%g . -R' CMakeLists.txt | sudo sh -

profile:  ## run starcry with valgrind's callgrind for profiling
	valgrind --tool=callgrind ./build/starcry input/test.js -v
	ls -althrst | tail -n 1

pull:  ## pull the starcry docker build image
	docker pull rayburgemeestre/build-starcry-ubuntu:20.04

docker-ubuntu2004:  ## build the starcry docker build image
	docker build -t rayburgemeestre/build-starcry-ubuntu:20.04 -f Dockerfile-ubuntu2004 .

runtime_deps:  ## install run-time dependencies
	# dependencies runtime
	sudo apt-get install -y libomp5 cmake git wget bzip2 python-dev libbz2-dev \
				pkg-config curl \
				liblzma-dev

deps: ## install dependencies required for running and building starcry
	sudo apt-get update
	sudo apt install -y lsb-release software-properties-common ca-certificates

	sudo apt-key adv --keyserver keyserver.ubuntu.com --recv-keys 5CE16B7B
	sudo add-apt-repository "deb [arch=amd64] https://cppse.nl/repo/ $$(lsb_release -cs) main"
	sudo apt-get install -y cppseffmpeg=1.1 v8pp=1.1 allegro5=1.1 allegro5sdl=1.1 fastpfor=1.1 boost=1.1 sfml=1.1 seasocks=1.1 pngpp=1.1 fmt=1.1 cppseopenexr=1.1 cppseimagemagick=1.1
	sudo apt-get install -y coz-profiler
	sudo apt-get install -y ccache

	sudo apt-get install -y cmake git wget bzip2 python-dev libbz2-dev \
				pkg-config curl \
				liblzma-dev libomp5

	# ubuntu 18.04: sudo apt-get install -y libssl1.0-dev
	sudo apt-get install -y libssl-dev
	sudo apt-get install -y freeglut3-dev libgl1-mesa-dev libglu1-mesa-dev mesa-common-dev \
			        libxcursor-dev libavfilter-dev
	sudo apt-get install -y libpng-dev libjpeg-dev libfreetype6-dev \
				libxrandr-dev libxinerama-dev libxi-dev
	sudo apt-get install -y libgtk2.0-dev
	sudo apt-get install -y libpng-dev libjpeg-dev libfreetype6-dev
	# sfml (system 2.4)
	#sudo apt-get install -y libbz2-dev liblzma-dev libz-dev
	#sudo pt-get install -y libsfml-dev
	# sfml self-compiled (2.5)
	sudo apt-get install -y libudev-dev libopenal-dev libflac-dev libvorbis-dev

	# image magick and openexr
	sudo apt-get install -y libwebp-dev libarchive-dev libzstd-dev libjbig-dev libtiff-dev
	# ubuntu runtime deps
	# libwebp6: ..
	# libarchive13: /usr/lib/x86_64-linux-gnu/libarchive.so.13
	# libzstd1: /usr/lib/x86_64-linux-gnu/libzstd.so.1.4.4
	# ncurses
	sudo apt-get install -y libncurses-dev

	# clion dependencies
	sudo apt install -y libxtst6

client_deps:  ## install dependencies for building webassembly client
	sudo apt-get update
	sudo apt-get install -y libsdl2-dev
	git clone https://github.com/emscripten-core/emsdk.git || true
	pushd emsdk && \
	./emsdk install latest && \
	./emsdk activate latest && \
	sudo cp -prv ./emsdk_env.sh /etc/profile.d/

prepare:  ## prepare environment before building (such as switch to clang)
	# switch to clang compiler
	update-alternatives --install /usr/bin/c++ c++ /usr/bin/clang++-10 50
	update-alternatives --install /usr/bin/cc cc /usr/bin/clang-10 50
	update-alternatives --install /usr/bin/clang++ clang++ /usr/bin/clang++-10 50
	update-alternatives --install /usr/bin/clang clang /usr/bin/clang-10 50
	update-alternatives --install /usr/bin/ld ld /usr/bin/ld.lld-10 50
	# prepare build dir
	mkdir -p build

core_:  ## execute build steps for building starcry binary
	pushd build && \
	CMAKE_EXE_LINKER_FLAGS=-fuse-ld=gold CXX=$$(which c++) cmake .. && \
	time make -j $$(nproc) starcry && \
	strip --strip-debug starcry

core_debug:  ## execute build steps for building starcry binary with debug
	pushd build && \
	CMAKE_EXE_LINKER_FLAGS=-fuse-ld=gold CXX=$$(which c++) cmake -DDEBUG=on .. && \
	time make VERBOSE=1 -j $$(nproc) starcry

core_debug_sanit:  ## execute build steps for building starcry binary with address sanitizer and debug
	pushd build && \
	CXX=$$(which c++) cmake -DSANITIZER=1 -DDEBUG=on .. && \
	make VERBOSE=1 -j $$(nproc)

core_format:  ## execute formatting
	mv -v webpack*.js input/
	find ./input -name '*.js' -type f -exec clang-format-10 -i {} \;
	mv -v input/webpack*.js ./
	cmake --build build --target clangformat

.PHONY: shell
shell:  ## start a shell in the starcry build image
	xhost +
	mkdir -p /tmp/ccache-root
	docker run -i --privileged -t -v /tmp/ccache-root:/root/.ccache -v $$PWD:/projects/starcry -v $$HOME:/home/trigen -w /projects/starcry -e DISPLAY=$$DISPLAY -v /etc/passwd:/etc/passwd -v /etc/shadow:/etc/shadow -v /etc/sudoers:/etc/sudoers -v /tmp/.X11-unix:/tmp/.X11-unix -u 1144 rayburgemeestre/build-starcry-ubuntu:20.04 /bin/bash

.PHONY: starcry
clean:  ## clean build artifacts
	docker run -t -v $$PWD:$$PWD --workdir $$PWD rayburgemeestre/build-starcry-ubuntu:20.04 sh -c "cd build && make clean"
	rm -rf CMakeCache.txt
	rm -rf build/CMakeCache.txt
	rm -rf out
	rm -f callgrind.out.*
	rm -f actor_log*.log
	rm -f webroot/stream/stream.m3u8*

attick:  ## clean local directory, move all untracked and ignored files to separate directory
	mkdir -p ../starcry-attick-$(DATE_STR)$(TIME_STR)
	git status -s --ignored | grep -E "^??|^!!" | cut -f 2 -d ' ' | xargs -I{} -n 1 mv "{}" ../starcry-attick-$(DATE_STR)$(TIME_STR)/

.PHONY: dockerize
dockerize:  ## dockerize starcry executable in stripped down docker image
	mkdir -p /tmp/ccache-root
	docker run $$FLAGS -v /tmp/ccache-root:/root/.ccache -v $$PWD:$$PWD --workdir $$PWD rayburgemeestre/build-starcry-ubuntu:20.04 sh -c "make prepare && make core_"
	docker run $$FLAGS --privileged -t -v $$PWD:$$PWD --workdir $$PWD rayburgemeestre/build-starcry-ubuntu:20.04 /bin/sh -c "make dockerize_run"
	cd out && docker build . -t rayburgemeestre/starcry:v2
	echo "$$DOCKER_PASSWORD" | docker login -u "$$DOCKER_USERNAME" --password-stdin
	docker push rayburgemeestre/starcry:v2

.PHONY: dockerize_run
dockerize_run:  ## execute dockerize steps
	apt update
	apt install python3-pip rsync git ncurses-term -y
	cd /tmp && git clone https://github.com/larsks/dockerize && cd dockerize && python3 setup.py install
	cp -prv $$PWD/build/starcry /starcry
	strip --strip-debug /starcry
	dockerize --verbose --debug -n -o out "/starcry"
	mkdir -p out/usr/share/terminfo/x
	cp -prv /usr/share/terminfo/x/xterm-16color out/usr/share/terminfo/x/
	sed -i.bak '2 a ENV TERM=xterm-16color' out/Dockerfile

build_web:  ## build web static files
	npm install
	npm run build

run_web:  ## run web in development hot-swappable mode
	npm run dev

