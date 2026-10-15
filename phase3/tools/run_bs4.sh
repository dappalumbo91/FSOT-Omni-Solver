#!/usr/bin/env bash
# run_bs4.sh <variant> <seed> <temp> <netlist key> : tier-4 (compiled leaves) block-sequential .op with the Phase 3b solver
# fork phase3/cpp/blocksolve4 (re-anchored verify, LOCK4 Amendment 4.3; ideal-seeded 4th attempt for MC, Amendment 4.4). Phase 2's run_bs.sh / blocksolve stay as locked.
# Log: phase2/spice/out/bs4_leafc_<variant>_s<seed>_t<temp>_<key>.log, same "v(n) = x" format, ending "SOLVER: blocks ...".
set -e; cd "$(dirname "$0")/../../phase2/spice"; v=$1; s=$2; t=$3; f=$4
tag=bs4_leafc_${v}_s${s}_t${t}_$f; w=$(mktemp -d /tmp/bs4XXXXXX)
../cpp/netgen2 leafc $v $s $w/n.cir $t op $f
# Amendment 4.4: a Monte Carlo run is seeded (4th attempt only) with the ideal solution of the same key and temperature
sd=out/bs4_seed_${f}_t$t.ns
if [ "$v" = ideal_ideal ]; then ../../phase3/cpp/blocksolve4 $w/n.cir $w out/$tag.log >/dev/null || true; grep -q "^SOLVER: blocks" out/$tag.log && cp $w/solution.ns $sd
else [ -s $sd ] || { echo "run_bs4: no ideal seed $sd (run the ideal case first)" >&2; exit 1; }
  FSOT_BS_SEED=$sd ../../phase3/cpp/blocksolve4 $w/n.cir $w out/$tag.log >/dev/null || true; fi
[ "${FSOT_KEEP:-0}" = 1 ] && echo $w || rm -rf $w
