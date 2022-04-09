FROM rayburgemeestre/build-ubuntu:20.04

MAINTAINER Ray Burgemeestre

RUN mkdir docs

COPY docs/install_deps.sh docs/
COPY docs/install_client_deps.sh docs/
COPY docs/install_dockerize_deps.sh docs/
COPY Makefile /

RUN make deps
RUN make client_deps
RUN make dockerize_deps

# warmup emscripten (this will pre-fetch SDL2, and cache it)
RUN /emsdk/upstream/emscripten/em++ -s WASM=1 -s USE_SDL=2 -s USE_SDL_TTF=2 -O3 /dev/null || true

# IDE dependencies (LSP plugin)
RUN apt install -y clangd-10

# Integration tests dependencies
RUN apt install -y imagemagick-6.q16 ffmpeg

COPY docs/entrypoint.sh /entrypoint.sh

RUN chmod a+rx /entrypoint.sh

ENTRYPOINT ["/entrypoint.sh"]