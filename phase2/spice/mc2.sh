#!/usr/bin/env bash
# mc2.sh <class mcB|mcC> <nseeds> [temp]: Monte Carlo of the Omega_b h^2 netlist (Chemistry + Cosmology full-core-law
# channels, current-mode 1 - S_chem) and of the Atomic / HEP / QM / Fluid_Dynamics folds. Seeds 1..n, 8 in parallel.
set -e; cd "$(dirname "$0")"; c=$1; n=$2; t=${3:-27}
seq 1 $n | xargs -P ${FSOT_P2_JOBS:-8} -I{} ./run_omb.sh $c {} $t
for f in Atomic_Physics High_Energy_Physics Quantum_Mechanics Fluid_Dynamics; do seq 1 $n | xargs -P ${FSOT_P2_JOBS:-8} -I{} ./run_fold.sh $c {} $t $f; done
{ for s in $(seq 1 $n); do printf "%s\t" $s; ../cpp/score2 omb $t out/omb_${c}_s${s}_t${t}.log; done; } > ../results/mc_omb_${c}_t${t}.tsv
for f in Atomic_Physics High_Energy_Physics Quantum_Mechanics Fluid_Dynamics; do { for s in $(seq 1 $n); do printf "%s\t" $s; ../cpp/score2 fold $f $t out/fold_${c}_s${s}_t${t}_${f}.log; done; } > ../results/mc_fold_${c}_t${t}_${f}.tsv; done
grep -l "SOLVER: FAILED" out/*_${c}_s*_t${t}*.log 2>/dev/null | wc -l | xargs echo failed_runs:
