SHELL:=/bin/bash

fast-docker-build:
	# build starcry with tailored image so we can invoke the make command straight away
	docker run -t -v $$PWD:$$PWD --workdir $$PWD rayburgemeestre/build-starcry-ubuntu:18.04 sh -c "make core"

docker:
	# build docker container specific for building starcry
	docker build -t rayburgemeestre/build-starcry-ubuntu:18.04 -f Dockerfile .

ubuntu1804:
	# build from a more generic ubuntu image
	docker run -t -v $$PWD:$$PWD --workdir $$PWD rayburgemeestre/build-ubuntu:18.04 sh -c "make impl"

impl:
	make deps
	make core

deps:
	sudo apt-get update
	sudo apt install -y lsb-release software-properties-common

	sudo apt-key adv --keyserver keyserver.ubuntu.com --recv-keys 5CE16B7B
	sudo add-apt-repository "deb https://cppse.nl/repo/ $$(lsb_release -cs) main"
	sudo apt-get install ffmpeg=1.0 v8pp crtmpserver allegro5 caf benchmarklib fastpfor boost

	# dependencies runtime
	sudo apt-get install -y cmake git wget bzip2 python-dev libbz2-dev \
				pkg-config curl \
				liblzma-dev
	sudo apt-get install -y libssl1.0-dev
	sudo apt-get install -y freeglut3-dev libgl1-mesa-dev libglu1-mesa-dev mesa-common-dev \
			        libxcursor-dev libavfilter-dev
	sudo apt-get install -y libpng-dev libjpeg-dev libfreetype6-dev \
				libxrandr-dev libxinerama-dev libxi-dev
	sudo apt-get install -y libgtk2.0-dev
	sudo apt-get install -y libpng-dev libjpeg-dev libfreetype6-dev

core:
	# switch to clang compiler
	update-alternatives --install /usr/bin/c++ c++ /usr/bin/clang++-7 40
	update-alternatives --install /usr/bin/cc cc /usr/bin/clang-7 40
	update-alternatives --install /usr/bin/clang++ clang++ /usr/bin/clang++-7 40
	update-alternatives --install /usr/bin/clang clang /usr/bin/clang-7 40
	update-alternatives --install /usr/bin/ld ld /usr/bin/ld.lld-7 40
	mkdir -p build
	pushd build && \
	CXX=$(which c++) cmake -DLIB_PREFIX_DIR=/usr/local/src/starcry/ .. && \
	make -j $$(nproc)

.PHONY: build-shell
build-shell:
	docker run -i --privileged -t -v $$PWD:$$PWD --workdir $$PWD rayburgemeestre/build-ubuntu:18.04 /bin/bash

.PHONY: build-shell2
build-shell2:
	docker run -i --privileged -t -v $$PWD:$$PWD --workdir $$PWD rayburgemeestre/build-starcry-ubuntu:18.04 /bin/bash

.PHONY: shell
shell:
	xhost +
	docker run -i --privileged -t -v $$PWD:/projects/starcry -v $$HOME:/home/trigen -e DISPLAY=$DISPLAY -v /etc/passwd:/etc/passwd -v /etc/shadow:/etc/shadow -v /etc/sudoers:/etc/sudoers -v /tmp/.X11-unix:/tmp/.X11-unix -u 1144 rayburgemeestre/build-starcry-ubuntu:18.04 /bin/bash

.PHONY: starcry
clean:
	docker run -t -v $$PWD:$$PWD --workdir $$PWD rayburgemeestre/build-starcry-ubuntu:18.04 sh -c "cd build && make clean"
	rm -rf CMakeCache.txt
	rm -rf build/CMakeCache.txt
	rm -rf out

.PHONY: dockerize
dockerize:
	docker run $$FLAGS --privileged -t -v $$PWD:$$PWD --workdir $$PWD rayburgemeestre/build-starcry-ubuntu:18.04 /bin/sh -c "make dockerize_run"
	cd out && docker build . -t rayburgemeestre/starcry:latest
	#docker push rayburgemeestre/starcry:latest

.PHONY: dockerize_run
dockerize_run:
	apt update
	apt install python-pip rsync -y
	pip install dockerize
	strip --strip-debug $$PWD/build/starcry
	dockerize --verbose --debug -n -o out $$PWD/build/starcry
