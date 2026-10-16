#!/usr/bin/env bash
# run_cell10.sh <cell> <run> : LOCK10A isolated-cell bench (netgen9 mode cell, blocksolve4). run: none | only_hi | only_rbib | only_ib | only_early |
# only_gain | only_round | full.  none = every mechanism off (FSOT_PHYS hi,rb,ib,early,gain,round); only_<m> = none with m on (rbib: rb and ib on);
# full = netgen8 physics. Log: phase2/spice/out/c10_<cell>_<run>.log
set -e; cd "$(dirname "$0")/../../phase2/spice"; c=$1; r=$2; w=$(mktemp -d /tmp/c10XXXXXX)
all="hi rb ib early gain round"; case $r in none) on="";; only_rbib) on="rb ib";; only_*) on=${r#only_};; full) on=$all;; *) echo bad run; exit 1;; esac
offs=""; for m in $all; do [[ " $on " == *" $m "* ]] || offs="$offs,$m"; done; offs=${offs#,}
FSOT_PHYS=$offs ../../phase3/cpp/netgen9 cell ideal_ideal 0 $w/n.cir 27 op $c
../../phase3/cpp/blocksolve4 $w/n.cir $w out/c10_${c}_$r.log >/dev/null || true; rm -rf $w
