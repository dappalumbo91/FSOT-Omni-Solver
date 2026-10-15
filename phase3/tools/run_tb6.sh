#!/usr/bin/env bash
# run_tb6.sh <variant> <temp> <Fold>: LOCK6 tiny-tap bench (netgen5 tapbench, plain ngspice transient over T_int).
# Log: phase2/spice/out/tb6_<variant>_t<temp>_<Fold>.log (lines "x_<tap> = ..." log-carry injected current, "u_" direct uncascoded current,
# "c_" log-carry scaled readout voltage, "q_"/"h_" integrator output at T_int and T_int/2).
set -e; cd "$(dirname "$0")/../../phase2/spice"; v=$1; t=$2; f=$3; w=$(mktemp -d /tmp/tb6XXXXXX)
../../phase3/cpp/netgen5 tapbench $v 0 $w/n.cir $t op $f
ngspice -b $w/n.cir > out/tb6_${v}_t${t}_$f.log 2>&1 || true
rm -rf $w
