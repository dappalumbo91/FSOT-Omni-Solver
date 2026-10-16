#!/usr/bin/env bash
# LOCK7 Amendment 7.1 check. Usage: verify_lock7b.sh <FSOT-2.1-Cpp checkout at 3466e8d>
#  sha256 of every locked7b file and of leafc7b.cpp + harness (recorded in LOCK7b.md); byte-identical re-derivation leafmap4 -> leafc7b;
#  taps7.tsv / design7_taps_values.tsv are byte-identical copies of the LOCK7 files (netgen6 reads them from the same directory).
set -euo pipefail; cd "$(dirname "$0")/../locked7b"; ENG=${1:-../../third_party/FSOT-2.1-Cpp}
sha256sum -c --quiet SHA256SUMS
while read -r h f; do grep -q "$h" LOCK7b.md || { echo "hash of $f not recorded in LOCK7b.md"; exit 1; }; done < SHA256SUMS
F="-std=c++20 -O2 -ffp-contract=off"; g++ $F -o ../cpp/leafmap4 ../cpp/leafmap4.cpp; g++ $F -I"$ENG/include" -o ../cpp/leafc7b ../cpp/leafc7b.cpp
tmp=$(mktemp -d); ../cpp/leafmap4 "$ENG/include/fsot/closed_forms_core.gen.inc" > "$tmp/leaves_map.tsv"; cmp "$tmp/leaves_map.tsv" ../locked4/leaves_map.tsv
../cpp/leafc7b "$tmp/leaves_map.tsv" "$ENG/golden/golden_AEB2AD.tsv" "$tmp" | tail -1
for f in taps7.tsv design7_taps_values.tsv; do cmp "$f" "../locked7/$f"; done
for f in $(ls | grep -v -e SHA256SUMS -e LOCK7b.md -e taps7.tsv -e design7_taps_values.tsv); do cmp "$tmp/$f" "$f"; done
rm -rf "$tmp"; echo "LOCK7B_OK"
