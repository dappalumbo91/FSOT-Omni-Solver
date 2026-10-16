#!/usr/bin/env bash
# run_cell10c.sh <cell> <run> [seedrun] (LOCK10A.2): cascoded antilog cells (CALa/b/c/t). As run_cell10.sh / run_cell10b.sh, but the
# "gain off" op-amp gain is 1e9 instead of netgen9's 1e10 (sed on the E-source gain field only): at 1e10 and above the cascoded CAL
# block is ill-conditioned in ngspice (CALa none: e = -1.8e-7, -1.4e-7, +2.3e-6, -2.2e-5, -3.6e-4 at 1e8..1e12). With 1e6 (gain on) the
# netlist is unchanged. Optional seed as in run_cell10b.sh (seed run solved with the same 1e9 substitution). Log c10c_<cell>_<run>.log
set -e; cd "$(dirname "$0")/../../phase2/spice"; c=$1; r=$2; sr=${3:-}; w=$(mktemp -d /tmp/c10cXXXXXX)
all="hi rb ib early gain round"
offs_of(){ local on; case $1 in none) on="";; only_rbib) on="rb ib";; only_*) on=${1#only_};; full) on=$all;; esac
  local o=""; for m in $all; do [[ " $on " == *" $m "* ]] || o="$o,$m"; done; echo ${o#,}; }
gen(){ FSOT_PHYS=$(offs_of $1) ../../phase3/cpp/netgen9 cell ideal_ideal 0 $2.raw 27 op $c; sed -E 's/^(E_[^ ]+ [^ ]+ [^ ]+ [^ ]+ [^ ]+) 1e10$/\1 1e9/' $2.raw > $2; }
gen $r $w/n0.cir
if [ -n "$sr" ]; then gen $sr $w/s.cir; mkdir $w/s; ../../phase3/cpp/blocksolve4 $w/s.cir $w/s $w/s.log >/dev/null || true
  awk -v ns=$w/s/solution.ns 'BEGIN{while((getline l < ns)>0){split(l,a," = "); if(a[1]!~/#/ && a[1]!~/^vos_|_p$/) s[a[1]]=a[2]}}
    /^\.options/{for(k in s) print ".nodeset v(" k ")=" s[k]} {print}' $w/n0.cir > $w/n.cir; else cp $w/n0.cir $w/n.cir; fi
mkdir $w/b; timeout 1200 ../../phase3/cpp/blocksolve4 $w/n.cir $w/b out/c10c_${c}_$r.log >/dev/null || true
[ -n "$sr" ] && echo "seeded from $sr" >> out/c10c_${c}_$r.log; rm -rf $w
