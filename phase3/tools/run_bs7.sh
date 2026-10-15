#!/usr/bin/env bash
# run_bs7.sh <variant> <seed> <temp> <netlist key | fold:<Fold>> : LOCK7 block-sequential .op, netlists from phase3/cpp/netgen6 (LOCK7 base engine and compiled leaves)
# (leafc7 mode: compiled leaves from phase3/locked7; fold:<name>: LOCK7 base-engine channel, log-carried taps),
# solver phase3/cpp/blocksolve4 (LOCK4 Amendments 4.3/4.4, unchanged). Monte Carlo runs are seeded with the ideal solution (Amendment 4.4).
# Log: phase2/spice/out/bs7_<mode>_<variant>_s<seed>_t<temp>_<key>.log
set -e; cd "$(dirname "$0")/../../phase2/spice"; v=$1; s=$2; t=$3; f=$4
if [[ $f == fold:* ]]; then m=fold; f=${f#fold:}; else m=leafc7; fi
tag=bs7_${m/leafc7/leafc}_${v}_s${s}_t${t}_$f; w=$(mktemp -d /tmp/bs7XXXXXX)
../../phase3/cpp/netgen6 $m $v $s $w/n.cir $t op $f
sd=out/bs7_seed_${m}_${f}_t$t.ns
if [ "$v" = ideal_ideal ]; then ../../phase3/cpp/blocksolve4 $w/n.cir $w out/$tag.log >/dev/null || true; grep -q "^SOLVER: blocks" out/$tag.log && cp $w/solution.ns $sd
else [ -s $sd ] || { echo "run_bs7: no ideal seed $sd (run the ideal case first)" >&2; exit 1; }
  FSOT_BS_SEED=$sd ../../phase3/cpp/blocksolve4 $w/n.cir $w out/$tag.log >/dev/null || true; fi
[ "${FSOT_KEEP:-0}" = 1 ] && echo $w || rm -rf $w
