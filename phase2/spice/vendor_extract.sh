#!/usr/bin/env bash
# vendor_extract.sh <mode fold|omb|consts> <temp> <tag> [fold]
# Vendor-model (TI OPAx187) error at every op-amp's own operating point, injected into the full channel.
# Why: ngspice cannot solve the full netlists with 150+ OPAx187 Green-Williams-Lis macro-models (cold start, nodeset and
# full-state .ic all fail: gmin/source stepping drives the models' IF()/LIMIT() clamp sources and self-controlled
# VSWITCH "ideal diodes" into a singular state). Each OPAx187 instance DOES solve alone, so:
#  1. the same E24 draw is solved with ideal op-amps (analysis dump);
#  2. each OPAx187 is solved alone in a servo jig holding its non-inverting input at the solved voltage, its output at
#     the solved voltage (feedback E source) and its output load at the solved load current, at the same temperature;
#  3. the jig's input error V(+)-V(-) (Vos, finite Aol at that output, CMRR/PSRR at that common mode) and its input bias
#     currents (from the source branch currents) are injected into the ideal-op netlist as that op-amp's VOS source
#     and two bias-current sources; that netlist is solved and scored.
# Iterating 1-3 once more changes the operating points by << the injected errors (checked, reported in the log).
set -e; cd "$(dirname "$0")"; m=$1; t=$2; tag=$3; f=${4:-}
J=out/vx_$tag; mkdir -p $J; rm -f $J/*
inc=".include $PWD/models/vendor/ti_sbomab6/OPAx187.LIB"
../cpp/netgen2 $m e24_ideal 0 $J/base.cir $t op $f   # E24 values, ideal op-amps (the netlist that gets the injected errors)
opts=$(grep '^\.options' $J/base.cir)
solve() { # $1 netlist -> $J/sol.txt (node = value lines) via nodeset seeds where available
  local obs=""; [ -n "$f" ] && obs=$(awk -F'\t' -v f=$f '$1==f{print $6}' ../locked/folds35_targets.tsv)
  sed '/^\.control/,$d' $1 > $J/s.cir; printf ".control\nop\nprint all\nquit\n.endc\n.end\n" >> $J/s.cir
  timeout 900 ngspice -b $J/s.cir 2>&1 | grep -E "^[a-z0-9_.#]+ += " > $J/sol.txt || true; }
mkns() { [ -n "$f" ] && echo out/seed_$(awk -F'\t' -v f=$f '$1==f{print $6}' ../locked/folds35_targets.tsv)_ideal_ideal_t27.ns; }
if [ "$m" = fold ]; then FSOT_P2_NODESET=$(mkns) ../cpp/netgen2 fold e24_ideal 0 $J/base.cir $t op $f; fi
if [ "$m" = omb ]; then ./run_omb.sh e24_ideal 0 $t >/dev/null; cp netlists/omb_e24_ideal_s0_t${t}.cir $J/base.cir; fi
solve $J/base.cir
val() { awk -v n="$1" '$1==n{print $3; exit}' $J/sol.txt; }
: > $J/inj.txt
grep -E "^E_[A-Za-z0-9_]+ " $J/base.cir | while read -r E o z p mi g; do
  nm=${E#E_}; l() { echo "$1" | tr 'A-Z' 'a-z'; }; pl=$(grep -E "^VOS_$nm " $J/base.cir | awk '{print $3}')
  vp=$(val "$(l $pl)"); vo=$(val "$(l $o)"); il=$(val "e_$(l $nm)#branch"); vp=${vp:-0}; vo=${vo:-0}; il=${il:-0}
  { echo "jig $nm"; echo "$inc"; echo "VCC vcc 0 9"; echo "VEE vee 0 -9"; echo "VP pj 0 DC $vp"
    echo "X_$nm pj mj vcc vee oj OPAx187"; echo "VX x 0 DC $(awk -v a="$vp" -v b="$vo" 'BEGIN{printf "%.9e", a-b}')"; echo "E_fb mj x oj 0 1"
    echo "I_ld oj 0 DC $(awk -v x="$il" 'BEGIN{printf "%.6e", -x}')"; echo "$opts"
    printf ".control\nop\nlet vd = v(pj)-v(mj)\nlet ibp = -i(vp)\nlet ibm = -i(e_fb)\nprint vd\nprint ibp\nprint ibm\nprint v(oj)\nquit\n.endc\n.end\n"; } > $J/j_$nm.cir
  r=$(timeout 120 ngspice -b $J/j_$nm.cir 2>&1 | awk -F' = ' '/^(vd|ibp|ibm|v\(oj\)) = /{printf "%s ", $2}')
  if [ $(echo $r | wc -w) -lt 4 ]; then  # retry 1: initial guess at the servo point; retry 2: unloaded output (load only shifts Vos via Aol)
    sed -i "s/^\.options/.nodeset v(oj)=$vo v(mj)=$vp v(pj)=$vp\n.options/" $J/j_$nm.cir
    r=$(timeout 120 ngspice -b $J/j_$nm.cir 2>&1 | awk -F' = ' '/^(vd|ibp|ibm|v\(oj\)) = /{printf "%s ", $2}')
    if [ $(echo $r | wc -w) -lt 4 ]; then sed -i 's/^I_ld oj 0 DC .*/I_ld oj 0 DC 0/' $J/j_$nm.cir
      r=$(timeout 120 ngspice -b $J/j_$nm.cir 2>&1 | awk -F' = ' '/^(vd|ibp|ibm|v\(oj\)) = /{printf "%s ", $2}'); [ -n "$r" ] && r="$r unloaded"; fi; fi
  echo "$nm $pl $mi $vo $r" >> $J/inj.txt
done
# inject: VOS_<nm> := -(V+ - V-)_vendor ; bias currents drawn from the + and - input nodes
awk 'NR==FNR{if(NF>=8){vos[$1]=-$5; ibp[$1]=$6; ibm[$1]=$7; pn[$1]=$2; mn[$1]=$3}; next}
  /^VOS_/{n=substr($1,5); if(n in vos){$5=sprintf("%.6e",vos[n]); print; printf "IBP_%s %s 0 DC %.6e\nIBM_%s %s 0 DC %.6e\n", n, pn[n], ibp[n], n, mn[n], ibm[n]; next}} {print}' $J/inj.txt $J/base.cir > netlists/$tag.cir
nj=$(wc -l < $J/inj.txt); ok=$(awk 'NF>=8' $J/inj.txt | wc -l)
timeout 1800 ngspice -b netlists/$tag.cir > out/$tag.log 2>&1 || true
echo "VENDOR_EXTRACT: OPAx187 jigs solved $ok/$nj; |Vos_eff| max $(awk 'NF>=8{x=$5<0?-$5:$5; if(x>m)m=x} END{printf "%.3g", m}' $J/inj.txt) V; |Ib| max $(awk 'NF>=8{for(i=6;i<=7;i++){x=$i<0?-$i:$i; if(x>m)m=x}} END{printf "%.3g", m}' $J/inj.txt) A" >> out/$tag.log
tail -1 out/$tag.log
