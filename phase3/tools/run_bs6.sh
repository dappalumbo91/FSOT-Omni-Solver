#!/usr/bin/env bash
# run_bs6.sh <variant> <seed> <temp> <netlist key | fold:<Fold>> : LOCK6 block-sequential .op, netlists from phase3/cpp/netgen5 (log-carried tiny taps, low-leakage FWR)
# (leafc5 mode: compiled leaves from phase3/locked5; fold:<name>: base-engine channel with the LOCK5 tiny-tap rule and tap sense sources),
# solver phase3/cpp/blocksolve4 (LOCK4 Amendments 4.3/4.4, unchanged). Monte Carlo runs are seeded with the ideal solution (Amendment 4.4).
# Log: phase2/spice/out/bs6_<mode>_<variant>_s<seed>_t<temp>_<key>.log
set -e; cd "$(dirname "$0")/../../phase2/spice"; v=$1; s=$2; t=$3; f=$4
if [[ $f == fold:* ]]; then m=fold; f=${f#fold:}; else m=leafc5; fi
tag=bs6_${m/leafc5/leafc}_${v}_s${s}_t${t}_$f; w=$(mktemp -d /tmp/bs6XXXXXX)
../../phase3/cpp/netgen5 $m $v $s $w/n.cir $t op $f
sd=out/bs6_seed_${m}_${f}_t$t.ns
if [ "$v" = ideal_ideal ]; then ../../phase3/cpp/blocksolve4 $w/n.cir $w out/$tag.log >/dev/null || true; grep -q "^SOLVER: blocks" out/$tag.log && cp $w/solution.ns $sd
else [ -s $sd ] || { echo "run_bs6: no ideal seed $sd (run the ideal case first)" >&2; exit 1; }
  FSOT_BS_SEED=$sd ../../phase3/cpp/blocksolve4 $w/n.cir $w out/$tag.log >/dev/null || true; fi
[ "${FSOT_KEEP:-0}" = 1 ] && echo $w || rm -rf $w
