#!/usr/bin/env bash
# reproduce2.sh <FSOT-2.1-Cpp checkout at 3466e8d>: Phase 2 nominal (ideal op-amp) reproduction used by CI.
#  build C++20 tools -> verify LOCK -> seed solves -> Chemistry, Atomic_Physics, High_Energy_Physics folds,
#  consts (alpha_s, 1/alpha) at 0/27/50 C, Omega_b h^2 -> score and check against the stated accuracy bars.
set -euo pipefail; cd "$(dirname "$0")/.."; ENG=${1:?path to FSOT-2.1-Cpp at 3466e8d}
F="-std=c++20 -O2 -ffp-contract=off"
g++ $F -I"$ENG/include" -o cpp/derive2 cpp/derive2.cpp; g++ $F -o cpp/netgen2 cpp/netgen2.cpp; g++ $F -o cpp/score2 cpp/score2.cpp
tools/verify_lock2.sh | tail -1
cd spice; mkdir -p out netlists
./make_seed.sh ideal_ideal 27 Chemistry 1 >/dev/null; ./make_seed.sh ideal_ideal 27 Fluid_Dynamics 0 >/dev/null
fail=0; chk() { awk -v x="$2" -v b="$3" 'BEGIN{x=(x<0?-x:x); exit !(x<=b)}' && echo "PASS $1 |rel|=$2 <= $3" || { echo "FAIL $1 rel=$2 bar $3"; fail=1; }; }
for f in Chemistry Atomic_Physics High_Energy_Physics; do ./run_fold.sh ideal_ideal 0 27 $f
  r=$(../cpp/score2 fold $f 27 out/fold_ideal_ideal_s0_t27_$f.log); echo "$r"; chk "S($f)" "$(echo "$r" | cut -f5)" 5e-4
  echo "$r" | awk -F'\t' '{exit !($12==$13)}' && echo "PASS D($f)" || { echo "FAIL D($f)"; fail=1; }; done
for t in 0 27 50; do ../cpp/netgen2 consts ideal_ideal 0 netlists/consts_ii_t$t.cir $t op
  timeout 600 ngspice -b netlists/consts_ii_t$t.cir > out/consts_ii_t$t.log 2>&1 || true
  r=$(../cpp/score2 consts $t out/consts_ii_t$t.log); echo "$r"; chk "1/alpha(${t}C)" "$(echo "$r" | cut -f7)" 3e-4; done
./run_omb.sh ideal_ideal 0 27; r=$(../cpp/score2 omb 27 out/omb_ideal_ideal_s0_t27.log); echo "$r"
chk "Omega_b h^2" "$(echo "$r" | cut -f9)" 6.7e-3
[ $fail = 0 ] && echo REPRODUCE2_OK || { echo REPRODUCE2_FAIL; exit 1; }
