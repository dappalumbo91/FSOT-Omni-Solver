#!/usr/bin/env bash
# run_cell10b.sh <cell> <run> <seedrun> (LOCK10A.1): as run_cell10.sh, for runs whose cascode output block does not converge from ngspice's
# default start (sub-uA VDMOS cascode with a 1e10 servo: the pseudo-transient op stalls). The SAME netlist (netgen9 cell, identical
# FSOT_PHYS) gets one .nodeset line per node taken from the converged solution.ns of <seedrun> of the same cell. blocksolve4 is
# unchanged: its verify (plain Newton reproduces the point, tolerances unchanged) still decides; the seed is never copied into the result.
set -e; cd "$(dirname "$0")/../../phase2/spice"; c=$1; r=$2; sr=$3; w=$(mktemp -d /tmp/c10bXXXXXX)
all="hi rb ib early gain round"
offs_of(){ local on; case $1 in none) on="";; only_rbib) on="rb ib";; only_*) on=${1#only_};; full) on=$all;; esac
  local o=""; for m in $all; do [[ " $on " == *" $m "* ]] || o="$o,$m"; done; echo ${o#,}; }
FSOT_PHYS=$(offs_of $sr) ../../phase3/cpp/netgen9 cell ideal_ideal 0 $w/s.cir 27 op $c; mkdir $w/s
../../phase3/cpp/blocksolve4 $w/s.cir $w/s $w/s.log >/dev/null || true
FSOT_PHYS=$(offs_of $r) ../../phase3/cpp/netgen9 cell ideal_ideal 0 $w/n0.cir 27 op $c
awk -v ns=$w/s/solution.ns 'BEGIN{while((getline l < ns)>0){split(l,a," = "); if(a[1]!~/#/ && a[1]!~/^vos_|_p$/) s[a[1]]=a[2]}}
  /^\.options/{for(k in s) print ".nodeset v(" k ")=" s[k]} {print}' $w/n0.cir > $w/n.cir
mkdir $w/b; ../../phase3/cpp/blocksolve4 $w/n.cir $w/b out/c10_${c}_$r.log >/dev/null || true
echo "seeded from $sr" >> out/c10_${c}_$r.log; rm -rf $w
