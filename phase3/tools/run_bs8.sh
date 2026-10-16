#!/usr/bin/env bash
# run_bs8.sh <variant> <seed> <temp> <netlist key> : LOCK8 (leaves, part C) block-sequential .op. Compiled leaves from phase3/locked8 (FSOT_P8_LOCKED),
# log-carry taps from phase3/locked8 (copies of LOCK7's, FSOT_P7_LOCKED), base engine LOCK7; netlists from phase3/cpp/netgen7 mode leafc8;
# solver phase3/cpp/blocksolve4 (LOCK4 Amendments 4.3/4.4, unchanged). Monte Carlo runs are seeded with the ideal solution (Amendment 4.4).
# Log: phase2/spice/out/bs8_leafc_<variant>_s<seed>_t<temp>_<key>.log
set -e; cd "$(dirname "$0")/../../phase2/spice"; export FSOT_P7_LOCKED=../../phase3/locked8 FSOT_P8_LOCKED=../../phase3/locked8; v=$1; s=$2; t=$3; f=$4
tag=bs8_leafc_${v}_s${s}_t${t}_$f; w=$(mktemp -d /tmp/bs8XXXXXX)
../../phase3/cpp/netgen7 leafc8 $v $s $w/n.cir $t op $f
sd=out/bs8_seed_leafc8_${f}_t$t.ns
if [ "$v" = ideal_ideal ]; then ../../phase3/cpp/blocksolve4 $w/n.cir $w out/$tag.log >/dev/null || true; grep -q "^SOLVER: blocks" out/$tag.log && cp $w/solution.ns $sd
else [ -s $sd ] || { echo "run_bs8: no ideal seed $sd (run the ideal case first)" >&2; exit 1; }
  FSOT_BS_SEED=$sd ../../phase3/cpp/blocksolve4 $w/n.cir $w out/$tag.log >/dev/null || true; fi
[ "${FSOT_KEEP:-0}" = 1 ] && echo $w || rm -rf $w
