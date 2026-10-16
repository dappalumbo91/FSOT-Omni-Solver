#!/usr/bin/env bash
# run_phys10.sh <set> <key> : LOCK10A leaf-level physics attribution. The 7.1/8.1 compiled leaves (phase3/locked9a/v71) on the LOCK7 base
# (netgen9 leafc9 = netgen8 netlists) with mechanisms switched off: set = full | none (FSOT_PHYS=all) | no_<m> (m in hi rb ib early gain vnep round).
# ideal_ideal, 27 C. Log: phase2/spice/out/bs10p_<set>_<key>.log
set -e; cd "$(dirname "$0")/../../phase2/spice"; s=$1; f=$2; w=$(mktemp -d /tmp/bs10pXXXXXX); d=../../phase3/locked9a/v71
case $s in full) ph="";; none) ph=all;; no_*) ph=${s#no_};; *) echo bad set; exit 1;; esac
FSOT_PHYS=$ph FSOT_P7_LOCKED=$d FSOT_P9_LOCKED=$d ../../phase3/cpp/netgen9 leafc9 ideal_ideal 0 $w/n.cir 27 op $f
../../phase3/cpp/blocksolve4 $w/n.cir $w out/bs10p_${s}_$f.log >/dev/null || true; rm -rf $w
