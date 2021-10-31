#!/bin/bash

set -e

if [[ "$OSTYPE" == "linux-gnu"* ]]; then
  sudo add-apt-repository -y ppa:ubuntu-toolchain-r/test
  sudo apt update
  sudo apt install -y gcc-11 g++-11

  curl -L https://apt.llvm.org/llvm.sh -o llvm.sh
  chmod +x llvm.sh
  sudo ./llvm.sh 13
  sudo apt install -y clang-tidy-13 valgrind

  sudo apt install -y make cmake autoconf automake autotools-dev autopoint libtool m4 tcl

  sudo update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-11 400
  sudo update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-11 400
  sudo update-alternatives --install /usr/bin/gcov gcov /usr/bin/gcov-11 400
  sudo update-alternatives --install /usr/bin/clang clang /usr/bin/clang-13 400
  sudo update-alternatives --install /usr/bin/clang++ clang++ /usr/bin/clang++-13 400
  sudo update-alternatives --install /usr/bin/lld lld /usr/bin/lld-13 400
  sudo update-alternatives --install /usr/bin/ld.lld ld.lld /usr/bin/ld.lld-13 400
  sudo update-alternatives --install /usr/bin/llvm-ar llvm-ar /usr/bin/llvm-ar-13 400
  sudo update-alternatives --install /usr/bin/llvm-nm llvm-nm /usr/bin/llvm-nm-13 400
  sudo update-alternatives --install /usr/bin/llvm-ranlib llvm-ranlib /usr/bin/llvm-ranlib-13 400
  sudo update-alternatives --install /usr/bin/clang-tidy clang-tidy /usr/bin/clang-tidy-13 400
  sudo update-alternatives --install /usr/bin/llvm-profdata llvm-profdata /usr/bin/llvm-profdata-13 400
  sudo update-alternatives --install /usr/bin/llvm-cov llvm-cov /usr/bin/llvm-cov-13 400
  sudo update-alternatives --install /usr/bin/llvm-symbolizer llvm-symbolizer /usr/bin/llvm-symbolizer-13 400

  curl -L https://github.com/KaiserLancelot/klib/releases/download/v0.7.8/klib-v0.7.8-ubuntu-20.04.deb \
    -o klib.deb
  sudo dpkg -i klib.deb

  curl -L https://github.com/KaiserLancelot/kpkg/releases/download/v0.6.10/kpkg-v0.6.10-ubuntu-20.04.deb \
    -o kpkg.deb
  sudo dpkg -i kpkg.deb

  if [ ! -d "dependencies" ]; then
    echo "mkdir dependencies"
    mkdir dependencies
  fi
  cd dependencies

  kpkg install lcov catch2 fmt boost icu pugixml spdlog cli11 indicators
  sudo ldconfig
else
  echo "The system does not support: $OSTYPE"
  exit 1
fi
