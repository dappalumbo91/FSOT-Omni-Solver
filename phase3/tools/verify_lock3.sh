#!/usr/bin/env bash
# Phase 3 LOCK check: sha256 of the locked tier-1 files and sources, every hash recorded in LOCK3.md, and a byte-identical
# re-derivation with the pinned engine (FSOT-2.1-Cpp 3466e8d). Usage: verify_lock3.sh <engine include dir> <golden_AEB2AD.tsv>
set -euo pipefail; cd "$(dirname "$0")/../locked"
INC=${1:-../../third_party/FSOT-2.1-Cpp/include}; GOLD=${2:-../../third_party/FSOT-2.1-Cpp/golden/golden_AEB2AD.tsv}
sha256sum -c SHA256SUMS
while read -r h f; do grep -q "$h" LOCK3.md || { echo "hash of $f not recorded in LOCK3.md"; exit 1; }; done < SHA256SUMS
g++ -std=c++20 -O2 -ffp-contract=off -I"$INC" -o ../cpp/derive3 ../cpp/derive3.cpp
tmp=$(mktemp -d); ../cpp/derive3 "$tmp" "$GOLD"
for f in design3_values.tsv design3_consts.tsv leaves3_targets.tsv; do cmp "$tmp/$f" "$f"; done
rm -rf "$tmp"; echo "LOCK3_OK"
