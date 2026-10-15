#!/usr/bin/env bash
# LOCK7 check. Usage: verify_lock7.sh <FSOT-2.1-Cpp checkout at 3466e8d>
#  1. sha256 of every locked7 file (base/ included) and of the deriver/compiler/netgen/harness sources; every hash recorded in LOCK7.md.
#  2. Byte-identical re-derivation, NO tolerance: derive7 -> base/*; tapc7 (from base/folds35_targets.tsv) -> taps7.tsv, design7_taps_values.tsv;
#     leafmap4 (LOCK4) -> leafc7 -> design7_/recipe7_/leaves7_targets/generator7/netlists7.
set -euo pipefail; cd "$(dirname "$0")/../locked7"; ENG=${1:-../../third_party/FSOT-2.1-Cpp}
sha256sum -c --quiet SHA256SUMS
while read -r h f; do grep -q "$h" LOCK7.md || { echo "hash of $f not recorded in LOCK7.md"; exit 1; }; done < SHA256SUMS
F="-std=c++20 -O2 -ffp-contract=off"
g++ $F -I"$ENG/include" -o ../cpp/derive7 ../cpp/derive7.cpp; g++ $F -I"$ENG/include" -o ../cpp/tapc7 ../cpp/tapc7.cpp
g++ $F -o ../cpp/leafmap4 ../cpp/leafmap4.cpp; g++ $F -I"$ENG/include" -o ../cpp/leafc7 ../cpp/leafc7.cpp; g++ -std=c++20 -O2 -o ../cpp/netgen6 ../cpp/netgen6.cpp
tmp=$(mktemp -d); mkdir "$tmp/base"
../cpp/derive7 "$tmp/base" | tail -1; for f in $(ls base); do cmp "$tmp/base/$f" "base/$f"; done
../cpp/tapc7 base/folds35_targets.tsv "$tmp" | tail -1
../cpp/leafmap4 "$ENG/include/fsot/closed_forms_core.gen.inc" > "$tmp/leaves_map.tsv"; cmp "$tmp/leaves_map.tsv" ../locked4/leaves_map.tsv
../cpp/leafc7 "$tmp/leaves_map.tsv" "$ENG/golden/golden_AEB2AD.tsv" "$tmp" | tail -1
for f in $(ls | grep -v -e SHA256SUMS -e LOCK7.md -e '^base$'); do cmp "$tmp/$f" "$f"; done
rm -rf "$tmp"; echo "LOCK7_OK"
