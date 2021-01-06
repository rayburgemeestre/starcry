SHELL:=/bin/bash

fast-docker-build:
	# build starcry with tailored image so we can invoke the make command straight away
	docker run -it -v $$HOME/.ccache:/root/.ccache -v $$PWD:$$PWD --workdir $$PWD rayburgemeestre/build-starcry-ubuntu:20.04 sh -c "make prepare && make core_"

client:
	docker run -it -v $$HOME/.ccache:/root/.ccache -v $$PWD:$$PWD --workdir $$PWD rayburgemeestre/build-starcry-ubuntu:20.04 sh -c "/emsdk/upstream/emscripten/em++ -s WASM=1 -s USE_SDL=2 -O3 --bind -o webroot/client.js src/client.cpp src/util/progress_visualizer.cpp -I./include -I./libs/cereal/include -I./libs/perlin_noise/ -s TOTAL_MEMORY=1073741824 -s ASSERTIONS=0 -s ALLOW_MEMORY_GROWTH=0"
	# -I/usr/include -I/emsdk/upstream/emscripten/system/include/ -I/usr/include/x86_64-linux-gnu/

client_desktop:
	docker run -it -v $$HOME/.ccache:/root/.ccache -v $$PWD:$$PWD --workdir $$PWD rayburgemeestre/build-starcry-ubuntu:20.04 sh -c "c++ -O3 -o client_desktop src/client.cpp -I./include -I./libs/cereal/include -I./libs/perlin_noise/ -lSDL2 "

client-ubuntu1804:
	docker run -it -v $$HOME/.ccache:/root/.ccache -v $$PWD:$$PWD --workdir $$PWD rayburgemeestre/build-starcry-ubuntu:18.04 sh -c "/emsdk/upstream/emscripten/em++ -s WASM=1 -s USE_SDL=2 -O3 --bind -o webroot/client.js src/client.cpp -I./include -I./libs/cereal/include -I./libs/perlin_noise/ -s TOTAL_MEMORY=1073741824 -s ASSERTIONS=0 -s ALLOW_MEMORY_GROWTH=0"
	# -I/usr/include -I/emsdk/upstream/emscripten/system/include/ -I/usr/include/x86_64-linux-gnu/

client_debug:
	docker run -it -v $$HOME/.ccache:/root/.ccache -v $$PWD:$$PWD --workdir $$PWD rayburgemeestre/build-starcry-ubuntu:20.04 sh -c "/emsdk/upstream/emscripten/em++ -s WASM=1 -s USE_SDL=2 -O3 -g --bind -o webroot/client.js src/client.cpp -I./include -I./libs/cereal/include -I./libs/perlin_noise/ -s TOTAL_MEMORY=1073741824 -s ASSERTIONS=1 -s ALLOW_MEMORY_GROWTH=1"

ci:
	# Only difference with above is: no -i flag
	docker run -t -v $$HOME/.ccache:/root/.ccache -v $$PWD:$$PWD --workdir $$PWD rayburgemeestre/build-starcry-ubuntu:18.04 sh -c "make prepare && make core_"


debug:
	# build starcry with tailored image so we can invoke the make command straight away
	docker run -t -v $$HOME/.ccache:/root/.ccache -v $$PWD:$$PWD --workdir $$PWD rayburgemeestre/build-starcry-ubuntu:20.04 sh -c "make prepare && make core_debug"

format:
	# build starcry with tailored image so we can invoke the make command straight away
	docker run -t -v $$PWD:$$PWD --workdir $$PWD rayburgemeestre/build-starcry-ubuntu:20.04 sh -c "make prepare && make core_format"
	stat -c 'chown %u:%g . -R' CMakeLists.txt | sudo sh -

profile:
	valgrind --tool=callgrind ./build/starcry input/test.js -v
	ls -althrst | tail -n 1

pull:
	docker pull rayburgemeestre/build-starcry-ubuntu:20.04

