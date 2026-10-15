#!/usr/bin/env bash
# run_bs.sh <fold|omb|consts> <variant> <seed> <temp> [fold] : block-sequential .op (../cpp/blocksolve, exact tearing at ideal
# op-amp outputs); log in out/bs_<tag>.log in the same "v(n) = x" format as the full-netlist runs, ending "SOLVER: blocks ...".
set -e; cd "$(dirname "$0")"; m=$1; v=$2; s=$3; t=$4; f=${5:-}
tag=bs_${m}_${v}_s${s}_t${t}${f:+_$f}; w=$(mktemp -d /tmp/bsXXXXXX)
../cpp/netgen2 $m $v $s $w/n.cir $t op $f
../cpp/blocksolve $w/n.cir $w out/$tag.log >/dev/null || true
[ "${FSOT_KEEP:-0}" = 1 ] && echo $w || rm -rf $w
