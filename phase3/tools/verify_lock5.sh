#!/usr/bin/env bash
# Phase 3c LOCK check (LOCK5). Usage: verify_lock5.sh <FSOT-2.1-Cpp checkout at 3466e8d>
#  1. sha256 of every locked5 file and of the compiler/netgen/harness sources; every hash recorded in LOCK5.md.
#  2. Byte-identical re-derivation, NO tolerance: leafmap4 (LOCK4) -> leafc5 -> design5_/recipe5_/leaves5_targets/generator5/netlists5,
#     and netgen4 taps -> taps5.tsv (base-engine tiny-tap decisions from the locked phase2 folds35_targets).
set -euo pipefail; cd "$(dirname "$0")/../locked5"; ENG=${1:-../../third_party/FSOT-2.1-Cpp}
sha256sum -c --quiet SHA256SUMS
while read -r h f; do grep -q "$h" LOCK5.md || { echo "hash of $f not recorded in LOCK5.md"; exit 1; }; done < SHA256SUMS
F="-std=c++20 -O2 -ffp-contract=off"; g++ $F -o ../cpp/leafmap4 ../cpp/leafmap4.cpp; g++ $F -I"$ENG/include" -o ../cpp/leafc5 ../cpp/leafc5.cpp; g++ -std=c++20 -O2 -o ../cpp/netgen4 ../cpp/netgen4.cpp
tmp=$(mktemp -d); ../cpp/leafmap4 "$ENG/include/fsot/closed_forms_core.gen.inc" > "$tmp/leaves_map.tsv"; cmp "$tmp/leaves_map.tsv" ../locked4/leaves_map.tsv
../cpp/leafc5 "$tmp/leaves_map.tsv" "$ENG/golden/golden_AEB2AD.tsv" "$tmp" | tail -1
(cd ../../phase2/spice && ../../phase3/cpp/netgen4 taps ideal_ideal 0 "$tmp/taps5.tsv")
for f in $(ls | grep -v -e SHA256SUMS -e LOCK5.md); do cmp "$tmp/$f" "$f"; done
rm -rf "$tmp"; echo "LOCK5_OK"
