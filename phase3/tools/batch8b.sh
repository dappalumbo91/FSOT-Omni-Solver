#!/usr/bin/env bash
# batch8b.sh: LOCK8 Amendment 8.1 run set: the 6 netlists whose E24 realisation changed in 8.1 (one or two summer resistors 62000 + 4700
# instead of 62000 + 4300 + 360): 2_M 2_P 2a_X GEN GENpp T1, ideal 0/27/50 C, mcC 1-8, mcB 1-4 at 27 C (keys overridable: FSOT_B8B_KEYS).
# C_S, C_H and the other 6 netlists are byte-identical to LOCK8 / LOCK7c (verify_lock8b.sh), so their results stand.
# Exits 0 once every run has been attempted; prints the number of logs without a converged solution.
cd "$(dirname "$0")"; K="${FSOT_B8B_KEYS:-2_M 2_P 2a_X GEN GENpp T1}"; P=${FSOT_JOBS:-7}
{ for k in $K; do for t in 0 27 50; do echo "ideal_ideal 0 $t $k"; done; done; } | xargs -P $P -L 1 ./run_bs8b.sh || true
{ for k in $K; do for s in 1 2 3 4 5 6 7 8; do echo "mcC $s 27 $k"; done; for s in 1 2 3 4; do echo "mcB $s 27 $k"; done; done; } | xargs -P $P -L 1 ./run_bs8b.sh || true
n=0; for l in ../../phase2/spice/out/bs8b_*.log; do grep -q "^SOLVER: blocks" "$l" || n=$((n+1)); done; echo "BATCH8B_DONE runs_without_solution=$n"
