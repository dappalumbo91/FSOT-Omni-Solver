#!/usr/bin/env bash
# run_bs9a.sh <ablation variant> <netlist key> : LOCK9A ablation, ideal_ideal 27 C block solve of one compiled-leaf netlist.
# Variant dirs phase3/locked9a/<v>: leafc9 with FSOT_L9_ABL switches (LOCK9A.md), run on the LOCK7 base engine (netgen8 mode leafc9);
# <v>_b6: the same recipes renamed design5_/recipe5_, run on the LOCK6 base engine (netgen5 mode leafc5, unchanged).
# Log: phase2/spice/out/bs9a_<v>_<key>.log
set -e; cd "$(dirname "$0")/../../phase2/spice"; v=$1; f=$2; d=../../phase3/locked9a/$v; w=$(mktemp -d /tmp/bs9aXXXXXX)
if [[ $v == *_b6 ]]; then FSOT_P5_LOCKED=$d ../../phase3/cpp/netgen5 leafc5 ideal_ideal 0 $w/n.cir 27 op $f
else FSOT_P7_LOCKED=$d FSOT_P9_LOCKED=$d ../../phase3/cpp/netgen8 leafc9 ideal_ideal 0 $w/n.cir 27 op $f; fi
../../phase3/cpp/blocksolve4 $w/n.cir $w out/bs9a_${v}_$f.log >/dev/null || true; rm -rf $w
