#!/usr/bin/env bash
# Phase 3b LOCK check: sha256 of every locked4 file and the compiler sources, every hash recorded in LOCK4.md, and a
# byte-identical re-derivation (leafmap4 -> leafc) from the pinned engine. Usage: verify_lock4.sh <FSOT-2.1-Cpp checkout at 3466e8d>
set -euo pipefail; cd "$(dirname "$0")/../locked4"; ENG=${1:-../../third_party/FSOT-2.1-Cpp}
sha256sum -c --quiet SHA256SUMS
while read -r h f; do grep -q "$h" LOCK4.md || { echo "hash of $f not recorded in LOCK4.md"; exit 1; }; done < SHA256SUMS
F="-std=c++20 -O2 -ffp-contract=off"; g++ $F -o ../cpp/leafmap4 ../cpp/leafmap4.cpp; g++ $F -I"$ENG/include" -o ../cpp/leafc ../cpp/leafc.cpp
tmp=$(mktemp -d); ../cpp/leafmap4 "$ENG/include/fsot/closed_forms_core.gen.inc" > "$tmp/leaves_map.tsv"; ../cpp/leafc "$tmp/leaves_map.tsv" "$ENG/golden/golden_AEB2AD.tsv" "$tmp" | tail -1
for f in $(ls | grep -v -e SHA256SUMS -e LOCK4.md); do cmp "$tmp/$f" "$f"; done
rm -rf "$tmp"; echo "LOCK4_OK"
