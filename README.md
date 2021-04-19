# epub

[![Build](https://github.com/KaiserLancelot/kepub/actions/workflows/build.yml/badge.svg)](https://github.com/KaiserLancelot/kepub/actions/workflows/build.yml)
[![Coverage Status](https://coveralls.io/repos/github/KaiserLancelot/kepub/badge.svg?branch=main)](https://coveralls.io/github/KaiserLancelot/kepub?branch=main)
[![GitHub License](https://img.shields.io/github/license/KaiserLancelot/kepub)](https://raw.githubusercontent.com/KaiserLancelot/kepub/master/LICENSE)
[![996.icu](https://img.shields.io/badge/link-996.icu-red.svg)](https://996.icu)
[![GitHub Releases](https://img.shields.io/github/release/KaiserLancelot/kepub)](https://github.com/KaiserLancelot/kepub/releases/latest)
[![GitHub Downloads](https://img.shields.io/github/downloads/KaiserLancelot/kepub/total)](https://github.com/KaiserLancelot/kepub/releases)
[![Bugs](https://img.shields.io/github/issues/KaiserLancelot/kepub/bug)](https://github.com/KaiserLancelot/kepub/issues?q=is%3Aopen+is%3Aissue+label%3Abug)

Generate epub

# Quick Start

#### Environment:

- Linux
- gcc/clang(Request to support C++20)

#### Libraries:

- fmt
- Boost
- ICU

#### Build

```bash
cmake -S . -B build
cmake --build build --config Release
```

#### Install

```bash
sudo cmake --build build --config Release --target install
```

#### Use

The font file needs to be in the working directory

- Generated from the integrated txt file in https://gitlab.com/demonovel/epub-txt/-/tree/master (introduction and packaging need to be done manually)

```bash
demonovel xxx.txt
```

- Generated from the integrated txt file in https://mega.nz/#F!dw4DzZhJ!RNFlsWOf-QTOZJvsMmqLlA (introduction, author, and packaging need to be done manually)

```bash
masiro xxx.txt
```
