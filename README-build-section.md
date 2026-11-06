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

### Web browser (WebAssembly)

The same game also builds for the browser with [Emscripten](https://emscripten.org/) and raylib's `PLATFORM_WEB`, following raylib's [Working for Web](https://github.com/raysan5/raylib/wiki/Working-for-Web-(HTML5)) guide. This build doesn't use the raylib or compiler from the sections above.

**Prerequisites:** `bash`, `git`, `python3`, `tar` with `xz`, `curl` (or Python's built-in `urllib`), and GNU `make`. The first build also needs internet access and about 1.7 GB of disk space. You don't need a system C/C++ compiler, because the Emscripten SDK brings its own clang. On Windows, use WSL.

```bash
make web     # builds dist/web/index.html, index.js, index.wasm
make serve   # serves them on http://127.0.0.1:8080/  (make serve PORT=9000 for another port)
```

Open http://127.0.0.1:8080/ and press Ctrl+C to stop the server. The page has to be served over HTTP: opening `index.html` directly from disk (`file://`) doesn't work.

The first `make web` clones raylib 6.2 (the version the release workflow uses) and the Emscripten SDK 5.0.3 (the version raylib 6.2's own web CI uses) into `.web-deps/`. Both are pinned to exact commits and git-ignored. It then installs Emscripten there, which downloads about 385 MB. Later builds reuse these files and take about 20 seconds. To share the game, copy the three files in `dist/web/` to any static web host.

How the web version differs from the desktop version:
- The browser drives the frame loop through `emscripten_set_main_loop`, instead of `SetTargetFPS(60)` and the `while` loop. The physics uses the frame time, so speeds are the same.
- ESC doesn't quit; the browser tab is the window.
- The canvas is the same fixed 1920×1080 as the desktop window. In a smaller browser window the page scrolls.
