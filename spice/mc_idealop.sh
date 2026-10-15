#!/usr/bin/env bash
# mc_idealop.sh — Harness amendment 3 (post-lock, no value/topology change): the full-slice TL082 .op fails to converge for
# ~60% of mcA draws (ngspice gmin/source stepping, 60 s timeout), which would bias the MC toward survivors. The scored MC
# therefore solves each draw with high-gain (1e6) op-amps carrying that draw's TL082-distributed Vos (netgen opt "idealop";
# every resistor, E24 piece, BJT IS, reference and switch Ron draw is identical to the TL082 netlist of the same seed).
# The TL082 finite-gain/input-stage systematic is taken from the nominal e24_tl082 vs e24 idealop pair and from the
# converged TL082 mcA seeds (paired check). Fold MC keeps the real TL082 macromodel (it converges).
cd "$(dirname "$0")"
one() { v=$1; s=$2; tag=slice_${v}_s${s}_t27_idealop; ../cpp/netgen slice $v $s netlists/$tag.cir 27 idealop && timeout 60 ngspice -b netlists/$tag.cir > out/$tag.log 2>&1; rm -f netlists/$tag.cir; }
export -f one
for v in mcA mcB mcC; do seq 1 200 | xargs -P 8 -I{} bash -c "one $v {}"; done
one e24_tl082 0
for v in mcA mcB mcC; do seq 1 100 | xargs -P 8 -I{} ./fold_mc.sh $v {} > ../results/fold_g_$v.txt; done
echo ALLDONE
