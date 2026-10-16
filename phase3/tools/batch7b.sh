#!/usr/bin/env bash
# batch7b.sh: LOCK7 Amendment 7.1 run set (compiled leaves only; the base engine is LOCK7's): 12 keys, ideal 0/27/50 C, mcC 1-8, mcB 1-4 at 27 C.
# Exits 0 once every run has been attempted; prints the number of logs without a converged solution.
cd "$(dirname "$0")"; K="GEN GENpp T1 2_M 2_P 2_T 2_X 2a_M 2a_P 2a_Q 2a_T 2a_X"; P=${FSOT_JOBS:-7}
{ for k in $K; do for t in 0 27 50; do echo "ideal_ideal 0 $t $k"; done; done; } | xargs -P $P -L 1 ./run_bs7b.sh || true
{ for k in $K; do for s in 1 2 3 4 5 6 7 8; do echo "mcC $s 27 $k"; done; for s in 1 2 3 4; do echo "mcB $s 27 $k"; done; done; } | xargs -P $P -L 1 ./run_bs7b.sh || true
n=0; for l in ../../phase2/spice/out/bs7b_*.log; do grep -q "^SOLVER: blocks" "$l" || n=$((n+1)); done; echo "BATCH7B_DONE runs_without_solution=$n"
