#!/usr/bin/env bash
# LOCK7 Amendment 7.2 check (the CI check for LOCK7 / 7.1 / 7.2). Usage: verify_lock7c.sh <FSOT-2.1-Cpp checkout at 3466e8d>
#  1. sha256 + LOCK-record of locked7, locked7b and locked7c (every hash recorded in LOCK7.md / LOCK7b.md / LOCK7c.md).
#  2. Byte-identical re-derivation, NO tolerance: derive7 -> locked7/base, tapc7 -> taps7 (locked7, and the copies in 7b/7c),
#     leafmap4 -> leafc7c -> locked7c leaf files.
#  3. locked7c and locked7b give identical netlists (ideal values and E24 pieces identical; they differ only in the E24_rel report column at
#     1e-19, which is what made the leafc7/leafc7b re-derivation host-dependent): checked by comparing those columns directly.
set -euo pipefail; cd "$(dirname "$0")/.."; ENG=${1:-../third_party/FSOT-2.1-Cpp}; case "$ENG" in /*) ;; *) ENG="$PWD/$ENG";; esac
for d in locked7:LOCK7.md locked7b:LOCK7b.md locked7c:LOCK7c.md; do (cd ${d%%:*}; sha256sum -c --quiet SHA256SUMS
  while read -r h f; do grep -q "$h" ${d##*:} || { echo "hash of $f not recorded in ${d##*:}"; exit 1; }; done < SHA256SUMS); done
F="-std=c++20 -O2 -ffp-contract=off"
g++ $F -I"$ENG/include" -o cpp/derive7 cpp/derive7.cpp; g++ $F -I"$ENG/include" -o cpp/tapc7 cpp/tapc7.cpp
g++ $F -o cpp/leafmap4 cpp/leafmap4.cpp; g++ $F -I"$ENG/include" -o cpp/leafc7c cpp/leafc7c.cpp; g++ -std=c++20 -O2 -o cpp/netgen6 cpp/netgen6.cpp
tmp=$(mktemp -d); mkdir "$tmp/base"
cpp/derive7 "$tmp/base" | tail -1; for f in $(ls locked7/base); do cmp "$tmp/base/$f" "locked7/base/$f"; done
cpp/tapc7 locked7/base/folds35_targets.tsv "$tmp" | tail -1; for f in taps7.tsv design7_taps_values.tsv; do cmp "$tmp/$f" locked7/$f; cmp "$tmp/$f" locked7b/$f; cmp "$tmp/$f" locked7c/$f; done
cpp/leafmap4 "$ENG/include/fsot/closed_forms_core.gen.inc" > "$tmp/leaves_map.tsv"; cmp "$tmp/leaves_map.tsv" locked4/leaves_map.tsv
cpp/leafc7c "$tmp/leaves_map.tsv" "$ENG/golden/golden_AEB2AD.tsv" "$tmp" | tail -1
for f in $(ls locked7c | grep -v -e SHA256SUMS -e LOCK7c.md -e taps7.tsv -e design7_taps_values.tsv); do cmp "$tmp/$f" "locked7c/$f"
  case $f in design7_*) cmp <(cut -f1-5,7- "locked7c/$f") <(cut -f1-5,7- "locked7b/$f");; *) cmp "locked7c/$f" "locked7b/$f";; esac; done
rm -rf "$tmp"; echo "LOCK7C_OK"
