#!/usr/bin/env bash
# loopgain.sh <netlist.cir> <out.tsv>: loop gain of every op-amp loop (LOCK Amendment 7 dynamic op-amp, FSOT_P2_OPDYN=1 netlists).
# The netlist is solved block-sequentially (blocksolve, .op); then, inside each block at that operating point, a 1 V AC source is
# put in series with each op-amp output (E_<nm>: out <- <nm>_a), T = -v(<nm>_a)/v(out) (voltage injection; exact for a
# zero-impedance source), and the unity-gain frequency and phase margin are read off. "-" = no unity crossing (open loop).
set -e; cir=$1; out=$2; w=$(mktemp -d /tmp/lgXXXXXX)
"$(dirname "$0")"/../cpp/blocksolve "$cir" $w $w/op.log >/dev/null || { echo "op failed"; exit 1; }
inj() {  # <block.cir> <E name> <out node> <dat> -> stdout netlist
  sed -E "s/^$2 $3 0 /$2 ${2}_a 0 /; s/^\.control/VINJ ${2}_a $3 DC 0 AC 1\n.control/; s/^op$/ac dec 40 0.01 1e8\nlet T = -v(${2}_a)\/v($3)\nwrdata $4 vdb(T) cph(T)/; /^print all/d; /^set numdgt/d" $1; }
withns() {  # add .nodeset lines (block solution) before .options
  awk -v NS=$w/solution.ns 'BEGIN{while((getline l<NS)>0){split(l,a," "); if(a[1]!~/#/) v[a[1]]=a[3]}} /^\.options/{for(n in seen) if((n in v) && !(n in vs)) printf ".nodeset v(%s)=%s\n", n, v[n]} /^[Vv]/{vs[tolower($2)]=1; vs[tolower($3)]=1} !/^[.*+]/{for(i=2;i<=NF;i++) seen[tolower($i)]=1} {print}' | awk '{print} END{}' ; }
printf "opamp\tblock\tf_unity_Hz\tphase_margin_deg\tdc_gain_dB\n" > $out
for b in $w/b[0-9]*.cir; do
  case $b in *.v.cir) continue;; esac; bk=$(basename $b .cir)
  for e in $(grep -oE "^E_[^ ]+ [^ ]+ 0 [^ ]+_f 0 1$" $b | awk '{print $1":"$2}'); do nm=${e%%:*}; o=${e##*:}; dat=${bk}_$nm.dat
    inj $b $nm $o $dat | grep -v "^\.nodeset" | withns > $w/lg.cir; (cd $w && timeout 120 ngspice -b lg.cir > lg.log 2>&1 || true)
    if [ ! -s $w/$dat ]; then inj $b $nm $o $dat > $w/lg.cir; (cd $w && timeout 120 ngspice -b lg.cir > lg.log 2>&1 || true); fi
    if [ ! -s $w/$dat ]; then printf "%s\t%s\tnan\tnan\tnan\n" $nm $bk >> $out; continue; fi
    awk -v nm=$nm -v bk=$bk 'NR==1{dc=$2} {f=$1; m=$2; p=$4*57.29578; if(NR>1 && pm=="" && pmag>0 && m<=0){fu=pf+(f-pf)*(pmag/(pmag-m)); ph=pp+(p-pp)*(pmag/(pmag-m)); pm=180+ph}; pf=f; pmag=m; pp=p}
      END{if(pm=="") printf "%s\t%s\t-\t-\t%.1f\n", nm, bk, dc; else printf "%s\t%s\t%.4g\t%.1f\t%.1f\n", nm, bk, fu, pm, dc}' $w/$dat >> $out
  done; done
[ "${FSOT_KEEP:-0}" = 1 ] && echo $w || rm -rf $w
