#!/usr/bin/env bash
# check_hostindep8b.sh <FSOT-2.1-Cpp checkout at 3466e8d>: LOCK8 Amendment 8.1 host-independence check (local, before pushing; not a CI step).
# Re-derives locked8b with leafc8b linked against detmath (1) perturbed by +1 ulp, (2) by -1 ulp on every expl/logl/sinl/cosl result
# (emulating another CPU's transcendental unit), (3) at -O0 and (4) at -O3 -march=native; every output must equal locked8b byte for byte.
set -euo pipefail; cd "$(dirname "$0")/.."; ENG=${1:-../third_party/FSOT-2.1-Cpp}; case "$ENG" in /*) ;; *) ENG="$PWD/$ENG";; esac
tmp=$(mktemp -d); rc=0
run() { local tag=$1 opt=$2 def=$3; g++ -std=c++20 $opt -ffp-contract=off -fno-builtin $def -c cpp/detmath.cpp -o $tmp/dm.o
  g++ -std=c++20 $opt -ffp-contract=off -I"$ENG/include" -o $tmp/lc cpp/leafc8b.cpp $tmp/dm.o; rm -rf $tmp/o; mkdir $tmp/o
  $tmp/lc locked4/leaves_map.tsv "$ENG/golden/golden_AEB2AD.tsv" $tmp/o >/dev/null
  local d=""; for f in $(ls $tmp/o); do cmp -s $tmp/o/$f locked8b/$f || d="$d $f"; done
  if [ -z "$d" ]; then echo "HOSTINDEP $tag: identical"; else echo "HOSTINDEP $tag: DIFFERS in$d"; rc=1; fi; }
run "+1ulp" "-O2" "-DDETMATH_PERTURB=+1"; run "-1ulp" "-O2" "-DDETMATH_PERTURB=-1"; run "-O0" "-O0" ""; run "-O3 -march=native" "-O3 -march=native" ""
rm -rf $tmp; [ $rc = 0 ] && echo HOSTINDEP8B_OK || { echo HOSTINDEP8B_FAIL; exit 1; }
