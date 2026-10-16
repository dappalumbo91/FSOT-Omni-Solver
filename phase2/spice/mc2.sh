#!/usr/bin/env bash
# mc2.sh <class mcB|mcC> <nseeds> [temp]: Monte Carlo of the Omega_b h^2 netlist (Chemistry + Cosmology full-core-law
# channels, current-mode 1 - S_chem) and of the Atomic / HEP / QM / Fluid_Dynamics folds. Seeds 1..n, 8 in parallel.
# Solved block-sequentially (run_bs.sh -> ../cpp/blocksolve: exact tearing at ideal op-amp outputs; LOCK.md Amendment 6).
set -e; cd "$(dirname "$0")"; c=$1; n=$2; t=${3:-27}; FL="Atomic_Physics High_Energy_Physics Quantum_Mechanics Fluid_Dynamics"
seq 1 $n | xargs -P ${FSOT_P2_JOBS:-8} -I{} ./run_bs.sh omb $c {} $t
for f in $FL; do seq 1 $n | xargs -P ${FSOT_P2_JOBS:-8} -I{} ./run_bs.sh fold $c {} $t $f; done
{ for s in $(seq 1 $n); do printf "%s\t" $s; ../cpp/score2 omb $t out/bs_omb_${c}_s${s}_t${t}.log; done; } > ../results/mc_omb_${c}_t${t}.tsv
for f in $FL; do { for s in $(seq 1 $n); do printf "%s\t" $s; ../cpp/score2 fold $f $t out/bs_fold_${c}_s${s}_t${t}_${f}.log; done; } > ../results/mc_fold_${c}_t${t}_${f}.tsv; done
echo "converged: $(grep -l "SOLVER: blocks" out/bs_*_${c}_s*_t${t}*.log | wc -l) / $(ls out/bs_*_${c}_s*_t${t}*.log | wc -l)"