docker-ubuntu1804:
	# build docker container specific for building starcry
	docker build -t rayburgemeestre/build-starcry-ubuntu:18.04 -f Dockerfile-ubuntu1804 .

docker-ubuntu2004:
	# build docker container specific for building starcry
	docker build -t rayburgemeestre/build-starcry-ubuntu:20.04 -f Dockerfile-ubuntu2004 .

ubuntu1804:
	# build from a more generic ubuntu image
	docker run -t -v $$PWD:$$PWD --workdir $$PWD rayburgemeestre/build-ubuntu:18.04 sh -c "make impl"

impl:
	make deps
	make prepare
	make core_

deps:
	sudo apt-get update
	sudo apt install -y lsb-release software-properties-common

	sudo apt-key adv --keyserver keyserver.ubuntu.com --recv-keys 5CE16B7B
	sudo add-apt-repository "deb [arch=amd64] https://cppse.nl/repo/ $$(lsb_release -cs) main"
	sudo apt-get install -y cppseffmpeg=1.1 v8pp=1.1 allegro5=1.1 allegro5sdl=1.1 fastpfor=1.1 boost=1.1 sfml=1.1 seasocks=1.1 pngpp=1.1 fmt=1.1 cppseopenexr=1.1 cppseimagemagick=1.1
	sudo apt-get install -y coz-profiler
	sudo apt-get install -y ccache

	# dependencies runtime
	sudo apt-get install -y cmake git wget bzip2 python-dev libbz2-dev \
				pkg-config curl \
				liblzma-dev
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
	#
	# ncurses
	sudo apt-get install -y libncurses-dev


client_deps:
	sudo apt-get update
	sudo apt-get install -y libsdl2-dev
	git clone https://github.com/emscripten-core/emsdk.git || true
	pushd emsdk && \
	./emsdk install latest && \
	./emsdk activate latest && \
	sudo cp -prv ./emsdk_env.sh /etc/profile.d/

prepare:
	# switch to clang compiler
	update-alternatives --install /usr/bin/c++ c++ /usr/bin/clang++-10 50
	update-alternatives --install /usr/bin/cc cc /usr/bin/clang-10 50
	update-alternatives --install /usr/bin/clang++ clang++ /usr/bin/clang++-10 50
	update-alternatives --install /usr/bin/clang clang /usr/bin/clang-10 50
	update-alternatives --install /usr/bin/ld ld /usr/bin/ld.lld-10 50
	# prepare build dir
	mkdir -p build

core_:
	pushd build && \
	CMAKE_EXE_LINKER_FLAGS=-fuse-ld=gold CXX=$(which c++) cmake .. && \
	make -j $$(nproc) starcry && \
	strip --strip-debug starcry

core_debug:
	pushd build && \
	CMAKE_EXE_LINKER_FLAGS=-fuse-ld=gold CXX=$(which c++) cmake -DDEBUG=on .. && \
	make VERBOSE=1 -j $$(nproc) starcry

core_debug_sanit:
	pushd build && \
	CXX=$(which c++) cmake -DSANITIZER=1 -DDEBUG=on .. && \
	make VERBOSE=1 -j $$(nproc)

core_format:
	cmake --build build --target clangformat
	find ./input -name '*.js' -type f -exec clang-format-10 -i {} \;

