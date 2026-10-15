#!/usr/bin/env bash
# LOCK8 Amendment 8.1 check — the CI check for LOCK7, 7.1, 7.2, LOCK8 and 8.1. Usage: verify_lock8b.sh <FSOT-2.1-Cpp checkout at 3466e8d>
#  1. sha256 + LOCK-record of locked7, locked7b, locked7c, locked8 and locked8b (every hash recorded in its LOCK*.md).
#  2. Byte-identical re-derivation, NO tolerance, with every deriver linked against cpp/detmath.cpp (host-independent transcendentals):
#     derive7 -> locked7/base, tapc7 -> taps7 (locked7 and the copies in 7b/7c/8/8b), leafmap4 -> leafc8b -> locked8b.
#  3. locked8b vs locked7c (the netlists results/leaves7b_* were simulated with) and vs locked8: recipes identical; design files identical in the
#     ideal and E24 columns except the 7 parts listed in LOCK8b.md (E24 piece count only), which are checked by name.
#  Retired from CI (disclosed in LOCK8b.md): the byte re-derivation of locked7c / locked7b / locked7 leaf files and of locked8 (their derivers use
#  libm's x87 transcendentals; they re-derive byte-identically on Intel hosts, including an Ubuntu 24.04 / glibc 2.39 / g++ 13.3 chroot, but not
#  on every GitHub runner CPU). Their hashes are still checked in step 1.
set -euo pipefail; cd "$(dirname "$0")/.."; ENG=${1:-../third_party/FSOT-2.1-Cpp}; case "$ENG" in /*) ;; *) ENG="$PWD/$ENG";; esac
for d in locked7:LOCK7.md locked7b:LOCK7b.md locked7c:LOCK7c.md locked8:LOCK8.md locked8b:LOCK8b.md; do (cd ${d%%:*}; sha256sum -c --quiet SHA256SUMS
  while read -r h f; do grep -q "$h" ${d##*:} || { echo "hash of $f not recorded in ${d##*:}"; exit 1; }; done < SHA256SUMS); done
F="-std=c++20 -O2 -ffp-contract=off"; tmp=$(mktemp -d)
g++ $F -fno-builtin -c cpp/detmath.cpp -o $tmp/detmath.o
g++ $F -I"$ENG/include" -o cpp/derive7 cpp/derive7.cpp $tmp/detmath.o; g++ $F -I"$ENG/include" -o cpp/tapc7 cpp/tapc7.cpp $tmp/detmath.o
g++ $F -o cpp/leafmap4 cpp/leafmap4.cpp; g++ $F -I"$ENG/include" -o cpp/leafc8b cpp/leafc8b.cpp $tmp/detmath.o; g++ -std=c++20 -O2 -o cpp/netgen7 cpp/netgen7.cpp
for b in derive7 tapc7 leafc8b; do if nm -D cpp/$b | grep -qE ' U (expl|logl|powl|pow|sinl|cosl|sincosl|acosl|log10l|atanl|atan2l|exp|log|sin|cos)@'; then echo "$b imports a libm transcendental"; exit 1; fi; done
mkdir "$tmp/base"; cpp/derive7 "$tmp/base" | tail -1; for f in $(ls locked7/base); do cmp "$tmp/base/$f" "locked7/base/$f"; done
cpp/tapc7 locked7/base/folds35_targets.tsv "$tmp" | tail -1; for f in taps7.tsv design7_taps_values.tsv; do for d in locked7 locked7b locked7c locked8 locked8b; do cmp "$tmp/$f" $d/$f; done; done
cpp/leafmap4 "$ENG/include/fsot/closed_forms_core.gen.inc" > "$tmp/leaves_map.tsv"; cmp "$tmp/leaves_map.tsv" locked4/leaves_map.tsv
mkdir "$tmp/l8b"; cpp/leafc8b "$tmp/leaves_map.tsv" "$ENG/golden/golden_AEB2AD.tsv" "$tmp/l8b" | tail -1
for f in $(ls "$tmp/l8b"); do cmp "$tmp/l8b/$f" "locked8b/$f"; done
CH="design8_2_M_values.tsv:P_zS159xc design8_2_P_values.tsv:P_zS152xc design8_2a_X_values.tsv:P_zS13xi1 design8_GEN_values.tsv:P_zS123xc design8_GENpp_values.tsv:P_zS13xc design8_T1_values.tsv:P_zS96xc design8_T1_values.tsv:P_zS113xc"
for f in $(ls locked8b | grep -e '^recipe8_'); do cmp "locked8b/$f" "locked8/$f"; g=locked7c/${f/8/7}; [ -f $g ] && cmp "locked8b/$f" $g; done
for f in $(ls locked8b | grep '^design8_'); do
  got=$(diff <(cut -f1-5 locked8/$f) <(cut -f1-5 locked8b/$f) | grep '^>' | cut -f1 | sed 's/^> //' | sort | tr '\n' ' ' || true)
  want=$(for c in $CH; do [ "${c%%:*}" = "$f" ] && echo "${c##*:}"; done | sort | tr '\n' ' ' || true)
  [ "$got" = "$want" ] || { echo "$f: changed parts [$got], expected [$want]"; exit 1; }
  g=locked7c/${f/8/7}; [ -f $g ] && { cmp <(cut -f1-5 locked8/$f) <(cut -f1-5 $g); }; done
w=$(for c in $CH; do grep "^${c##*:}	" "locked8b/${c%%:*}" | cut -f3-5; done | sort -u | tr '\t\n' ', '); [ "$w" = "6200,470,0 62000,4700,0 " ] || { echo "8.1 parts not 62000+4700 / 6200+470: $w"; exit 1; }
rm -rf "$tmp"; echo "LOCK8B_OK"
