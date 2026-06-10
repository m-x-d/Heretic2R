#!/usr/bin/env bash
#
# run_win32.sh — launch Heretic2R Windows build via Wine.
#

REPO="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="$REPO/build-win32"
EXE="$BUILD_DIR/Heretic2R.exe"

if [ ! -f "$EXE" ]; then
    echo "Executable not found at $EXE"
    exit 1
fi

if [ ! -d "$BUILD_DIR/base" ]; then
    echo "Warning: missing game data at $BUILD_DIR/base"
    echo "Copy a Heretic II v1.06 'base' folder there (see README.md)."
fi

cd "$BUILD_DIR"
wine ./Heretic2R.exe "$@"
