#!/usr/bin/env bash
# run_omb.sh <variant> <seed> <temp>: Chemistry + Cosmology channels, 1 - S_chem current node, Omega_b h^2.
# Solver aid (no value/topology change): .nodeset from single-channel converged solves (prefix c / m) of the same variant
# with ideal op-amps; then the full netlist iterates freely.
set -e; cd "$(dirname "$0")"; v=$1; s=$2; t=$3; tag=omb_${v}_s${s}_t${t}
ns=out/$tag.ns; : > $ns
for pair in "c Chemistry" "m Cosmology"; do set -- $pair
  FSOT_P2_CHPREFIX=$1 FSOT_P2_NODESET=out/seed_$([ $1 = c ] && echo 1 || echo 0)_ideal_ideal_t27.ns ../cpp/netgen2 fold $v $s netlists/$tag.$1.cir $t dump $2
  timeout 900 ngspice -b netlists/$tag.$1.cir 2>&1 | grep -E "^[a-z0-9_.]+ += " >> $ns || true; rm -f netlists/$tag.$1.cir
done
FSOT_P2_NODESET=$ns ../cpp/netgen2 omb $v $s netlists/$tag.cir $t op
timeout 1800 ngspice -b netlists/$tag.cir > out/$tag.log 2>&1 || true
if grep -q "v(vomb) = " out/$tag.log && ! grep -q aborted out/$tag.log; then echo "SOLVER: nodeset" >> out/$tag.log; else echo "SOLVER: FAILED" >> out/$tag.log; fi
rm -f $ns; [ "$s" -gt 0 ] && rm -f netlists/$tag.cir || true
