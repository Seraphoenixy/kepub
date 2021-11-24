# kepub

[![Build](https://github.com/KaiserLancelot/kepub/actions/workflows/build.yml/badge.svg)](https://github.com/KaiserLancelot/kepub/actions/workflows/build.yml)
[![Platform](https://img.shields.io/badge/Platform-Debian%2011-brightgreen)](https://www.debian.org/)
[![Coverage Status](https://coveralls.io/repos/github/KaiserLancelot/kepub/badge.svg?branch=main)](https://coveralls.io/github/KaiserLancelot/kepub?branch=main)
[![GitHub Releases](https://img.shields.io/github/release/KaiserLancelot/kepub)](https://github.com/KaiserLancelot/kepub/releases/latest)
[![GitHub Downloads](https://img.shields.io/github/downloads/KaiserLancelot/kepub/total)](https://github.com/KaiserLancelot/kepub/releases)
[![GitHub License](https://img.shields.io/github/license/KaiserLancelot/kepub)](https://github.com/KaiserLancelot/kepub/blob/main/LICENSE)
[![996.icu](https://img.shields.io/badge/link-996.icu-red.svg)](https://996.icu)

---

Crawl novels from esjzone, ciweimao and sfacg. Generate epub from txt file

## Environment

- Debian 11
- GCC 11 or Clang 13

## Dependency

- [klib](https://github.com/KaiserLancelot/klib)
- [Boost](https://www.boost.org/)
- [gsl-lite](https://github.com/gsl-lite/gsl-lite)
- [CLI11](https://github.com/CLIUtils/CLI11)
- [fmt](https://github.com/fmtlib/fmt)
- [spdlog](https://github.com/gabime/spdlog)
- [simdjson](https://github.com/simdjson/simdjson)
- [pugixml](https://github.com/zeux/pugixml)
- [icu](https://github.com/unicode-org/icu)
- [CPython](https://github.com/python/cpython)
- [indicators](https://github.com/p-ranav/indicators)

## Font

- [source-han-sans](https://github.com/adobe-fonts/source-han-sans)

## Build

```bash
cmake -S . -B build
cmake --build build --config Release -j"$(nproc)"
```

## Install

```bash
sudo cmake --build build --config Release --target install
```

## Uninstall

```bash
sudo cmake --build build --config Release --target uninstall
```

## Usage

### addition

Add new chapter to EPUB file

When using, the current directory must have an EPUB file with the same name

```bash
addition book-name.txt
```

Txt files need to be preprocessed: each volume starts with [VOLUME] plus a space; each chapter starts with [WEB] plus a space; pictures start with [IMAGE] plus a space, followed by the picture number.

### kepub

Generate EPUB file according to the given TXT file

```bash
kepub book-name.txt
```

Txt files need to be preprocessed: the author starts with [AUTHOR] and a newline character; the introduction starts with [INTRO] and a newline character; the postscript starts with [POST] and a newline character; each volume starts with [VOLUME] plus a space; each chapter starts with [WEB] plus a space; pictures start with [IMAGE] plus a space, followed by the picture number.

### esjzone

Crawl the novels on esjzone and generate TXT files, then you can use kepub to generate EPUB files

The first command line parameter is book id

```bash
esjzone 1578022447
```

### ciweimao

Crawl the novels on ciweimao and generate TXT files, then you can use kepub to generate EPUB files

The first command line parameter is book id

```bash
ciweimao 100194379
```

### sfacg

Crawl the novels on sfacg and generate TXT files, then you can use kepub to generate EPUB files

The first command line parameter is book id

```bash
sfacg 263060
```

---

Thanks to [JetBrains](https://www.jetbrains.com/) for donating product licenses to help develop this project <a href="https://www.jetbrains.com/"><img src="logo/jetbrains.svg" width="94" align="center" /></a>
