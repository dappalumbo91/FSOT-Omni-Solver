#!/usr/bin/env bash
# integ6.sh <variant> <temp> <Fold> : LOCK6 charge-integrator cross-check (Amendment 6.1). For each tiny tap of the fold, the DC current of the
# bench's direct uncascoded stage (u_<tap> in tb6_<variant>_t<temp>_<Fold>.log, the same device current an integrator would collect) is integrated
# on C_int = 1 nF || R_leak = 100 G (C0G insulation, model assumption) by an op-amp integrator from 0 V for T_int = 1 s, then held (input off) for
# another T_int. Three op-amp input-bias cases at the integrating node: 0 (ideal), 100 pA (OPA187 typical, datasheet), 25 fA (LMC6001A class, max).
# Output: phase2/spice/out/ig6_<variant>_t<temp>_<Fold>.tsv  tap  I_stage  IB  V(T/2)  V(T)  V(2T)  I_read=C V(T)/T
set -e; cd "$(dirname "$0")/../../phase2/spice"; v=$1; t=$2; f=$3; L=out/tb6_${v}_t${t}_$f.log; O=out/ig6_${v}_t${t}_$f.tsv; w=$(mktemp -d /tmp/ig6XXXXXX)
echo -e "tap\tI_stage\tIB\tV_half\tV_T\tV_2T\tI_read" > $O
grep -E "^u_" $L | while read -r nm eq I; do tap=${nm#u_}
  for IB in 0 100e-12 25e-15; do
    cat > $w/i.cir <<CIR
* integrator cross-check $tap IB=$IB
I1 ii 0 PWL(0 $I 1 $I 1.000001 0 2 0)
IB ii 0 DC $IB
E1 vo 0 0 ii 1e6
C1 ii vo 1n IC=0
RL ii vo 100G
.options temp=$t reltol=1e-9 abstol=1e-24 vntol=1e-15
.control
tran 1m 2 0 1m uic
meas tran vh find v(vo) at=0.5
meas tran vt find v(vo) at=1
meas tran v2 find v(vo) at=2
quit
.endc
.end
CIR
    ngspice -b $w/i.cir > $w/i.log 2>&1 || true
    r=$(awk '$1=="vh"||$1=="vt"||$1=="v2"{printf "%s ", $3}' $w/i.log)
    set -- $r; echo -e "$tap\t$I\t$IB\t$1\t$2\t$3\t$(awk -v x=$2 'BEGIN{printf "%.6e", x*1e-9/1}')" >> $O
  done; done
rm -rf $w; cat $O
