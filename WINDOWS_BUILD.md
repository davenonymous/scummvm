# Windows Cross-Compilation Guide

This guide documents how to cross-compile ScummVM for Windows x86_64 from Linux
using the MinGW-w64 toolchain and MSYS2 pre-built dependency packages.

## Prerequisites

- Ubuntu/Debian Linux (or derivative)
- `sudo` access to install packages
- Internet access to download MSYS2 packages

---

## Step 1 — Install the MinGW-w64 Toolchain

```sh
sudo apt install mingw-w64 mingw-w64-tools libz-mingw-w64-dev zstd
```

This installs:
- `x86_64-w64-mingw32-g++` — the cross-compiler
- `x86_64-w64-mingw32-windres` — Windows resource compiler
- `x86_64-w64-mingw32-ar`, `ranlib`, `strip` — binutils
- `zstd` — needed to extract MSYS2 `.pkg.tar.zst` packages

### Switch to the POSIX thread model

The apt package installs two GCC variants. The **posix** variant must be active
— it supports `std::thread`, `std::mutex`, and C++ exceptions. The **win32**
variant disables these and will produce linker errors when building the SCUMM
API.

```sh
sudo update-alternatives --set x86_64-w64-mingw32-g++ \
  /usr/bin/x86_64-w64-mingw32-g++-posix
sudo update-alternatives --set x86_64-w64-mingw32-gcc \
  /usr/bin/x86_64-w64-mingw32-gcc-posix
```

Verify the correct variant is active:

```sh
x86_64-w64-mingw32-g++ --version
# Should print: x86_64-w64-mingw32-g++ (GCC) 13-posix
```

---

## Step 2 — Create a Windows Dependency Sysroot

