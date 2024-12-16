FROM docker.io/rayburgemeestre/build-ubuntu:24.04 AS build1

MAINTAINER Ray Burgemeestre

RUN mkdir docs

COPY docs/install_deps.sh docs/
COPY Makefile /

RUN make deps

# warmup emscripten (this will pre-fetch SDL2, and cache it)
RUN /emsdk/upstream/emscripten/em++ -s WASM=1 -s USE_SDL=2 -s USE_SDL_TTF=2 -O3 /dev/null || true

# IDE dependencies (LSP plugin)
# EDIT: we already have it in the new image
#RUN apt install -y clangd-10

# IDE plugin SonarLint requires nodejs
#RUN curl -fsSL https://deb.nodesource.com/setup_18.x | bash -
#RUN apt-get install -y nodejs

# We need a newer Nodejs:
ENV VERSION=v18.18.0
ENV DISTRO=linux-x64
RUN wget https://nodejs.org/dist/$VERSION/node-$VERSION-$DISTRO.tar.xz && \
    mkdir -p /usr/local/lib/nodejs && \
    tar -xJvf node-$VERSION-$DISTRO.tar.xz -C /usr/local/lib/nodejs

#RUN echo "export PATH=/usr/local/lib/nodejs/node-$VERSION-$DISTRO/bin:$PATH" >> /etc/profile
# Let's just put in our global PATH
RUN apt install -y rsync && rsync -raPv /usr/local/lib/nodejs/node-$VERSION-$DISTRO/ /usr/local/

# IDE needs browser as well sometimes
RUN apt install -y firefox

# IDE needs bunch of stuff for markdown viewer
RUN apt install -y libnss3 libnspr4 libatk-bridge2.0-0 libatspi2.0-0

# IDE no longer ships jre
RUN apt install -y openjdk-21-jre

# IDE requires a browser
#RUN apt install apt-transport-https curl && \
#    curl -s https://brave-browser-apt-beta.s3.brave.com/brave-core-nightly.asc | apt-key --keyring /etc/apt/trusted.gpg.d/brave-browser-prerelease.gpg add - && \
#    echo "deb [arch=amd64] https://brave-browser-apt-beta.s3.brave.com/ stable main" | tee /etc/apt/sources.list.d/brave-browser-beta.list && \
#    apt update && \
#    apt install brave-browser-beta

# Integration tests dependencies
RUN apt install -y imagemagick-6.q16 ffmpeg

# xeyes (for troubleshooting X11)
RUN apt install -y x11-apps

# docs/sphinx/breathe
RUN apt install -y doxygen python3-pip
RUN python3 -m pip install --break-system-packages sphinx breathe sphinx_rtd_theme

# clean up cache
RUN apt clean

# fix weird IDs (podman doesn't seem to like it much)
# EDIT: seems no longer required, actually breaks stuff now
# EDIT 2 : see below, got issues with lchown again..
RUN chown root:root /emsdk -R

COPY docs/entrypoint.sh /entrypoint.sh

RUN chmod a+rx /entrypoint.sh

COPY docs/starcry-dev-wrapper.sh /workdir/docs/starcry-dev-wrapper.sh

# This saves diskspace, but is also very inconvenient in combination with Tilt (reuploading 6 GB all the time)
# Keeping it: will try to configure Tilt to not rebuild all the time instead. Having some issues with pulling
# non-squashed docker image:
# failed to register layer: failed to Lchown "/emsdk/upstream/emscripten/cache/ports/harfbuzz/harfbuzz-3.2.0" for UID 172486, GID 89939 (try increasing the number of subordinate IDs in /etc/subuid and /etc/subgid): lchown /emsdk/upstream/emscripten/cache/ports/harfbuzz/harfbuzz-3.2.0: invalid argument
# EDIT: this is reason to re-enable that chown on /emsdk

#FROM scratch
#
#COPY --from=build1 / /
#
#ENTRYPOINT ["/entrypoint.sh"]
