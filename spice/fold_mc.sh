#!/usr/bin/env bash
# fold_mc.sh <variant> <seed>: the fold channel at every generation g = 0..19 (same component draw), one line per g
cd "$(dirname "$0")"; v=$1; s=$2
for g in $(seq 0 19); do f=netlists/fg_${v}_${s}_${g}.cir; ../cpp/netgen foldg$g $v $s $f 27 op
  r=$(timeout 60 ngspice -b $f 2>&1 | grep -E "^[iv]\(" | awk '{print $3}' | tr '\n' ' '); echo "$v $s $g $r"; rm -f $f; done
