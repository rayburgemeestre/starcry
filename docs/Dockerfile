FROM ubuntu:22.04

COPY . /

RUN apt update && apt install mesa-common-dev libxrandr2 libgl1 ncurses-term -y && apt clean && apt autoremove -y

ENV TERM=xterm-16color

