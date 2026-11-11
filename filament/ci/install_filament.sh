#!/bin/bash
# Builds the Filament release the filament plugin needs (see
# filament/README.md) and installs it into <prefix>. Used by
# .github/workflows/filament.yml; runs the same way locally.
#
# Usage: install_filament.sh <prefix> [<work dir>]
# The work dir (default: a temporary one) holds the source and build trees.
set -euo pipefail

FILAMENT_COMMIT=0643fd0f4363b85295fcaccd067a042072e483dd
# Optional filament/patches applied on top, in order. One line per patch.
# 0004 is kept so CI covers the gz light semantics path.
FILAMENT_PATCHES=(
  0004-light-specular-scale-and-polynomial-attenuation.patch
)

prefix=$(realpath -m "$1")
work=$(realpath -m "${2:-$(mktemp -d)}")
patches=$(cd "$(dirname "$0")/../patches" && pwd)

mkdir -p "$work/src"
cd "$work/src"
git init -q
git fetch -q --depth 1 https://github.com/google/filament.git "$FILAMENT_COMMIT"
git checkout -q --force FETCH_HEAD
for p in "${FILAMENT_PATCHES[@]}"; do
  echo "Applying $p"
  git apply "$patches/$p"
done

cmake -S . -B "$work/build" -G Ninja \
  -DCMAKE_BUILD_TYPE=MinSizeRel \
  -DCMAKE_C_FLAGS_MINSIZEREL="-O3 -DNDEBUG" \
  -DCMAKE_CXX_FLAGS_MINSIZEREL="-O3 -DNDEBUG" \
  -DCMAKE_INSTALL_PREFIX="$prefix" \
  -DFILAMENT_ENABLE_EXPERIMENTAL_GCC_SUPPORT=ON \
  -DMATC_OPT_FLAGS=--platform=all \
  -DFILAMENT_SUPPORTS_EGL_ON_LINUX=ON \
  -DFILAMENT_SUPPORTS_VULKAN=ON \
  -DFILAMENT_BUILD_FILAMAT=ON \
  -DFILAMENT_SKIP_SAMPLES=ON \
  -DFILAMENT_BUILD_TESTING=OFF \
  -DFILAMENT_SKIP_SDL2=ON \
  -DCMAKE_CXX_STANDARD_LIBRARIES=-lGL \
  -DUSE_STATIC_LIBCXX=ON
cmake --build "$work/build" --target install
