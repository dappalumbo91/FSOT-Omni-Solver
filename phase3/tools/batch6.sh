#!/usr/bin/env bash
# batch6.sh: LOCK6 run set. Folds (35, ideal 27 C) with log-carried taps; tap bench + integrator per fold at 0/27/50 C;
# compiled-leaf keys affected by LOCK6 (T1: CHAN taps; 2a_X: rectifier) ideal 0/27/50, mcC 1-8, mcB 1-4. 7 parallel.
set -e; cd "$(dirname "$0")"; FL=$(tail -n +2 ../../phase2/locked/folds35_targets.tsv | cut -f1)
{ for f in $FL; do echo "ideal_ideal 0 27 fold:$f"; done; for k in T1 2a_X; do for t in 0 27 50; do echo "ideal_ideal 0 $t $k"; done; done; } | xargs -P 7 -L 1 ./run_bs6.sh
for f in $FL; do for t in 0 27 50; do ./run_tb6.sh ideal_ideal $t $f; ./integ6.sh ideal_ideal $t $f > /dev/null; done; done
{ for k in T1 2a_X; do for s in 1 2 3 4 5 6 7 8; do echo "mcC $s 27 $k"; done; for s in 1 2 3 4; do echo "mcB $s 27 $k"; done; done; } | xargs -P 7 -L 1 ./run_bs6.sh
echo BATCH6_DONE
