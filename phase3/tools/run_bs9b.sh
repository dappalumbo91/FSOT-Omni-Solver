#!/usr/bin/env bash
# run_bs9b.sh <variant> <netlist key> : LOCK9B ablation, ideal_ideal 27 C block solve.
#  base_m<M> : compiler v71 (phase3/locked9a/v71) on the LOCK7 base engine re-derived by derive9 with PTAT ratio M (phase3/locked9b/base_m<M>), netgen8 leafc9
#  <c>_b6    : compiler ablation <c> (phase3/locked9b/<c>_b6, = locked9a/<c> renamed) on the LOCK6 base engine (netgen5 leafc5)
# Log: phase2/spice/out/bs9b_<variant>_<key>.log
set -e; cd "$(dirname "$0")/../../phase2/spice"; v=$1; f=$2; w=$(mktemp -d /tmp/bs9bXXXXXX)
if [[ $v == *_b6 ]]; then FSOT_P5_LOCKED=../../phase3/locked9b/$v ../../phase3/cpp/netgen5 leafc5 ideal_ideal 0 $w/n.cir 27 op $f
else d=../../phase3/locked9a/v71; FSOT_P2_LOCKED=../../phase3/locked9b/$v FSOT_P7_LOCKED=$d FSOT_P9_LOCKED=$d ../../phase3/cpp/netgen8 leafc9 ideal_ideal 0 $w/n.cir 27 op $f; fi
../../phase3/cpp/blocksolve4 $w/n.cir $w out/bs9b_${v}_$f.log >/dev/null || true; rm -rf $w
