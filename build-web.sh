#!/usr/bin/env bash
# ------------------------------------------------------------------
# simplegravity.cpp - WebAssembly build (raylib PLATFORM_WEB + Emscripten)
# Usually run through `make web`. Output: dist/web/{index.html,index.js,index.wasm}
#
# First run downloads pinned raylib + Emscripten SDK into .web-deps/
# (about 1.7 GB, git-ignored); later runs reuse them.
# ------------------------------------------------------------------
set -euo pipefail
cd "$(dirname "$0")"

RAYLIB_TAG=6.2                                            # same raylib as the release workflow
RAYLIB_COMMIT=02566453880e06bec97f5b8160fa6fe1e98397ca
EMSDK_VERSION=5.0.3                                       # raylib 6.2's own web CI version
EMSDK_COMMIT=a620cf1d71c62dfdfbb0c01fe0a371e2af2dda6c

DEPS=.web-deps
OUT=dist/web

fetch() { # dir url tag commit
    [ -d "$1/.git" ] || git clone -q --depth 1 --branch "$3" "$2" "$1"
    local have; have=$(git -C "$1" rev-parse HEAD)
    [ "$have" = "$4" ] || { echo "error: $1 is at $have, expected $4" >&2; exit 1; }
}
fetch "$DEPS/raylib" https://github.com/raysan5/raylib "$RAYLIB_TAG" "$RAYLIB_COMMIT"
fetch "$DEPS/emsdk" https://github.com/emscripten-core/emsdk "$EMSDK_VERSION" "$EMSDK_COMMIT"

if [ ! -x "$DEPS/emsdk/upstream/emscripten/emcc" ]; then
    command -v xz >/dev/null || { echo "error: xz is needed to unpack the Emscripten SDK (install xz-utils)" >&2; exit 1; }
    "$DEPS/emsdk/emsdk" install "$EMSDK_VERSION"
fi
"$DEPS/emsdk/emsdk" activate "$EMSDK_VERSION" >/dev/null
# shellcheck disable=SC1091
EMSDK_QUIET=1 source "$DEPS/emsdk/emsdk_env.sh"

# raylib for the web: same modules and flags as raylib's src/Makefile
# with PLATFORM=PLATFORM_WEB (release mode).
RL="$DEPS/raylib/src"
OBJ="$DEPS/build"
mkdir -p "$OBJ"
objs=()
for m in rcore rshapes rtextures rtext rmodels raudio; do
    emcc -c "$RL/$m.c" -o "$OBJ/$m.o" -Wall -D_DEFAULT_SOURCE -DPLATFORM_WEB -DGRAPHICS_API_OPENGL_ES2 \
        -Wno-missing-braces -Werror=pointer-arith -fno-strict-aliasing -std=gnu99 -Os \
        -I"$RL" -I"$RL/external/glfw/include"
    objs+=("$OBJ/$m.o")
done
rm -f "$OBJ/libraylib.web.a"
emar rcs "$OBJ/libraylib.web.a" "${objs[@]}"

# The game. -O2 -Wall -Wextra -std=c++17 as in the Makefile.
#  -Wno-c++11-narrowing: clang rejects the int -> unsigned char Color brace init
#    in gravityPlain::draw() that g++/MSVC only warn about (behaviour unchanged).
#  ALLOW_MEMORY_GROWTH: there is no limit on bodies, every click adds one.
mkdir -p "$OUT"
em++ simplegravity.cpp "$OBJ/libraylib.web.a" -o "$OUT/index.html" \
    -O2 -Wall -Wextra -std=c++17 -Wno-c++11-narrowing -I. -I"$RL" -DPLATFORM_WEB \
    -sUSE_GLFW=3 -sALLOW_MEMORY_GROWTH=1 -sMINIFY_HTML=0 --shell-file web/shell.html

echo "built $OUT/index.html - run 'make serve' and open http://127.0.0.1:8080/"