Since nearly none of the required libraries (SDL2, Vorbis, FLAC, etc.) ship as
MinGW-compatible apt packages, we fetch them from the
[MSYS2 package repository](https://packages.msys2.org/). The packages are
extracted into a local sysroot directory — no full MSYS2 installation needed.

### 2a — Download the MSYS2 Package Database

```sh
mkdir -p ~/mingw-deps/pkgs ~/mingw-deps/sysroot
curl -L -o ~/mingw-deps/mingw64.db "https://mirror.msys2.org/mingw/mingw64/mingw64.db"
mkdir -p ~/mingw-deps/db-extract
tar -xf ~/mingw-deps/mingw64.db -C ~/mingw-deps/db-extract
```

Use the database to look up exact package filenames:

```sh
ls ~/mingw-deps/db-extract/ | grep "^mingw-w64-x86_64-SDL2-"
# e.g. mingw-w64-x86_64-SDL2-2.32.10-1
```

### 2b — Download Required Packages

```sh
MIRROR="https://mirror.msys2.org/mingw/mingw64"
DEST=~/mingw-deps/pkgs

PACKAGES=(
  # Core libraries
  "mingw-w64-x86_64-SDL2-2.32.10-1-any.pkg.tar.zst"
  "mingw-w64-x86_64-SDL2_net-2.2.0-1-any.pkg.tar.zst"
  "mingw-w64-x86_64-flac-1.5.0-1-any.pkg.tar.zst"
  "mingw-w64-x86_64-freetype-2.14.3-1-any.pkg.tar.zst"
  "mingw-w64-x86_64-libjpeg-turbo-3.1.4.1-1-any.pkg.tar.zst"
  "mingw-w64-x86_64-libmad-0.15.1b-5-any.pkg.tar.zst"
  "mingw-w64-x86_64-libogg-1.3.6-1-any.pkg.tar.zst"
  "mingw-w64-x86_64-libpng-1.6.56-1-any.pkg.tar.zst"
  "mingw-w64-x86_64-libvorbis-1.3.7-2-any.pkg.tar.zst"
  "mingw-w64-x86_64-zlib-1.3.2-2-any.pkg.tar.zst"
  # Runtime dependencies
  "mingw-w64-x86_64-gcc-libs-15.2.0-13-any.pkg.tar.zst"
  "mingw-w64-x86_64-libiconv-1.19-1-any.pkg.tar.zst"
  "mingw-w64-x86_64-brotli-1.2.0-1-any.pkg.tar.zst"
  "mingw-w64-x86_64-bzip2-1.0.8-3-any.pkg.tar.zst"
  "mingw-w64-x86_64-gettext-runtime-1.0-1-any.pkg.tar.zst"
  "mingw-w64-x86_64-libwinpthread-14.0.0.r1.gbd663f0e1-1-any.pkg.tar.zst"
  "mingw-w64-x86_64-vulkan-headers-1~1.4.341.0-1-any.pkg.tar.zst"
)

for pkg in "${PACKAGES[@]}"; do
  curl -L --max-time 60 -o "$DEST/$pkg" "$MIRROR/$pkg"
done
```

> **Note:** Check the MSYS2 database for the latest version numbers if any
> package above fails to download — versions change over time.

### 2c — Extract into the Sysroot

```sh
for pkg in ~/mingw-deps/pkgs/*.pkg.tar.zst; do
  tar --zstd -xf "$pkg" -C ~/mingw-deps/sysroot
done
```

Packages extract into a `mingw64/` subdirectory automatically. The sysroot
root is therefore `~/mingw-deps/sysroot/mingw64`.

### 2d — Fix pkg-config Prefix Paths

The `.pc` files inside the packages hardcode `/mingw64` as their prefix.
Rewrite them to point to the actual sysroot:

```sh
SYSROOT=~/mingw-deps/sysroot/mingw64
find "$SYSROOT/lib/pkgconfig" -name "*.pc" \
  -exec sed -i "s|prefix=/mingw64|prefix=$SYSROOT|g" {} \;
```

### 2e — Create a Linux sdl2-config Wrapper

The `sdl2-config` script shipped in the MSYS2 package is a Windows binary and
cannot execute on Linux. Replace it with a shell wrapper:

```sh
SYSROOT=~/mingw-deps/sysroot/mingw64

mv "$SYSROOT/bin/sdl2-config" "$SYSROOT/bin/sdl2-config.win32"

cat > "$SYSROOT/bin/sdl2-config" << EOF
#!/bin/sh
SYSROOT="$SYSROOT"
PKG_CONFIG_PATH="\$SYSROOT/lib/pkgconfig"
export PKG_CONFIG_PATH

case "\$1" in
  --version)    pkg-config --modversion sdl2 ;;
  --cflags)     pkg-config --cflags sdl2 ;;
  --libs)       pkg-config --libs sdl2 ;;
  --static-libs) pkg-config --static --libs sdl2 ;;
  --prefix)     echo "\$SYSROOT" ;;
  *)            pkg-config sdl2 "\$@" ;;
esac
EOF

chmod +x "$SYSROOT/bin/sdl2-config"
```

---

## Step 3 — Configure ScummVM

Run `configure` from the ScummVM source root. The key flag is
`--host=x86_64-w64-mingw32` which puts configure into MinGW cross-compile mode.

```sh
SYSROOT=$HOME/mingw-deps/sysroot/mingw64

PKG_CONFIG_PATH="$SYSROOT/lib/pkgconfig" \
PKG_CONFIG_LIBDIR="$SYSROOT/lib/pkgconfig" \
./configure \
  --host=x86_64-w64-mingw32 \
  --disable-all-engines \
  --enable-engine=scumm \
  --with-sdl-prefix="$SYSROOT" \
  --with-ogg-prefix="$SYSROOT" \
  --with-vorbis-prefix="$SYSROOT" \
  --with-flac-prefix="$SYSROOT" \
  --with-mad-prefix="$SYSROOT" \
  --with-png-prefix="$SYSROOT" \
  --with-jpeg-prefix="$SYSROOT" \
  --with-zlib-prefix="$SYSROOT" \
  --disable-freetype2 \
  --disable-discord \
  --disable-sparkle \
  --disable-fluidsynth \
  --disable-fluidlite \
  --disable-mikmod \
  --disable-openmpt \
  --disable-mpcdec \
  --disable-faad \
  --disable-mpeg2 \
  --disable-theoradec \
  --disable-vpx \
  --disable-a52 \
  --disable-readline \
  --disable-libcurl \
  --disable-sdlnet
```

### Why these flags?

| Flag | Reason |
|------|--------|
| `--host=x86_64-w64-mingw32` | Cross-compile target: 64-bit Windows via MinGW |
| `--disable-all-engines` | Start with no engines, then enable only what we need |
| `--enable-engine=scumm` | Enable the SCUMM engine (v0–v6 games) |
| `--with-XXX-prefix=$SYSROOT` | Point each library check at our sysroot |
| `--disable-freetype2` | FreeType2 requires HarfBuzz → GLib2 (deep dep chain); only needed for SCUMM v7/v8 |
| `--disable-discord/sparkle/...` | Optional features with no MinGW packages available |

### What configure enables

- **SDL2 2.32.10** — windowing, input, audio
- **OGG / Vorbis / FLAC / MAD** — audio codec support
- **JPEG / PNG / zlib** — image and compression support
- **OpenGL** — runtime-detected GPU rendering
- **Windows SAPI** — text-to-speech
- **Windows taskbar / dialogs / printing** — native OS integration

### What is skipped

- **SCUMM v7 & v8** — requires FreeType2 (TTF rendering)
- **FreeType2 / ImGui** — disabled (see above)
- All other engines — not needed for this build

---

## Step 4 — Build

### Standard build

```sh
make -j$(nproc)
```

This produces `scummvm.exe` in the source root.

### Build with the SCUMM HTTP API

The SCUMM API is an optional HTTP REST server built into the engine that exposes
game state and accepts commands. It is enabled at **compile time** via
`USE_SCUMM_API=1` and at **run time** via a command-line flag or INI setting.

```sh
make -j$(nproc) USE_SCUMM_API=1
```

#### How `USE_SCUMM_API` works in the build system

`USE_SCUMM_API` is a make-level variable, not a `configure` feature. Passing it
on the `make` command line does two things:

1. **Adds extra object files** — `engines/scumm/api/` and `backends/api/` are
   compiled in (controlled by `ifdef USE_SCUMM_API` in the respective
   `module.mk` files).
2. **Defines the preprocessor symbol** — `Makefile.common` appends
   `-DUSE_SCUMM_API` to `DEFINES`, which activates all `#ifdef USE_SCUMM_API`
   guards in the C++ source. This step is what was missing in earlier builds and
   caused the API to be silently absent from the binary.

It also links `-lws2_32` automatically (required by `httplib` for Winsock2 on
Windows).

> **Important:** Always run `make clean USE_SCUMM_API=1` (or delete the API
> `.o` files manually) when switching between builds with and without
> `USE_SCUMM_API=1`. Because `make clean` without the flag does not know about
> the API objects, stale objects compiled by a different toolchain variant can
> persist and cause linker errors.

#### Enabling the API at runtime

The API is off by default. Enable it when launching the game:

```
# Command line — enable on default port 9000:
scummvm.exe --scumm_api atlantis

# Command line — enable on a custom port (also enables the API):
scummvm.exe --scumm_api_port=9000 atlantis

# scummvm.ini — must be under the specific game target section:
[atlantis]
scumm_api=true
scumm_api_port=9000
```

#### API endpoints (default port 9000)

| URL | Description |
|-----|-------------|
| `http://localhost:9000/` | Test/debug page |
| `http://localhost:9000/openapi.json` | OpenAPI spec |
| `http://localhost:9000/api/state` | Full game state snapshot |
| `http://localhost:9000/api/actors` | Actor list |
| `http://localhost:9000/api/inventory` | Inventory |
| `http://localhost:9000/api/verbs` | Verb bar |
| `http://localhost:9000/api/hotspots` | Room hotspots |
| `http://localhost:9000/api/boxes` | Walkboxes |
| `http://localhost:9000/api/room` | Room metadata |
| `http://localhost:9000/api/dialog` | Active dialog state |
| `http://localhost:9000/api/screenshot` | PNG screenshot (blocking) |
| `http://localhost:9000/events` | SSE event stream |

#### API startup log messages

On a successful start you will see on stderr:

```
WARNING: SCUMM API: HTTP server thread started, binding to 0.0.0.0:9000
WARNING: SCUMM API: Server is ready at http://localhost:9000
```

If the port is already in use:

```
WARNING: SCUMM API: HTTP server thread started, binding to 0.0.0.0:9000
WARNING: SCUMM API: HTTP server thread exited
WARNING: SCUMM API: ERROR - Server failed to bind on port 9000 (port already in use?)
```

Every request is logged:

```
WARNING: SCUMM API: GET /api/state -> 200
```

---

## Step 5 — Assemble the Distribution

Copy the executable and all required runtime DLLs into a single directory:

```sh
SYSROOT=$HOME/mingw-deps/sysroot/mingw64/bin
mkdir -p built

cp scummvm.exe built/

for dll in \
  SDL2.dll \
  libFLAC.dll \
  libjpeg-8.dll \
  libmad-0.dll \
  libpng16-16.dll \
  libvorbisfile-3.dll \
  libvorbis-0.dll \
  libogg-0.dll \
  zlib1.dll \
  libgcc_s_seh-1.dll \
  libwinpthread-1.dll; do
  cp "$SYSROOT/$dll" built/
done
```

The `built/` directory is now self-contained and can be copied to any Windows
x86_64 machine. Windows system DLLs (`KERNEL32.dll`, `GDI32.dll`, etc.) are
already provided by the OS.

---

## Dependency Graph

```
scummvm.exe
├── SDL2.dll
├── libFLAC.dll
│   ├── libogg-0.dll
│   └── libwinpthread-1.dll
├── libjpeg-8.dll
│   └── libgcc_s_seh-1.dll
│       └── libwinpthread-1.dll
├── libmad-0.dll
├── libpng16-16.dll
│   └── zlib1.dll
├── libvorbisfile-3.dll
│   ├── libvorbis-0.dll
│   │   └── libogg-0.dll
│   └── libogg-0.dll
└── zlib1.dll
```

---

## Re-running configure

The generated `config.mk` records all configure flags. To reconfigure with the
same settings, either re-run the full configure command from Step 3, or inspect
`config.mk` for `SAVED_CONFIGFLAGS`.

---

## Notes

- The `~/mingw-deps/` sysroot is reusable across builds — no need to re-download.
- The `.exe` built with default configure flags includes debug symbols (~108MB
  with `USE_SCUMM_API`, ~96MB without).
  Strip for release: `x86_64-w64-mingw32-strip scummvm.exe`
- `sdl2-config.win32` in the sysroot bin is the original Windows binary, kept
  as a reference.
- When rebuilding after switching `USE_SCUMM_API` on or off, always delete the
  stale API objects explicitly:
  ```sh
  rm -f backends/api/apieventqueue.o \
        backends/api/apicommandqueue.o \
        backends/api/httpserver.o \
        engines/scumm/api/scummapi.o \
        engines/scumm/api/statebuilder.o \
        engines/scumm/api/eventinstrumentation.o
  ```

---

## ⚠️ Stale Object Files — Known Dependency Tracking Issue

**The build system does not fully track header dependencies.** If you modify a
header file (e.g. `engines/scumm/scumm.h`) and add a new field or method, `.o`
files that include that header **may not be automatically recompiled** by `make`.
This results in a binary where some object files see the old header layout and
others see the new one — causing silent bugs (wrong struct offsets, missing code
paths, etc.) that are extremely hard to diagnose.

**Rule: after modifying any `.h` file, always force-rebuild the `.o` files that
include it.** The safest approach:

```sh
# Nuclear option — rebuild everything:
make clean && USE_SCUMM_API=1 make -j$(nproc)

# Targeted option — delete specific stale .o files, then rebuild:
rm engines/scumm/string.o engines/scumm/scumm.o
USE_SCUMM_API=1 make -j$(nproc)
```

This is especially critical for changes to `engines/scumm/scumm.h`, which is
included by nearly every file in the engine. A field added behind
`#ifdef USE_SCUMM_API` will change the object layout for all translation units
that see it, but `make` won't know to recompile them.
