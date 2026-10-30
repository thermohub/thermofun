#!/usr/bin/env bash
# SPDX-License-Identifier: LGPL-3.0-or-later
# Copyright (C) 2026 ThermoFun contributors
#
# Build and install the baseline of the golden comparison: the commit with the tag golden-baseline.
#
#   bash pytests/golden/build_baseline.sh [TAG] [PREFIX]
#   bash pytests/golden/build_baseline.sh --options     # print the build options and stop
#
# TAG     default golden-baseline
# PREFIX  default .golden-baseline/install (the library and the Python package go there)
# GOLDEN_CMAKE_ARGS  more CMake arguments, for example a compiler
#
# The script uses the active Python and the active conda environment, as the CI build does.
set -euo pipefail

OPTIONS=(-DCMAKE_BUILD_TYPE=Release -DTFUN_BUILD_TESTS=OFF -DTFUN_BUILD_PYTHON=ON)

if [ "${1:-}" = "--options" ]; then
    printf '%s\n' "${OPTIONS[@]}"
    echo "GOLDEN_CMAKE_ARGS=${GOLDEN_CMAKE_ARGS:-}"
    exit 0
fi

TAG=${1:-golden-baseline}
ROOT=$(git rev-parse --show-toplevel)
BASE="$ROOT/.golden-baseline"
PREFIX=${2:-$BASE/install}
case "$PREFIX" in /*) ;; *) PREFIX="$PWD/$PREFIX" ;; esac

mkdir -p "$BASE"
bash "$0" --options > "$BASE/options.txt"

if [ -d "$BASE/src" ]; then
    git -C "$ROOT" worktree remove --force "$BASE/src"
fi
git -C "$ROOT" worktree add --detach "$BASE/src" "$TAG"
echo "baseline: $TAG = $(git -C "$BASE/src" rev-parse HEAD)"

GENERATOR=()
if command -v ninja > /dev/null; then GENERATOR=(-GNinja); fi

rm -rf "$BASE/build"
# An autodiff installed in the prefix by an earlier run has no Eigen path and breaks the build: start empty.
# Only the default prefix is removed; a prefix given by the user is never deleted.
if [ "$PREFIX" = "$BASE/install" ]; then rm -rf "$PREFIX"; fi
# shellcheck disable=SC2086
cmake -S "$BASE/src" -B "$BASE/build" "${GENERATOR[@]}" "${OPTIONS[@]}" \
    -DCMAKE_INSTALL_PREFIX="$PREFIX" -DTHERMOFUN_PYTHON_INSTALL_PREFIX="$PREFIX" ${GOLDEN_CMAKE_ARGS:-}
cmake --build "$BASE/build" -j "$(nproc 2> /dev/null || echo 4)"
# The install runs "pip install --prefix PREFIX", which first uninstalls a thermofun found in the active
# environment. PIP_IGNORE_INSTALLED keeps the active environment unchanged.
PIP_IGNORE_INSTALLED=1 cmake --install "$BASE/build"
echo "baseline installed in $PREFIX"
