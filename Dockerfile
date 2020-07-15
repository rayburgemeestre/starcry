FROM rayburgemeestre/build-ubuntu:18.04

MAINTAINER Ray Burgemeestre

COPY Makefile /

RUN make deps

RUN make client_deps

# warmup emscripten (this will pre-fetch SDL2, and cache it)
RUN /emsdk/upstream/emscripten/em++ -s WASM=1 -s USE_SDL=2 -O3 /dev/null || true
