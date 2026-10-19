#!/usr/bin/env bash
#
# Run the final monolithic phase-space productions sequentially.
# Start from the repository root or from this directory:
#   ./macros/final_runs/run_final_productions.sh
#
# Override THREADS, SEED, MIN_FREE_GB, and RUNS when needed.

set -Eeuo pipefail

SCRIPT_DIR=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)
REPO=$(cd -- "$SCRIPT_DIR/../.." && pwd)
BUILD="$REPO/build"
PHSP="$REPO/phsp"
FINAL_ROOT="$PHSP/final_runs"
EXE="$BUILD/phantoms_sim"

THREADS=${THREADS:-32}
SEED=${SEED:-20261016}
MIN_FREE_GB=${MIN_FREE_GB:-40}
RUNS=${RUNS:-\
final_Cylinder_NeuCure.mac \
final_Cylinder_MIT.mac \
final_Sphere_NeuCure.mac \
final_Sphere_MIT.mac \
final_Snyder1_NeuCure.mac \
final_Snyder1_MIT.mac \
final_Snyder2_NeuCure.mac \
final_Snyder2_MIT.mac}

[[ -x "$EXE" ]] || { echo "ERROR: executable not found: $EXE" >&2; exit 2; }
mkdir -p "$PHSP"
mkdir -p "$FINAL_ROOT"

free_kb=$(df -Pk "$PHSP" | awk 'NR==2 {print $4}')
required_kb=$((MIN_FREE_GB * 1024 * 1024))
if (( free_kb < required_kb )); then
    echo "ERROR: only $((free_kb / 1024 / 1024)) GiB free in $PHSP; need ${MIN_FREE_GB} GiB." >&2
    exit 2
fi

for macro in $RUNS; do
    macro_path="$SCRIPT_DIR/$macro"
    [[ -f "$macro_path" ]] || { echo "ERROR: missing macro: $macro_path" >&2; exit 2; }

    prefix=$(awk '$1 == "/action/IAEAphspWriter/namePrefix" {print $2; exit}' "$macro_path")
    boundary=$(awk '$1 == "/action/IAEAphspWriter/scoringBoundary" {print $2; exit}' "$macro_path")
    [[ -n "$prefix" && -n "$boundary" ]] || {
        echo "ERROR: cannot determine output from $macro_path" >&2
        exit 2
    }
    tag=${boundary//->/_to_}
    base="$BUILD/${prefix}_${tag}"
    experiment=${macro%.mac}
    output_dir="$FINAL_ROOT/$experiment"
    [[ ! -e "$output_dir" ]] || {
        echo "ERROR: output directory already exists; refusing to overwrite: $output_dir" >&2
        exit 2
    }
    mesh_files=()
    while IFS= read -r mesh_file; do
        [[ -n "$mesh_file" ]] && mesh_files+=("$mesh_file")
    done < <(awk '$1 == "/score/dumpQuantityToFile" {print $4}' "$macro_path")
    [[ ${#mesh_files[@]} -gt 0 ]] || {
        echo "ERROR: no scoring-mesh CSV outputs found in $macro_path" >&2
        exit 2
    }
    for suffix in IAEAphsp IAEAheader; do
        if [[ -e "$base.$suffix" ]]; then
            echo "ERROR: output already exists for $macro; refusing to overwrite." >&2
            echo "       expected base: $base" >&2
            exit 2
        fi
    done
    for mesh_file in "${mesh_files[@]}"; do
        [[ ! -e "$BUILD/$mesh_file" ]] || {
            echo "ERROR: scoring-mesh output already exists: $BUILD/$mesh_file" >&2
            exit 2
        }
    done

    mkdir "$output_dir"
    log="$output_dir/run.log"
    echo "[$(date --iso-8601=seconds)] starting $macro with $THREADS threads, seed $SEED"
    (
        cd "$BUILD"
        "$EXE" "$macro_path" "$THREADS" "$SEED"
    ) 2>&1 | tee "$log"

    for suffix in IAEAphsp IAEAheader; do
        [[ -s "$base.$suffix" ]] || {
            echo "ERROR: expected output missing or empty: $base.$suffix" >&2
            exit 1
        }
    done
    for mesh_file in "${mesh_files[@]}"; do
        [[ -s "$BUILD/$mesh_file" ]] || {
            echo "ERROR: scoring-mesh output missing or empty: $BUILD/$mesh_file" >&2
            exit 1
        }
    done

    mv "$base.IAEAphsp" "$output_dir/"
    mv "$base.IAEAheader" "$output_dir/"
    for mesh_file in "${mesh_files[@]}"; do
        mv "$BUILD/$mesh_file" "$output_dir/"
    done
    echo "[$(date --iso-8601=seconds)] completed $macro"
    SEED=$((SEED + 1))
done
