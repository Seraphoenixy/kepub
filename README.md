# epub

[![Build](https://github.com/KaiserLancelot/epub/workflows/Build/badge.svg)](https://github.com/KaiserLancelot/epub/actions?query=workflow%3ABuild)
[![GitHub License](https://img.shields.io/github/license/KaiserLancelot/epub)](https://raw.githubusercontent.com/KaiserLancelot/epub/master/LICENSE)
[![996.icu](https://img.shields.io/badge/link-996.icu-red.svg)](https://996.icu)
[![GitHub Releases](https://img.shields.io/github/release/KaiserLancelot/epub)](https://github.com/KaiserLancelot/epubv/releases/latest)
[![GitHub Downloads](https://img.shields.io/github/downloads/KaiserLancelot/epubv/total)](https://github.com/KaiserLancelot/epubv/releases)
[![Bugs](https://img.shields.io/github/issues/KaiserLancelot/epubv/bug)](https://github.com/KaiserLancelot/epubv/issues?q=is%3Aopen+is%3Aissue+label%3Abug)

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

```shell
cmake -S . -B build
cmake --build build --config Release
```

#### Install

```shell
sudo cmake --build build --config Release --target install
```

#### Use

The font file needs to be in the working directory

- Generated from the integrated txt file in https://gitlab.com/demonovel/epub-txt/-/tree/master (introduction and packaging need to be done manually)

```shell
demonovel xxx.txt
```

- Generated from the integrated txt file in https://mega.nz/#F!dw4DzZhJ!RNFlsWOf-QTOZJvsMmqLlA (introduction, author, and packaging need to be done manually)

```shell
masiro xxx.txt
```
