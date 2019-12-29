FROM rayburgemeestre/build-ubuntu:18.04

MAINTAINER Ray Burgemeestre

COPY Makefile /

RUN make deps