core_experimental:
	# switch to clang compiler

	update-alternatives --install /usr/bin/c++ c++ /home/trigen/projects/build-config/tmp/v8/third_party/llvm-build/Release+Asserts/bin/clang++ 40
	update-alternatives --install /usr/bin/cc cc /home/trigen/projects/build-config/tmp/v8/third_party/llvm-build/Release+Asserts/bin/clang 40
	update-alternatives --install /usr/bin/clang++ clang++ /home/trigen/projects/build-config/tmp/v8/third_party/llvm-build/Release+Asserts/bin/clang++ 40
	update-alternatives --install /usr/bin/clang clang /home/trigen/projects/build-config/tmp/v8/third_party/llvm-build/Release+Asserts/bin/clang 40
	update-alternatives --install /usr/bin/ld ld /home/trigen/projects/build-config/tmp/v8/third_party/llvm-build/Release+Asserts/bin/ld.lld 40
	mkdir -p build
	pushd build && \
	CXX=/home/trigen/projects/build-config/tmp/v8/third_party/llvm-build/Release+Asserts/bin/clang++ cmake .. && \
	CXX=/home/trigen/projects/build-config/tmp/v8/third_party/llvm-build/Release+Asserts/bin/clang++ make VERBOSE=1 -j $$(nproc)
	# CXX=$(which c++) cmake -DDEBUG=on .. && ..

.PHONY: build-shell
build-shell:
	docker run -i --privileged -t -v $$PWD:$$PWD --workdir $$PWD rayburgemeestre/build-ubuntu:18.04 /bin/bash

.PHONY: build-shell2
build-shell2:
	docker run -i --privileged -t -v $$HOME/.ccache:/root/.ccache -v $$PWD:$$PWD --workdir $$PWD rayburgemeestre/build-starcry-ubuntu:18.04 /bin/bash

.PHONY: shell
shell:
	xhost +
	docker run -i --privileged -t -v $$HOME/.ccache:/root/.ccache -v $$PWD:/projects/starcry -v $$HOME:/home/trigen -e DISPLAY=$DISPLAY -v /etc/passwd:/etc/passwd -v /etc/shadow:/etc/shadow -v /etc/sudoers:/etc/sudoers -v /tmp/.X11-unix:/tmp/.X11-unix -u 1144 rayburgemeestre/build-starcry-ubuntu:20.04 /bin/bash

.PHONY: starcry
clean:
	docker run -t -v $$PWD:$$PWD --workdir $$PWD rayburgemeestre/build-starcry-ubuntu:20.04 sh -c "cd build && make clean"
	rm -rf CMakeCache.txt
	rm -rf build/CMakeCache.txt
	rm -rf out
	rm -f callgrind.out.*
	rm -f actor_log*.log
	rm -f webroot/stream/stream.m3u8*

.PHONY: dockerize
dockerize:
	docker run -it -v $$HOME/.ccache:/root/.ccache -v $$PWD:$$PWD --workdir $$PWD rayburgemeestre/build-starcry-ubuntu:20.04 sh -c "make prepare && make core_"
	docker run $$FLAGS --privileged -t -v $$PWD:$$PWD --workdir $$PWD rayburgemeestre/build-starcry-ubuntu:20.04 /bin/sh -c "make dockerize_run"
	cd out && docker build . -t rayburgemeestre/starcry:v2
	docker push rayburgemeestre/starcry:v2

.PHONY: dockerize_run
dockerize_run:
	apt update
	apt install python3-pip rsync git -y
	cd /tmp && git clone https://github.com/larsks/dockerize && cd dockerize && python3 setup.py install
	# python3 -m pip install dockerize
	cp -prv $$PWD/build/starcry /starcry
	strip --strip-debug /starcry
	dockerize --verbose --debug -n -o out /starcry

gui:
	#docker run -it --privileged -v /etc/hosts:/etc/hosts -v $$HOME:$$HOME -u 1144 --workdir $$HOME -e DISPLAY=$$DISPLAY -v /tmp/.X11-unix:/tmp/.X11-unix rayburgemeestre/build-starcry-ubuntu:18.04 /bin/bash
	docker run -it --privileged -v $$HOME/.ccache:/root/.ccache -v /etc/hosts:/etc/hosts -v $$HOME:$$HOME --workdir $$HOME -e DISPLAY=$$DISPLAY -u 1144 -v /etc:/etc -v /tmp/.X11-unix:/tmp/.X11-unix rayburgemeestre/build-starcry-ubuntu:18.04 /bin/bash

build_web:
	npm install
	npm run build

run_web:
	npm run dev

