# ------------------------------------------------------------------
# simplegravity.cpp - Makefile  [kit v4]
#
#   v4: removed the "prefer static libglfw3.a" branch - MSYS2's
#       libraylib.a references glfw through __imp_ import thunks, so
#       glfw must link via its import library (glfw3.dll). The
#       Release zip ships glfw3.dll next to the exe; libstdc++ and
#       libgcc stay statically linked.
#   v3: recipe lines use real Tab characters again (v2 shipped
#       spaces, which made make fail with "missing separator").
#   v2: Windows static link adds -lwinmm (fixes
#       "undefined reference to __imp_timeBeginPeriod").
#
#   make        build the executable
#   make run    build and run it
#   make clean  delete the built executable
#
# Needs raylib installed (pkg-config is used to find its flags).
# raygui.h is header-only and lives next to this file.
#
# NOTE: the indented command lines below MUST start with a real
# Tab character, not spaces. If you copy-paste and make says
# "missing separator", fix the indentation to tabs.
# ------------------------------------------------------------------

CXX      ?= g++
CXXFLAGS ?= -O2 -Wall -Wextra -std=c++17
# Point this at your raylib lib folder if raylib isn't installed system-wide,
# e.g.  make LDFLAGS="-L/home/you/raylib/lib"
LDFLAGS  ?=

# Windows (MSYS2/MinGW) builds produce a .exe automatically
ifeq ($(OS),Windows_NT)
    TARGET := simplegravity.exe
else
    TARGET := simplegravity
endif

# Ask pkg-config where raylib is installed.
RAYLIB_CFLAGS := $(shell pkg-config --cflags raylib 2>/dev/null)
RAYLIB_LIBDIR := $(shell pkg-config --variable=libdir raylib 2>/dev/null)

# Prefer the STATIC raylib library when it exists: the executable then
# runs on machines that don't have raylib installed, which is exactly
# what you want for binaries you share on GitHub Releases.
# (MSYS2, Homebrew and the official raylib releases all ship libraylib.a;
#  Debian/Ubuntu's apt package does not, so there we fall back to dynamic.)
RAYLIB_STATIC := $(RAYLIB_LIBDIR)/libraylib.a
RAYLIB_TAIL   := $(shell pkg-config --static --libs raylib 2>/dev/null | sed -e 's/-lraylib//')

ifneq ($(wildcard $(RAYLIB_STATIC)),)
    RAYLIB_LIBS := $(RAYLIB_STATIC) $(RAYLIB_TAIL)
    ifeq ($(OS),Windows_NT)
        # MSYS2's raylib.pc lists no Windows system libraries for a static
        # link, which makes the build fail with e.g.
        #   undefined reference to `__imp_timeBeginPeriod'
        # (winmm is the Windows multimedia timer library). The compiler
        # driver already links user32/shell32/kernel32 by default; listing
        # them explicitly anyway is harmless.
        WIN_LIBS  := -lwinmm -luser32 -lshell32 -lkernel32 -lgdi32 -lopengl32
        RAYLIB_LIBS := $(RAYLIB_STATIC) $(RAYLIB_TAIL) $(WIN_LIBS)
        # NOTE: glfw must link DYNAMICALLY (via its import library):
        # MSYS2's static libraylib.a references glfw through __imp_
        # thunks, so the static libglfw3.a cannot satisfy them. The exe
        # therefore needs glfw3.dll at runtime - release.yml copies it
        # from /ucrt64/bin into the Release zip for you.
        # Embed the GCC runtime so the exe also does not need
        # libstdc++-6.dll / libgcc_s_seh-1.dll sitting next to it.
        LDFLAGS += -static-libgcc -static-libstdc++
    endif
else
    RAYLIB_LIBS := $(shell pkg-config --libs raylib 2>/dev/null)
    ifeq ($(strip $(RAYLIB_LIBS)),)
        RAYLIB_LIBS := -lraylib
    endif
endif
# Tip: you can force different link flags with e.g.
#   make RAYLIB_LIBS="-lraylib"          (dynamic linking)

SRC := simplegravity.cpp

.PHONY: all run clean

all: $(TARGET)

# raygui.h is a prerequisite too, so changing it triggers a rebuild
$(TARGET): $(SRC) raygui.h
	$(CXX) $(CXXFLAGS) $(RAYLIB_CFLAGS) $(SRC) -o $(TARGET) $(LDFLAGS) $(RAYLIB_LIBS)

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET)
