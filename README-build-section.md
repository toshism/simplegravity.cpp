# Build instructions — paste this section into your README.md

## Building

**Prerequisites:** a C++17 compiler (g++, clang++ or MSVC) and [raylib](https://www.raylib.com/) installed.
`raygui.h` is included in this repo (it's header-only, MIT licensed by Ramon Santamaria).

### Linux (Debian/Ubuntu)

```bash
sudo apt install build-essential pkg-config libraylib-dev
make
```

### Windows (MSYS2/UCRT64)

Install [MSYS2](https://www.msys2.org/), open the **MSYS2 UCRT64** shell, then:

```bash
pacman -S mingw-w64-ucrt-x86_64-gcc mingw-w64-ucrt-x86_64-make mingw-w64-ucrt-x86_64-pkgconf mingw-w64-ucrt-x86_64-raylib
mingw32-make
```

### macOS

```bash
brew install raylib pkg-config
make
```

### Running

```bash
make run       # or just double-click / ./simplegravity
```

### Prebuilt binaries

No compiler? Download a ready-made build for Windows, Linux or macOS
from the [Releases](../../releases) page.
