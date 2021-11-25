#!/bin/bash

set -e

if [[ "$OSTYPE" == "linux-gnu"* ]]; then
  curl -L https://github.com/KaiserLancelot/klib/releases/download/v0.8.7/klib-0.8.7-Linux.deb \
    -o klib.deb
  sudo dpkg -i klib.deb

  curl -L https://github.com/KaiserLancelot/kpkg/releases/download/v0.9.1/kpkg-0.9.1-Linux.deb \
    -o kpkg.deb
  sudo dpkg -i kpkg.deb

  kpkg install pyftsubset
else
  echo "The system does not support: $OSTYPE"
  exit 1
fi
