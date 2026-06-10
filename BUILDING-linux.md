# Building Heretic2R on Linux

Heretic2R was originally a Windows/MSVC-only project. This documents the Linux port:
a CMake build that produces the same module layout as the MSVC solution, as ELF
shared objects loaded at runtime via `dlopen()`.

> Status: **compiles and links** (engine, launcher, and all game-side modules).
> Runtime has not yet been validated end-to-end and still needs Heretic II game data
> plus a real display/audio backend.

## Prerequisites

- GCC (tested with 14.2) and CMake ≥ 3.16
- OpenGL development headers (`libgl1-mesa-dev` / `libglu1-mesa-dev`)
- **SDL3 3.4.2** — must match the headers bundled in `include/SDL3/`. The distro package
  (3.2.x) is too old. Build it from source:

  ```sh
  git clone --depth 1 --branch release-3.4.2 https://github.com/libsdl-org/SDL.git
  cd SDL
  # -DSDL_X11_XTEST=OFF: this machine lacks the XTest extension dev headers.
  # Drop that flag if you have them (libxtst-dev).
  cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DSDL_SHARED=ON -DSDL_STATIC=OFF \
        -DSDL_TEST_LIBRARY=OFF -DSDL_X11_XTEST=OFF -DCMAKE_INSTALL_PREFIX=/path/to/sdl3
  cmake --build build -j$(nproc)
  cmake --install build
  ```

## Build

```sh
cmake -S . -B build-linux -DCMAKE_BUILD_TYPE=Release -DSDL3_ROOT=/path/to/sdl3
cmake --build build-linux -j$(nproc)
```

Outputs (matching the MSVC layout):

- `build/` — `Heretic2R` (executable), `quake2.so`, `H2Common.so`, `ref_gl1.so`, `snd_sdl3.so`
- `build/base/` — `gamex86.so`, `Player.so`, `Client Effects.so`

`SDL3_ROOT` defaults to `/tmp/sdl3-install`; pass `-DSDL3_ROOT=` to point at your SDL3 prefix.

## How the port works

All Linux-specific code lives under `src/linux/`:

- **`linux_compat.h`** — force-included (via `-include`) before every translation unit.
  Provides the MSVC "secure CRT" functions (`strcpy_s`, `sprintf_s`, `fopen_s`,
  `memcpy_s`, …), `min`/`max`, the Win32 types/APIs the engine references outside its
  platform layer (`HINSTANCE`, `DWORD`, `LoadLibrary`→`dlopen`, `MessageBox`→stderr, …),
  and maps MSVC keywords (`__declspec`, `__forceinline`, `_stricmp`, …).
- **`compat/windows.h`, `compat/direct.h`, `compat/io.h`** — stub headers (first on the
  include path) so the handful of cross-platform files that `#include <windows.h>` compile
  unchanged.
- **`sys_unix.c`** — system layer + main loop + `dlopen`-based DLL loading (replaces
  `win32/sys_win.c`).
- **`q_shunix.c`** — filesystem/time helpers via POSIX + `glob()` (replaces `win32/q_shwin.c`).
- **`net_udp.c`** — BSD-sockets UDP networking (replaces `win32/net_wins.c`); IPX is dropped.
- **`launcher_main.c`** — `main()` entry replacing the Win32 `WinMain`.

`win32/sys_win.c`, `win32/q_shwin.c`, and `win32/net_wins.c` are excluded from the Linux
build; `win32/vid_Screenshot.c` and `win32/dll_io/*.c` are portable and kept.

### Source changes for GCC

The original code is clean under MSVC but trips several GCC/Linux differences (all small,
behaviour-preserving, and guarded for Windows where applicable):

- **Case-sensitive includes** — Linux is case-sensitive; Windows is not. A number of
  `#include`s spell header names with different casing than the file on disk
  (e.g. `q_shared.h` → `q_Shared.h`, `Monsters/PlaguesSithra` → `PlagueSsithra`).
  Case-correct symlinks beside the real headers fix this. They are not committed
  (git symlinks misbehave on Windows), so after a fresh clone run once:
  `python3 src/linux/gen-case-symlinks.py`
- `Hunk.c` — `VirtualAlloc`/`VirtualFree` → `mmap`/`munmap` (`#ifdef _WIN32`).
- `DllMain.c` — Windows `DllMain` → ELF `__attribute__((constructor/destructor))`.
- `g_ClassStatics.h` — `enum ClassID_e` defined before its includes (GCC rejects an
  incomplete enum as a struct field; the include cycle made it incomplete).
- `g_Obj.h`, `ce_*.h`, `gl1_*.h` — file-scope forward declarations so prototype parameters
  refer to the real struct tags (C tag-scoping rules; MSVC is lax).
- `q_Shared.c` — `BoxOnPlaneSide` and friends are compiled into several modules, mirroring
  the MSVC solution.
- `qcommon.h` — `BUILDSTRING`/`CPUSTRING` defined for non-Windows.
- `SurfaceProps.c` — definition matched to its `const void*` prototype.
- `gl1_FlexModel.c` — a stray Latin-1 (`0xF1`) identifier byte replaced with ASCII.
- `p_items.c` — cosmetic `#pragma region` markers (illegal mid-initializer in GCC) removed.

### Compiler flags of note

`-fcommon` (legacy tentative globals), `-fms-extensions` (forward-declared enums, lax tag
scoping), and `-Wno-error=` for the constructs GCC 14 promotes to hard errors
(`incompatible-pointer-types`, `implicit-function-declaration`, `int-conversion`).

## Known limitations / next steps

- 64-bit build (the original is 32-bit x86). Struct layouts are consistent across all
  modules, but pointer-size assumptions in save/network code are unverified at runtime.
- IPX networking removed (dead protocol).
- Runtime not yet validated: needs Heretic II v1.06 game data and an X11/Wayland display.
- The launcher's RPATH currently points at the build/SDL prefixes; an installed layout
  would want `$ORIGIN`-relative RPATHs.
