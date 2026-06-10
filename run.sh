#!/usr/bin/env bash
#
# run.sh — launch Heretic2R on Linux.
#
# Wires up the runtime environment the CMake build expects (see BUILDING-linux.md):
#   - cd into the build dir so the engine finds ./base and dlopen()s the engine
#     modules (quake2.so, ref_gl1.so, snd_sdl3.so) via the executable's RUNPATH.
#   - make libSDL3.so locatable (it is built separately, not installed system-wide).
#
# Usage:
#   ./run.sh [game args...]            # e.g. ./run.sh +set vid_fullscreen 0
#
# Environment overrides:
#   BUILD=build-dbg ./run.sh          # pick a different build tree (default: build)
#   SDL3_ROOT=/path/to/sdl3 ./run.sh  # SDL3 install prefix (overrides auto-detection)
#
set -euo pipefail

REPO="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"

BUILD="${BUILD:-build}"
BUILD_DIR="$REPO/$BUILD"
EXE="$BUILD_DIR/Heretic2R"

# SDL3 is built separately (see BUILDING-linux.md). Honour SDL3_ROOT if set,
# otherwise probe the usual prefixes.
SDL3_CANDIDATES=("${SDL3_ROOT:-}" "$HOME/sdl3-install" /tmp/sdl3-install /usr/local)

die() { echo "run.sh: $*" >&2; exit 1; }

[ -x "$EXE" ] || die "no executable at $EXE
  Build first:  cmake --build $BUILD -j\$(nproc)   (see BUILDING-linux.md)"

[ -d "$BUILD_DIR/base" ] || die "missing game data at $BUILD_DIR/base
  Copy a Heretic II v1.06 'base' folder there (see README.md)."

# Locate libSDL3.so.* — built separately, so it is usually not on the default
# loader path. Search the candidate prefixes, then let the loader's config decide.
sdl_libdir=""
for root in "${SDL3_CANDIDATES[@]}"; do
    [ -n "$root" ] || continue
    for d in "$root/lib" "$root/lib64"; do
        if compgen -G "$d/libSDL3.so*" > /dev/null 2>&1; then
            sdl_libdir="$d"
            break 2
        fi
    done
done

if [ -n "$sdl_libdir" ]; then
    export LD_LIBRARY_PATH="$sdl_libdir${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}"
elif ! ldconfig -p 2>/dev/null | grep -q 'libSDL3\.so'; then
    die "libSDL3.so not found under ${SDL3_CANDIDATES[*]} or on the system loader path.
  Build SDL 3.4.2 and point SDL3_ROOT at its install prefix (see BUILDING-linux.md),
  e.g.  SDL3_ROOT=/path/to/sdl3 ./run.sh"
fi

cd "$BUILD_DIR"
exec ./Heretic2R "$@"
