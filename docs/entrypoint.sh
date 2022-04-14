#!/bin/bash

set -e
set -o pipefail

if [[ "$_UID" != "" ]]; then
  groupadd -g $_GID user || true
  useradd -r -u $_UID -g $_GID user || true
  usermod -a -G wheel user 1>/dev/null 2>&1 || true
  mkdir -p /home/user
  chown $_UID:$_GID /home/user
  echo 'user ALL=(ALL) NOPASSWD:ALL' >> /etc/sudoers
  set +e
  echo 'export PATH=/usr/local/bin:$PATH' | sudo tee -a /home/user/.bashrc >/dev/null
  chown $_UID:$_GID /home/user/.bashrc /tmp/.ccache /tmp/.emscripten_cache
  ln -sf /tmp/.ccache /home/user/.ccache
  ln -sf /tmp/.emscripten_cache /home/user/.emscripten_cache
  sudo -u $(id -u -n $_UID) -g $(id -g -n $_UID) /bin/bash "$@"
else
  echo 'export PATH=/usr/local/bin:$PATH' | sudo tee -a /root/.bashrc >/dev/null
  /bin/bash "$@"
fi
