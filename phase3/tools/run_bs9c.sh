#!/usr/bin/env bash
# run_bs9c.sh <variant> <netlist key> : LOCK9C ablation (high-injection compensation), ideal_ideal 27 C block solve.
#  hi, allhi        : leafc10 recipes (phase3/locked9c/<v>) on the LOCK7 base engine, netgen8 leafc9
#  hi_m16           : locked9c/hi on the LOCK7 base re-derived at PTAT ratio 16 (phase3/locked9b/base_m16)
#  hi_b6, allhi_b6  : the same recipes renamed design5_/recipe5_ on the LOCK6 base engine (netgen5 leafc5)
# Log: phase2/spice/out/bs9c_<variant>_<key>.log
set -e; cd "$(dirname "$0")/../../phase2/spice"; v=$1; f=$2; w=$(mktemp -d /tmp/bs9cXXXXXX)
if [[ $v == *_b6 ]]; then FSOT_P5_LOCKED=../../phase3/locked9c/$v ../../phase3/cpp/netgen5 leafc5 ideal_ideal 0 $w/n.cir 27 op $f
elif [[ $v == hi_m16 ]]; then d=../../phase3/locked9c/hi; FSOT_P2_LOCKED=../../phase3/locked9b/base_m16 FSOT_P7_LOCKED=$d FSOT_P9_LOCKED=$d ../../phase3/cpp/netgen8 leafc9 ideal_ideal 0 $w/n.cir 27 op $f
else d=../../phase3/locked9c/$v; FSOT_P7_LOCKED=$d FSOT_P9_LOCKED=$d ../../phase3/cpp/netgen8 leafc9 ideal_ideal 0 $w/n.cir 27 op $f; fi
../../phase3/cpp/blocksolve4 $w/n.cir $w out/bs9c_${v}_$f.log >/dev/null || true; rm -rf $w
