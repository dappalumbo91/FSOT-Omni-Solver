#!/usr/bin/env bash
# batch8.sh: LOCK8 run set: the two new netlists C_S (domain_scalar, fsot_correct factors) and C_H (hub dynamics coefficients): ideal 0/27/50 C,
# mcC 1-8, mcB 1-4 at 27 C. The other 12 LOCK8 netlists are identical to LOCK7c's (verify_lock8.sh), so results/leaves7b_* stand for them.
# Exits 0 once every run has been attempted; prints the number of logs without a converged solution.
cd "$(dirname "$0")"; K="${FSOT_B8_KEYS:-C_S C_H}"; P=${FSOT_JOBS:-7}
{ for k in $K; do for t in 0 27 50; do echo "ideal_ideal 0 $t $k"; done; done; } | xargs -P $P -L 1 ./run_bs8.sh || true
{ for k in $K; do for s in 1 2 3 4 5 6 7 8; do echo "mcC $s 27 $k"; done; for s in 1 2 3 4; do echo "mcB $s 27 $k"; done; done; } | xargs -P $P -L 1 ./run_bs8.sh || true
n=0; for l in ../../phase2/spice/out/bs8_*.log; do grep -q "^SOLVER: blocks" "$l" || n=$((n+1)); done; echo "BATCH8_DONE runs_without_solution=$n"
