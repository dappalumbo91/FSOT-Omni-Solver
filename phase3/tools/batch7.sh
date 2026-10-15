#!/usr/bin/env bash
# batch7.sh: the full LOCK7 run set. Every compiled-leaf netlist (12 keys incl. GENpp): ideal 0/27/50 C, mcC seeds 1-8, mcB seeds 1-4 at 27 C;
# the 35 base-engine folds ideal 27 C. Individual run failures are recorded in the logs (summ7.py reports them); the batch itself exits 0
# once every run has been attempted, and prints the number of runs whose log has no converged solution.
cd "$(dirname "$0")"; FL=$(tail -n +2 ../locked7/base/folds35_targets.tsv | cut -f1); K="GEN GENpp T1 2_M 2_P 2_T 2_X 2a_M 2a_P 2a_Q 2a_T 2a_X"; P=${FSOT_JOBS:-7}
{ for k in $K; do for t in 0 27 50; do echo "ideal_ideal 0 $t $k"; done; done; for f in $FL; do echo "ideal_ideal 0 27 fold:$f"; done; } | xargs -P $P -L 1 ./run_bs7.sh || true
{ for k in $K; do for s in 1 2 3 4 5 6 7 8; do echo "mcC $s 27 $k"; done; for s in 1 2 3 4; do echo "mcB $s 27 $k"; done; done; } | xargs -P $P -L 1 ./run_bs7.sh || true
n=0; for l in ../../phase2/spice/out/bs7_*.log; do grep -q "^SOLVER: blocks" "$l" || n=$((n+1)); done; echo "BATCH7_DONE runs_without_solution=$n"
