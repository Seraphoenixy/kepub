#!/bin/bash

set -e

if [[ "$OSTYPE" == "linux-gnu"* ]]; then
  curl -L https://github.com/KaiserLancelot/klib/releases/download/v0.8.1/klib-v0.8.1-ubuntu-20.04.deb \
    -o klib.deb
  dpkg -i klib.deb
else
  echo "The system does not support: $OSTYPE"
  exit 1
fi
