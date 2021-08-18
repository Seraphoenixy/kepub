#!/bin/bash

set -e

if [[ "$OSTYPE" == "linux-gnu"* ]]; then
  sudo add-apt-repository -y ppa:ubuntu-toolchain-r/test
  sudo apt update
  sudo apt install -y gcc-11 g++-11

  wget https://apt.llvm.org/llvm.sh
  chmod +x llvm.sh
  sudo ./llvm.sh 12
  sudo apt install -y clang-tidy-12 valgrind

  sudo apt install -y make cmake re2c autoconf automake libtool m4 tcl

  sudo update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-11 400
  sudo update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-11 400
  sudo update-alternatives --install /usr/bin/gcov gcov /usr/bin/gcov-11 400
  sudo update-alternatives --install /usr/bin/clang clang /usr/bin/clang-12 400
  sudo update-alternatives --install /usr/bin/clang++ clang++ /usr/bin/clang++-12 400
  sudo update-alternatives --install /usr/bin/lld lld /usr/bin/lld-12 400
  sudo update-alternatives --install /usr/bin/ld.lld ld.lld /usr/bin/ld.lld-12 400
  sudo update-alternatives --install /usr/bin/llvm-ar llvm-ar /usr/bin/llvm-ar-12 400
  sudo update-alternatives --install /usr/bin/llvm-nm llvm-nm /usr/bin/llvm-nm-12 400
  sudo update-alternatives --install /usr/bin/llvm-ranlib llvm-ranlib /usr/bin/llvm-ranlib-12 400
  sudo update-alternatives --install /usr/bin/clang-tidy clang-tidy /usr/bin/clang-tidy-12 400
  sudo update-alternatives --install /usr/bin/llvm-profdata llvm-profdata /usr/bin/llvm-profdata-12 400
  sudo update-alternatives --install /usr/bin/llvm-cov llvm-cov /usr/bin/llvm-cov-12 400
  sudo update-alternatives --install /usr/bin/llvm-symbolizer llvm-symbolizer /usr/bin/llvm-symbolizer-12 400

  curl -L https://github.com/KaiserLancelot/klib/releases/download/v0.2.14/klib-v0.2.14-ubuntu-20.04.deb \
      -o klib.deb
    sudo dpkg -i klib.deb

  curl -L https://github.com/KaiserLancelot/kpkg/releases/download/v0.3.10/kpkg-v0.3.10-ubuntu-20.04.deb \
      -o kpkg.deb
  sudo dpkg -i kpkg.deb

  if [ ! -d "dependencies" ]; then
    echo "mkdir dependencies"
    mkdir dependencies
  fi
  cd dependencies

  kpkg install lcov catch2 fmt boost icu pugixml
  sudo ldconfig
else
  echo "The system does not support: $OSTYPE"
  exit 1
fi
