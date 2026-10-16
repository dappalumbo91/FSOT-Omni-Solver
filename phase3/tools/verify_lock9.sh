#!/usr/bin/env bash
# LOCK9A/9B/9C/9H/9H.1 check (diagnostic ablation locks and the hub bench). Usage: verify_lock9.sh <FSOT-2.1-Cpp checkout at 3466e8d>
#  1. sha256 + LOCK-record of locked9a, locked9b, locked9c, locked9h, locked9h1 (each SHA256SUMS also covers its derivers and scripts).
#  2. Byte-identical re-derivation, no tolerance, every deriver linked against cpp/detmath.cpp:
#     leafc10 (no switch) -> locked9a/v71 and (flip,abs,base,early,rho) -> locked9a/all [leafc10 without "hi" is leafc9 + an inactive switch];
#     leafc10 hi -> locked9c/hi, (flip,abs,base,early,rho,hi) -> locked9c/allhi; derive9 16 -> locked9b/base_m16; derive9 256 -> locked7/base
#     (numeric columns); hubbench2 values -> locked9h1 (and hubref_cpp.tsv = locked9h's).
set -euo pipefail; cd "$(dirname "$0")/.."; ENG=${1:-../third_party/FSOT-2.1-Cpp}; case "$ENG" in /*) ;; *) ENG="$PWD/$ENG";; esac
for d in locked9a:LOCK9A.md locked9b:LOCK9B.md locked9c:LOCK9C.md locked9h:LOCK9H.md locked9h1:LOCK9H1.md; do (cd ${d%%:*}; sha256sum -c --quiet SHA256SUMS
  while read -r h f; do grep -q "$h" ${d##*:} || { echo "hash of $f not recorded in ${d##*:}"; exit 1; }; done < SHA256SUMS); done
F="-std=c++20 -O2 -ffp-contract=off"; tmp=$(mktemp -d)
g++ $F -fno-builtin -c cpp/detmath.cpp -o $tmp/detmath.o
for b in leafc10 derive9 hubbench2; do g++ $F -I"$ENG/include" -o cpp/$b cpp/$b.cpp $tmp/detmath.o
  if nm -D cpp/$b | grep -qE ' U (expl|logl|powl|pow|sinl|cosl|sincosl|acosl|log10l|atanl|atan2l|exp|log|sin|cos)@'; then echo "$b imports a libm transcendental"; exit 1; fi; done
G="$ENG/golden/golden_AEB2AD.tsv"
chk() { local s=$1 ref=$2 o=$tmp/l_$RANDOM; mkdir $o; FSOT_L9_ABL=$s cpp/leafc10 locked4/leaves_map.tsv "$G" $o | tail -1
  for f in $(ls $o); do cmp $o/$f $ref/$f; done; }
chk "" locked9a/v71; chk flip,abs,base,early,rho locked9a/all; chk hi locked9c/hi; chk flip,abs,base,early,rho,hi locked9c/allhi
mkdir $tmp/m16 $tmp/m256; cpp/derive9 $tmp/m16 16 | tail -1; for f in $(ls locked9b/base_m16); do cmp $tmp/m16/$f locked9b/base_m16/$f; done
cpp/derive9 $tmp/m256 256 | tail -1; for f in $(ls locked7/base); do cmp <(cut -f1-6 $tmp/m256/$f) <(cut -f1-6 locked7/base/$f); done
mkdir $tmp/h; cpp/hubbench2 values $tmp/h; for f in hubbench_values.tsv hubref_cpp.tsv; do cmp $tmp/h/$f locked9h1/$f; done; cmp $tmp/h/hubref_cpp.tsv locked9h/hubref_cpp.tsv
rm -rf "$tmp"; echo "LOCK9_OK"
