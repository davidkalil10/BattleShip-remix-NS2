#!/usr/bin/env bash
# Configure and build BattleShip-Remix.nro for Nintendo Switch 2 (Ounce / libounce + OunceVK).
set -euo pipefail

REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
BUILD_DIR="${BUILD_DIR:-$REPO_ROOT/build-switch2}"
JOBS="${JOBS:-$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)}"
SSB64_VERSION="${SSB64_VERSION:-remix}"

export DEVKITPRO="${DEVKITPRO:-/opt/devkitpro}"
export PATH="$DEVKITPRO/tools/bin:$DEVKITPRO/devkitA64/bin:$PATH"

TOOLCHAIN="$DEVKITPRO/libounce/cmake/Switch2.cmake"
if [ ! -f "$TOOLCHAIN" ]; then
    echo "error: $TOOLCHAIN not found. Install ounce-devkit (libounce + ounce-tools + OunceVK) first." >&2
    exit 1
fi

if [ -f "$REPO_ROOT/patches/libultraship-switch2.patch" ]; then
    if git -C "$REPO_ROOT/libultraship" apply --check "$REPO_ROOT/patches/libultraship-switch2.patch" 2>/dev/null; then
        echo "==> Applying patches/libultraship-switch2.patch to libultraship..."
        git -C "$REPO_ROOT/libultraship" apply "$REPO_ROOT/patches/libultraship-switch2.patch"
    fi
fi

cmake -B "$BUILD_DIR" -S "$REPO_ROOT" \
    -DCMAKE_TOOLCHAIN_FILE="$TOOLCHAIN" \
    -DCMAKE_BUILD_TYPE=Release \
    -DBATTLESHIP_SWITCH=ON \
    -DDISABLE_SCRIPTING=ON \
    -DSSB64_VERSION="$SSB64_VERSION"

cmake --build "$BUILD_DIR" --target GenerateF3DO2R ssb64_nro -j "$JOBS"

NRO_NAME="BattleShip.nro"
MAP_NAME="BattleShip-switch2.map"
if [ "$SSB64_VERSION" = "jp" ]; then
    NRO_NAME="BattleShip-JP.nro"
    MAP_NAME="BattleShip-JP-switch2.map"
elif [ "$SSB64_VERSION" = "remix" ]; then
    NRO_NAME="BattleShip-Remix.nro"
    MAP_NAME="BattleShip-Remix-switch2.map"
fi

python3 "$REPO_ROOT/tools/verify_switch2_package.py" \
    --nro "$BUILD_DIR/$NRO_NAME" \
    --elf "$BUILD_DIR/ssb64.elf" \
    --map "$BUILD_DIR/$MAP_NAME"

echo "Built Switch 2 NRO: $BUILD_DIR/$NRO_NAME"
