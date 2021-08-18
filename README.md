# kepub

[![Open in Visual Studio Code](https://open.vscode.dev/badges/open-in-vscode.svg)](https://open.vscode.dev/KaiserLancelot/kepub)
[![Build](https://github.com/KaiserLancelot/kepub/actions/workflows/build.yml/badge.svg)](https://github.com/KaiserLancelot/kepub/actions/workflows/build.yml)
[![Coverage Status](https://coveralls.io/repos/github/KaiserLancelot/kepub/badge.svg?branch=main)](https://coveralls.io/github/KaiserLancelot/kepub?branch=main)
[![GitHub License](https://img.shields.io/github/license/KaiserLancelot/kepub)](https://github.com/KaiserLancelot/kepub/blob/main/LICENSE)
[![996.icu](https://img.shields.io/badge/link-996.icu-red.svg)](https://996.icu)
[![GitHub Releases](https://img.shields.io/github/release/KaiserLancelot/kepub)](https://github.com/KaiserLancelot/kepub/releases/latest)
[![GitHub Downloads](https://img.shields.io/github/downloads/KaiserLancelot/kepub/total)](https://github.com/KaiserLancelot/kepub/releases)
[![Bugs](https://img.shields.io/github/issues/KaiserLancelot/kepub/bug)](https://github.com/KaiserLancelot/kepub/issues?q=is%3Aopen+is%3Aissue+label%3Abug)

---

Generate epub

## Environment

- Ubuntu 20.04
- GCC 11 or Clang 12

## Dependency

- [fmt](https://github.com/fmtlib/fmt)
- [pugixml](https://github.com/zeux/pugixml)
- [icu](https://github.com/unicode-org/icu)
- [boost](https://www.boost.org/)
- [klib](https://github.com/KaiserLancelot/klib)
- [spdlog](https://github.com/gabime/spdlog)

## Font

- [source-han-sans](https://github.com/adobe-fonts/source-han-sans)

## Reference

- [Cirno-go](https://github.com/zsakvo/Cirno-go)

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

---

Thanks to [JetBrains](https://www.jetbrains.com/) for donating product licenses to help develop this project <a href="https://www.jetbrains.com/"><img src="logo/jetbrains.svg" width="94" align="center" /></a>
