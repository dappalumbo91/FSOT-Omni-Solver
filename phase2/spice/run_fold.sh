#!/usr/bin/env bash
# run_fold.sh <variant> <seed> <temp> <fold> : one full-core-law channel .op; log in out/<tag>.log
# Solver aid only (no value or topology change): (1) the same draw is solved with ideal op-amps/comparators, starting from
# a converged seed solution of the same observed class; (2) its node voltages are the .nodeset initial guess of the real
# run, which then iterates freely. The attempt is recorded as "SOLVER: ..." at the end of the log.
set -e; cd "$(dirname "$0")"; v=$1; s=$2; t=$3; f=$4; tag=fold_${v}_s${s}_t${t}_${f}
ok() { grep -q "v(vsx) = " out/$tag.log && ! grep -q "aborted" out/$tag.log; }
obs=$(awk -F'\t' -v f=$f '$1==f{print $6}' ../locked/folds35_targets.tsv)
FSOT_P2_NODESET=out/seed_${obs}_ideal_ideal_t27.ns ../cpp/netgen2 fold $v $s netlists/$tag.d.cir $t dump $f
timeout 900 ngspice -b netlists/$tag.d.cir 2>&1 | grep -E "^[a-z0-9_.]+ += " > out/$tag.ns || true; rm -f netlists/$tag.d.cir
FSOT_P2_NODESET=out/$tag.ns ../cpp/netgen2 fold $v $s netlists/$tag.cir $t op $f
timeout ${FSOT_P2_TIMEOUT:-1800} ngspice -b netlists/$tag.cir > out/$tag.log 2>&1 || true
if ok; then echo "SOLVER: nodeset(ideal-op solve of the same draw)" >> out/$tag.log; else
  ../cpp/netgen2 fold $v $s netlists/$tag.cir $t op $f; timeout ${FSOT_P2_TIMEOUT:-1800} ngspice -b netlists/$tag.cir > out/$tag.log 2>&1 || true
  if ok; then echo "SOLVER: plain" >> out/$tag.log; else echo "SOLVER: FAILED" >> out/$tag.log; fi
fi
rm -f out/$tag.ns; [ "$s" -gt 0 ] && rm -f netlists/$tag.cir || true
