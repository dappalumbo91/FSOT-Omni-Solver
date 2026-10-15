#!/usr/bin/env bash
# run_bs4.sh <variant> <seed> <temp> <netlist key> : tier-4 (compiled leaves) block-sequential .op with the Phase 3b solver
# fork phase3/cpp/blocksolve4 (re-anchored verify; LOCK4 Amendment 4.3). Phase 2's run_bs.sh / blocksolve stay as locked.
# Log: phase2/spice/out/bs4_leafc_<variant>_s<seed>_t<temp>_<key>.log, same "v(n) = x" format, ending "SOLVER: blocks ...".
set -e; cd "$(dirname "$0")/../../phase2/spice"; v=$1; s=$2; t=$3; f=$4
tag=bs4_leafc_${v}_s${s}_t${t}_$f; w=$(mktemp -d /tmp/bs4XXXXXX)
../cpp/netgen2 leafc $v $s $w/n.cir $t op $f
../../phase3/cpp/blocksolve4 $w/n.cir $w out/$tag.log >/dev/null || true
[ "${FSOT_KEEP:-0}" = 1 ] && echo $w || rm -rf $w
