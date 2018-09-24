SHELL:=/bin/bash

.PHONY: starcry_ubuntu1804
starcry_ubuntu1804:
	bash build_docker_ubuntu1804.sh "mkdir -p build && cd build && cmake -DLIB_PREFIX_DIR=/usr/local/src/starcry/ .. && make -j $$(nproc)" && cp -prv build/starcry .

.PHONY: debug
debug:
	bash build_docker_ubuntu1804.sh "mkdir -p build && cd build && cmake -DLIB_PREFIX_DIR=/usr/local/src/starcry/ -DDEBUG=on .. && make -j $$(nproc)" && cp -prv build/starcry .

.PHONY: starcry_ubuntu1604
starcry_ubuntu1604:
	bash build_docker_ubuntu1604.sh "mkdir -p build && cd build && cmake -DLIB_PREFIX_DIR=/usr/local/src/starcry/ .. && make -j $$(nproc)" && cp -prv build/starcry .

.PHONY: prepare_local
prepare_local:
	bash prepare_linux.sh

.PHONY: local
local:
	mkdir -p build && cd build && cmake -DLIB_PREFIX_DIR=/home/trigen/projects/starcry -DDEBUG=1 .. && make -j $$(nproc)

.PHONY: starcry
clean:
	bash build_docker_ubuntu1804.sh "cd build && make clean"
	rm -rf CMakeCache.txt
	rm -rf build/CMakeCache.txt
	rm -rf out

.PHONY: docker1804
docker1804:
	cd docker && bash build_Ubuntu18.04.sh

.PHONY: docker1804nocache
docker1804nocache:
	cd docker && bash build_Ubuntu18.04.sh --no-cache

.PHONY: docker1604
docker1604:
	cd docker && bash build_Ubuntu16.04.sh

.PHONY: docker1604nocache
docker1604nocache:
	cd docker && bash build_Ubuntu16.04.sh --no-cache

.PHONY: docker1604publish
docker1604publish:
	docker tag sc_build_ubuntu:16.04 rayburgemeestre/sc_build_ubuntu:16.04
	docker push rayburgemeestre/sc_build_ubuntu:16.04

.PHONY: docker1804publish
docker1804publish:
	docker tag sc_build_ubuntu:18.04 rayburgemeestre/sc_build_ubuntu:18.04
	docker push rayburgemeestre/sc_build_ubuntu:18.04

.PHONY: dockerize
dockerize:
	bash build_docker_ubuntu1804.sh "make dockerize_run"
	cd out && docker build . -t rayburgemeestre/starcry && docker push rayburgemeestre/starcry

.PHONY: dockerize_run
dockerize_run:
	apt install python-pip rsync -y
	pip install dockerize
	strip --strip-debug /projects/starcry/starcry
	dockerize --verbose --debug -n -o out /projects/starcry/starcry
