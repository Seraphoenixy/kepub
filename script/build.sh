#!/bin/bash

set -e

if [[ "$OSTYPE" == "linux-gnu"* ]]; then
  curl -L https://github.com/KaiserLancelot/klib/releases/download/v0.8.6/klib-0.8.6-Linux.deb \
    -o klib.deb
  dpkg -i klib.deb
else
  echo "The system does not support: $OSTYPE"
  exit 1
fi
