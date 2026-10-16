#!/usr/bin/env bash
# LOCK8 check (leaves, part C). Usage: verify_lock8.sh <FSOT-2.1-Cpp checkout at 3466e8d>
#  1. sha256 + LOCK-record of locked8 (every hash recorded in LOCK8.md).
#  2. Byte-identical re-derivation, NO tolerance: leafmap4 -> leafc8 -> locked8 leaf files; tapc7 on locked7/base -> the taps copies in locked8.
#  3. Every LOCK8 design/recipe/generator file of the 12 LOCK7c netlists is byte-identical to its locked7c file (LOCK8 only adds C_S and C_H),
#     and leaves8_targets.tsv equals leaves7_targets.tsv on every row except the 13 domain_scalar rows (now compiled) and the 16 appended partC rows.
#  4. netgen7 builds C_S and C_H (ideal, 27 C) from locked8.
set -euo pipefail; cd "$(dirname "$0")/.."; ENG=${1:-../third_party/FSOT-2.1-Cpp}; case "$ENG" in /*) ;; *) ENG="$PWD/$ENG";; esac
(cd locked8; sha256sum -c --quiet SHA256SUMS; while read -r h f; do grep -q "$h" LOCK8.md || { echo "hash of $f not recorded in LOCK8.md"; exit 1; }; done < SHA256SUMS)
F="-std=c++20 -O2 -ffp-contract=off"
g++ $F -I"$ENG/include" -o cpp/tapc7 cpp/tapc7.cpp; g++ $F -o cpp/leafmap4 cpp/leafmap4.cpp; g++ $F -I"$ENG/include" -o cpp/leafc8 cpp/leafc8.cpp
g++ -std=c++20 -O2 -o cpp/netgen7 cpp/netgen7.cpp
tmp=$(mktemp -d)
cpp/tapc7 locked7/base/folds35_targets.tsv "$tmp" | tail -1; for f in taps7.tsv design7_taps_values.tsv; do cmp "$tmp/$f" locked8/$f; done
cpp/leafmap4 "$ENG/include/fsot/closed_forms_core.gen.inc" > "$tmp/leaves_map.tsv"; cmp "$tmp/leaves_map.tsv" locked4/leaves_map.tsv
mkdir "$tmp/l8"; cpp/leafc8 "$tmp/leaves_map.tsv" "$ENG/golden/golden_AEB2AD.tsv" "$tmp/l8" | tail -1
for f in $(ls "$tmp/l8"); do cmp "$tmp/l8/$f" "locked8/$f"; done
for f in $(ls locked8 | grep -e '^design8_' -e '^recipe8_' -e '^generator8' | grep -v -e '_C_S' -e '_C_H'); do cmp "locked8/$f" "locked7c/${f/8/7}"; done
d=$(diff <(cut -f1 locked7c/leaves7_targets.tsv) <(cut -f1 locked8/leaves8_targets.tsv) | grep -c '^>' || true); [ "$d" = 16 ] || { echo "expected 16 appended rows, got $d"; exit 1; }
c=$(diff locked7c/leaves7_targets.tsv locked8/leaves8_targets.tsv | grep '^<' | grep -c 'domain_scalar' || true); a=$(diff locked7c/leaves7_targets.tsv locked8/leaves8_targets.tsv | grep -c '^<' || true)
[ "$c" = 13 ] && [ "$a" = 13 ] || { echo "leaves8_targets: $a changed rows, $c domain_scalar (expected 13/13)"; exit 1; }
cd ../phase2/spice; for k in C_S C_H; do FSOT_P7_LOCKED=../../phase3/locked8 FSOT_P8_LOCKED=../../phase3/locked8 ../../phase3/cpp/netgen7 leafc8 ideal_ideal 0 "$tmp/$k.cir" 27 op $k; grep -q . "$tmp/$k.cir"; done
rm -rf "$tmp"; echo "LOCK8_OK"
