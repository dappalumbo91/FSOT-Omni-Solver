#!/usr/bin/env bash
# batch5.sh: the full LOCK5 run set (ideal 0/27/50 C, mcC seeds 1-8, mcB seeds 1-4 at 27 C, every key; base-engine folds ideal 27 C). 8 parallel.
# Exit code: 0 once every run has been attempted (a failed run, e.g. the recorded 2a_X LOCK5 ideal failure whose MC seeds then have no ideal
# seed file, is reported in its log and counted below, not propagated through xargs/set -e).
cd "$(dirname "$0")"; K="GEN T1 2_M 2_P 2_T 2_X 2a_M 2a_P 2a_Q 2a_T 2a_X"
{ for k in $K; do for t in 0 27 50; do echo "ideal_ideal 0 $t $k"; done; done
  for f in $(tail -n +2 ../../phase2/locked/folds35_targets.tsv | cut -f1); do echo "ideal_ideal 0 27 fold:$f"; done; } | xargs -P 8 -L 1 ./run_bs5.sh || true
{ for k in $K; do for s in 1 2 3 4 5 6 7 8; do echo "mcC $s 27 $k"; done; for s in 1 2 3 4; do echo "mcB $s 27 $k"; done; done; } | xargs -P 8 -L 1 ./run_bs5.sh || true
n=0; for l in ../../phase2/spice/out/bs5_*.log; do grep -q "^SOLVER: blocks" "$l" || n=$((n+1)); done; echo "BATCH5_DONE runs_without_solution=$n"
