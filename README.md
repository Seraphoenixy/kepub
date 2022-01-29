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
- [mimalloc](https://github.com/microsoft/mimalloc)
- [Boost](https://www.boost.org/)
- [gsl-lite](https://github.com/gsl-lite/gsl-lite)
- [CLI11](https://github.com/CLIUtils/CLI11)
- [fmt](https://github.com/fmtlib/fmt)
- [spdlog](https://github.com/gabime/spdlog)
- [simdjson](https://github.com/simdjson/simdjson)
- [pugixml](https://github.com/zeux/pugixml)
- [icu](https://github.com/unicode-org/icu)
- [OpenCC](https://github.com/BYVoid/OpenCC)
- [indicators](https://github.com/p-ranav/indicators)

## Font

- [source-han-sans](https://github.com/adobe-fonts/source-han-sans)
- [fonttools](https://github.com/fonttools/fonttools)

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

```bash
sfacg 263060
kepub book-name.txt
```

## Reference

- https://blog.csdn.net/qq_38851536/article/details/117828334

---

Thanks to [JetBrains](https://www.jetbrains.com/) for donating product licenses to help develop this project <a href="https://www.jetbrains.com/"><img src="logo/jetbrains.svg" width="94" align="center" /></a>
