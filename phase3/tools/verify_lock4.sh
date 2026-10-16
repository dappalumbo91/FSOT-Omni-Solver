#!/usr/bin/env bash
# Phase 3b LOCK check (LOCK4 + Amendment 4.5 step locked4b). Usage: verify_lock4.sh <FSOT-2.1-Cpp checkout at 3466e8d>
#  1. sha256 of every locked4 and locked4b file and the compiler/harness sources; every hash recorded in LOCK4.md.
#  2. Byte-identical re-derivation, NO tolerance: leafmap4 -> leafc4b (fork of leafc, engine column at 18 significant digits).
#     recipes, values, netlists4 and leaves_map are compared with locked4. leaves4_targets.tsv and generator4.tsv are compared with
#     locked4b, which supersedes the 21-digit locked4 copies; those stay hash-pinned as the historical record.
set -euo pipefail; cd "$(dirname "$0")/../locked4"; ENG=${1:-../../third_party/FSOT-2.1-Cpp}
sha256sum -c --quiet SHA256SUMS; (cd ../locked4b && sha256sum -c --quiet SHA256SUMS)
while read -r h f; do grep -q "$h" LOCK4.md || { echo "hash of $f not recorded in LOCK4.md"; exit 1; }; done < <(cat SHA256SUMS ../locked4b/SHA256SUMS)
F="-std=c++20 -O2 -ffp-contract=off"; g++ $F -o ../cpp/leafmap4 ../cpp/leafmap4.cpp; g++ $F -I"$ENG/include" -o ../cpp/leafc4b ../cpp/leafc4b.cpp
tmp=$(mktemp -d); ../cpp/leafmap4 "$ENG/include/fsot/closed_forms_core.gen.inc" > "$tmp/leaves_map.tsv"; ../cpp/leafc4b "$tmp/leaves_map.tsv" "$ENG/golden/golden_AEB2AD.tsv" "$tmp" | tail -1
for f in $(ls | grep -v -e SHA256SUMS -e LOCK4.md -e leaves4_targets.tsv -e generator4.tsv); do cmp "$tmp/$f" "$f"; done
for f in leaves4_targets.tsv generator4.tsv; do cmp "$tmp/$f" "../locked4b/$f"; done
rm -rf "$tmp"; echo "LOCK4_OK"
