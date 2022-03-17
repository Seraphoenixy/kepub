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

## Third party

- klib ([MIT License](https://github.com/KaiserLancelot/klib/blob/main/LICENSE))
- mimalloc ([MIT License](https://github.com/microsoft/mimalloc/blob/master/LICENSE))
- parallel-hashmap ([Apache License 2.0](https://github.com/greg7mdp/parallel-hashmap/blob/master/LICENSE))
- Boost ([Boost License](https://www.boost.org/users/license.html))
- GSL ([MIT License](https://github.com/Microsoft/GSL/blob/master/LICENSE))
- CLI11 ([License](https://github.com/CLIUtils/CLI11/blob/main/LICENSE))
- fmt ([License](https://github.com/fmtlib/fmt/blob/master/LICENSE.rst))
- simdjson ([Apache License 2.0](https://github.com/simdjson/simdjson/blob/master/LICENSE))
- pugixml ([MIT License](https://github.com/zeux/pugixml/blob/master/LICENSE.md))
- OpenCC ([Apache License 2.0](https://github.com/BYVoid/OpenCC/blob/master/LICENSE))
- indicators ([MIT License](https://github.com/p-ranav/indicators/blob/master/LICENSE))
- Catch2 ([Boost License](https://github.com/catchorg/Catch2/blob/devel/LICENSE.txt))

## Build environment

- [kenv](https://github.com/KaiserLancelot/kenv)

## Usage

- Make sure you are using a recent version of Debian/Ubuntu/WSL2 (other Linux systems have not been tested, but should work)
- The program requires AVX2 instruction set, the CPU is at least Intel Haswell (2013), if you use a virtual machine, you need additional settings about the CPU
- Install the DEB package (or use the compressed package) or compile it yourself
- Then you can start using, for example:

```bash
sfacg book-id
kepub book-name.txt
```

---

Thanks to [JetBrains](https://www.jetbrains.com/) for donating product licenses to help develop this project <a href="https://www.jetbrains.com/"><img src="logo/jetbrains.svg" width="94" align="center" /></a>
