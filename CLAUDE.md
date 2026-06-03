# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## What this is

Heretic2R is a reverse-engineered source port of **Heretic II** (1998, Raven Software), which itself
is built on the **Quake II engine**. The codebase is C (compiled as C++ in places via MSVC), Win32-only,
and built for **x86 / Win32** (not x64).

The game needs Heretic II v1.06 game data to run (see README.md). It is not asset- or save-compatible
with the original Heretic II, and its renderer/sound/game DLLs use changed APIs incompatible with H2's.

## Building

There is no CMake/Makefile — this is a **Visual Studio solution**. Open `src/Heretic2R.sln` in Visual
Studio and build. Configurations are `Debug|x86` and `Release|x86` only.

Build outputs land in:
- `build/` — `Heretic2R.exe`, `quake2.dll`, `H2Common.dll`, `ref_gl1.dll`, `snd_sdl3.dll`
- `build/base/` — `gamex86.dll`, `Player.dll`, `Client Effects.dll` (the moddable game-side DLLs)

`pack_release.bat` zips a release into `release/`, deriving the revision from `VERSION_REVISION` in
`src/qcommon/Version.h`. Bump `VERSION_REVISION` there when cutting a release.

There are no tests and no lint configuration in this repo.

## Module / DLL architecture

The engine is split into separate DLLs that talk to each other through **import/export struct tables**
(`*_import_t` provided by the host, `*_export_t` returned by the loaded DLL). DLLs are loaded at runtime
via `Sys_LoadGameDll` + `GetProcAddress` on a well-known entry point, so they are hot-swappable. The
project (`src/<name>/`) → output mapping:

| Project (`src/…`)        | Output                | Role |
|--------------------------|-----------------------|------|
| `launcher/` (Heretic2R)  | `Heretic2R.exe`       | Thin `WinMain` wrapper; statically links `quake2.lib` and calls `Quake2Main`. |
| `quake2/`                | `quake2.dll`          | The engine: client, server, Win32 platform layer, and shared engine code (`cs_shared/`). |
| `H2Common/`              | `H2Common.dll`        | Shared low-level utilities (math, matrices, `ResourceManager`, linked lists, byte order, physics, surface props). Linked by **every** other module. |
| `game/` (gamex86)        | `base/gamex86.dll`    | Server-side game logic (entities, monsters, spells, physics, scripting, saving). Entry point `GetGameAPI`. |
| `Player/`                | `base/Player.dll`     | Player animation/action/state machine. Loaded by `gamex86.dll`, not the engine. Interface in `qcommon/p_dll.*`. |
| `client effects/`        | `base/Client Effects.dll` | Client-side particle/special-effects system. Loaded by the engine (`win32/dll_io/clfx_dll.*`). |
| `ref_gl1/`               | `ref_gl1.dll`         | OpenGL 1.3 renderer (glad). Loaded via `win32/dll_io/vid_dll.*`. |
| `snd_sdl3/`              | `snd_sdl3.dll`        | SDL3 sound backend (OGG/WAV, lowpass filter). Loaded via `win32/dll_io/snd_dll.*`. Selected by `snd_dll` cvar. |

Loading order at a glance: `Heretic2R.exe` → `quake2.dll` (engine), which loads the renderer, sound, and
client-effects DLLs; the engine's server loads `gamex86.dll`, which in turn loads `Player.dll`.

### Shared headers — `src/qcommon/`

`src/qcommon/` is the cross-module header hub (the Quake `q_shared` equivalent): vector/matrix/angle math,
effect/particle flags, message (netmsg) serialization, skeletons, references, the Player and Physics
interfaces, and `Version.h`. It is on the include path of essentially every project. When changing a
struct or constant used across the DLL boundary, both sides must be rebuilt or the API checksum/layout
will mismatch.

`src/quake2/src/cs_shared/` holds engine code shared between client and server (cmd, cvar, cmodel,
`pmove`, files, netchan). Third-party libraries live in top-level `include/` (glad GL1.3, libsmacker for
Smacker cinematics, SDL3, stb) with SDL3 import libs in `lib/SDL3/`.

## Conventions

- **`//mxd` markers**: the maintainer tags modern changes, fixes, and additions to the original Raven
  code with a `//mxd` comment (thousands of occurrences across the tree). Untagged code is generally the
  original reverse-engineered 1998 logic. When editing, follow this convention — annotate behavioral
  changes/additions with `//mxd` (and a short rationale where the original behavior differed, as the
  existing comments do). Comments often note "original logic did X" to document intentional deviations.
- Code is organized by subsystem folders within each project (e.g. `game/src/Monsters`, `SpellsOffensive`,
  `Scripting`, `Saving`, `NavSys`, `Physics`, `GameObjects`).
- Player/config/save data is stored at runtime under `%USERPROFILE%\Saved Games\Heretic2R`, not the game
  folder.
