#!/usr/bin/env bash
# Phase 3d LOCK check (LOCK6). Usage: verify_lock6.sh <FSOT-2.1-Cpp checkout at 3466e8d>
#  1. sha256 of every locked6 file and of the deriver/netgen/harness sources; every hash recorded in LOCK6.md.
#  2. Byte-identical re-derivation, NO tolerance: tapc6 from the locked phase2/locked/folds35_targets.tsv -> taps6.tsv, design6_taps_values.tsv.
set -euo pipefail; cd "$(dirname "$0")/../locked6"; ENG=${1:-../../third_party/FSOT-2.1-Cpp}
sha256sum -c --quiet SHA256SUMS
while read -r h f; do grep -q "$h" LOCK6.md || { echo "hash of $f not recorded in LOCK6.md"; exit 1; }; done < SHA256SUMS
g++ -std=c++20 -O2 -ffp-contract=off -I"$ENG/include" -o ../cpp/tapc6 ../cpp/tapc6.cpp; g++ -std=c++20 -O2 -o ../cpp/netgen5 ../cpp/netgen5.cpp
tmp=$(mktemp -d); ../cpp/tapc6 ../../phase2/locked/folds35_targets.tsv "$tmp" | tail -1
for f in taps6.tsv design6_taps_values.tsv; do cmp "$tmp/$f" "$f"; done
rm -rf "$tmp"; echo "LOCK6_OK"
