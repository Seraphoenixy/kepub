# epub

[![CI](https://github.com/KaiserLancelot/epub/workflows/CI/badge.svg)](https://github.com/KaiserLancelot/epub/actions)
[![License](https://img.shields.io/github/license/KaiserLancelot/epub)](LICENSE)
[![Badge](https://img.shields.io/badge/link-996.icu-%23FF4D5B.svg?style=flat-square)](https://996.icu/#/en_US)
[![Lines](https://tokei.rs/b1/github/KaiserLancelot/epub)](https://github.com/Aaronepower/tokei)

Generate epub

# Quick Start

#### Environment:

- Linux
- gcc/clang(Request to support C++20)

#### Libraries:

- fmt
- Boost
- ICU

#### Tools

- curl
- zip

#### Build

```bash
mkdir build && cd build
cmake ..
cmake --build . -j$(nproc)
```

#### Install

```bash
make install
```

#### Use

- Generated from the integrated txt file in https://gitlab.com/demonovel/epub-txt/-/tree/master(introduction and packaging need to be done manually)

```bash
demonovel xxx.txt
```

- Obtain and generate from https://esjzone.cc/

```bash
esjzone 12345678
```
