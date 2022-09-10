FROM docker.io/rayburgemeestre/build-ubuntu:22.04 AS build1

MAINTAINER Ray Burgemeestre

RUN mkdir docs

COPY docs/install_deps.sh docs/
COPY docs/install_dockerize_deps.sh docs/
COPY Makefile /

RUN make deps
RUN make dockerize_deps

# warmup emscripten (this will pre-fetch SDL2, and cache it)
RUN /emsdk/upstream/emscripten/em++ -s WASM=1 -s USE_SDL=2 -s USE_SDL_TTF=2 -O3 /dev/null || true

# IDE dependencies (LSP plugin)
# EDIT: we already have it in the new image
#RUN apt install -y clangd-10

# IDE plugin SonarLint requires nodejs
RUN curl -fsSL https://deb.nodesource.com/setup_18.x | sudo -E bash -
RUN sudo apt-get install -y nodejs

# IDE needs browser as well sometimes
RUN sudo apt install -y firefox

# Integration tests dependencies
RUN apt install -y imagemagick-6.q16 ffmpeg

# xeyes (for troubleshooting X11)
RUN apt install -y x11-apps

# clean up cache
RUN apt clean

# fix weird IDs (podman doesn't seem to like it much)
RUN chown root.root /emsdk -R

COPY docs/entrypoint.sh /entrypoint.sh

RUN chmod a+rx /entrypoint.sh

FROM scratch

COPY --from=build1 / /

ENTRYPOINT ["/entrypoint.sh"]
