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

# The corrections the code made on purpose in all results, with no error in the baseline to find: applying them to the
# baseline keeps the comparison sensitive to anything else. Without them, ln(10) alone gives some 300000 tiny differences.
sed -i -e 's/lg_to_ln = 2.302585093,/lg_to_ln = 2.302585092994046,/' -e 's/ln_to_lg = 0.434294481,/ln_to_lg = 0.4342944819032518,/' \
    -e 's/-67921\.9 \* cal_to_J/-67877.71 * cal_to_J/' -e 's/-67922\.22 \* cal_to_J/-67878.03 * cal_to_J/' -e 's/-55434\.49 \* cal_to_J/-55391.17 * cal_to_J/' \
    "$BASE/src/ThermoFun/GlobalVariables.h"
sed -i 's/R = 8\.31439;/R = 8.31451;/' "$BASE/src/ThermoFun/Substances/Gases/s_solmod2_.cpp"
sed -i -e 's/8\.2312e0 \* 0\.4342945 \* log/8.2312e0 * 0.4342944819032518 * log/' -e 's/pl = 2\.302585e0 + log/pl = 2.302585092994046 + log/' \
    "$BASE/src/ThermoFun/Substances/Solvent/WaterHGK-JNgems.cpp"
sed -i 's/const auto Ttr =  273\.15;/const auto Ttr =  273.16;/' "$BASE/src/ThermoFun/Substances/Solvent/WaterHGKreaktoro.cpp"
git -C "$BASE/src" diff --stat

GENERATOR=()
if command -v ninja > /dev/null; then GENERATOR=(-GNinja); fi

rm -rf "$BASE/build"
# An autodiff installed in the prefix by an earlier run has no Eigen path and breaks the build: start empty.
# Only the default prefix is removed; a prefix given by the user is never deleted.
if [ "$PREFIX" = "$BASE/install" ]; then rm -rf "$PREFIX"; fi
# shellcheck disable=SC2086
cmake -S "$BASE/src" -B "$BASE/build" "${GENERATOR[@]}" "${OPTIONS[@]}" \
    -DCMAKE_INSTALL_PREFIX="$PREFIX" -DTHERMOFUN_PYTHON_INSTALL_PREFIX="$PREFIX" ${GOLDEN_CMAKE_ARGS:-}
cmake --build "$BASE/build" -j 4
# The install runs "pip install --prefix PREFIX", which first uninstalls a thermofun found in the active
# environment. PIP_IGNORE_INSTALLED keeps the active environment unchanged.
PIP_IGNORE_INSTALLED=1 cmake --install "$BASE/build"
echo "baseline installed in $PREFIX"
