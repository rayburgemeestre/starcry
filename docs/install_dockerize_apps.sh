#!/bin/bash

sudo apt update -y
sudo apt install python3-pip rsync git ncurses-term -y
cd /tmp && git clone https://github.com/larsks/dockerize && cd dockerize && sudo python3 setup.py install
