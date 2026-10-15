#!/usr/bin/env bash
# stepresp.sh <fold> <E_name>... : loops whose AC loop gain could not be measured (loopgain.sh "nan" / "-").
# Ideal block solve -> dynamic op-amp (LOCK Amendment 7) block solve from that nodeset -> for each named op-amp, its block alone:
# a transient started at the block's operating point (.ic on every node; transient op with the .ic pins, uic only as fallback) with a 1 mV step injected in series with the
# op-amp output at 20 us. The loop rejects the disturbance; from the recovery y(t) = v(out) - v(out, t<20us):
#   * "saturated": the op-amp's integrator sits at its +-8.9 V clamp, so the loop is not in linear operation (no loop gain, it cannot ring);
#   * otherwise the peak overshoot past the pre-step value, opposite to the kick, OS = -extreme(y)/y(0+) -> damping ratio zeta -> phase margin of the equivalent 2nd-order loop
#     (OS = 0 -> zeta >= 1 -> PM >= 76 deg). Validated against loopgain.sh on loops where AC works (see results/stepresp_*.tsv).
# Env: FSOT_SR_TS (step time, default 20u) and FSOT_SR_TSTOP (default 2m), FSOT_SR_DT (max step, default 0.05u). The slow
# tiny-tap cascodes (nA collector currents) need TS ~ 20m so the uic start-up has settled before the step.
set -e; cd "$(dirname "$0")"; fold=$1; shift; w=$(mktemp -d /tmp/srXXXXXX)
TS=${FSOT_SR_TS:-20u}; TSTOP=${FSOT_SR_TSTOP:-2m}; DT=${FSOT_SR_DT:-0.05u}
num() { awk -v t=$1 'BEGIN{m=1; if(t~/u$/)m=1e-6; else if(t~/m$/)m=1e-3; sub(/[um]$/,"",t); print t*m}'; }
tsn=$(num $TS); dtn=$(num $DT)
../cpp/netgen2 fold ideal_ideal 0 $w/i.cir 27 op $fold; ../cpp/blocksolve $w/i.cir $w/i $w/i.log >/dev/null
FSOT_P2_OPDYN=1 FSOT_P2_NODESET=$w/i/solution.ns ../cpp/netgen2 fold ideal_ideal 0 $w/d.cir 27 op $fold; ../cpp/blocksolve $w/d.cir $w/d $w/d.log >/dev/null || true
# If the dynamic-op-amp block solve fails (its plain-Newton verify cannot restart the OPDYN clamps at nA cascode currents; the
# solver itself is LOCK2 and is not modified for this), fall back to SEEDED mode: each target block is taken from the ideal block
# solve and every static op-amp E (gain 1e6) in it is replaced by its OPDYN expansion from d.cir (same DC gain 1e6, so the same
# operating point to ~1e-6); the integrator node X_f starts at the op-amp output value. The transient then starts at equilibrium.
D=$w/d; NS=$w/d/solution.ns
if ! grep -q "SOLVER: blocks" $w/d.log; then echo "# dynamic op-amp block solve failed ($(tail -1 $w/d.log | cut -c1-80)); seeded mode from the ideal solve" >&2
  D=$w/s; mkdir -p $D; NS=$D/solution.ns
  awk 'FNR==NR { if ($1 ~ /^E_/ && $4 ~ /_f$/ && $6 == 1) f[tolower($4)] = tolower($2); next } { split($0, a, " = "); n = tolower(a[1]); v[n] = a[2]; print }
       END { for (x in f) if (f[x] in v) print x " = " v[f[x]] }' $w/d.cir $w/i/solution.ns > $NS
  for b in $w/i/b[0-9]*.cir; do case $b in *.v*) continue;; esac
    awk 'FNR==NR { if (match($1, /^(B_.*_g|R_.*_o|C_.*_o|B_.*_c)$/)) { x=$1; sub(/^[BRC]_/, "", x); sub(/_[gocO]$/, "", x); dyn[tolower(x)] = dyn[tolower(x)] $0 "\n" }
                   if ($1 ~ /^E_/ && $4 ~ /_f$/) { x=$1; sub(/^E_/, "", x); dyn[tolower(x)] = dyn[tolower(x)] $0 "\n" } next }
         $1 ~ /^E_/ && $NF == "1e6" { x=$1; sub(/^E_/, "", x); if (tolower(x) in dyn) { printf "%s", dyn[tolower(x)]; next } } { print }' $w/d.cir $b > $D/$(basename $b); done
fi
printf "opamp\tblock\tintegrator_V\tstate\tundershoot\tzeta\tPM_est_deg\tring_Hz\n"
for e in "$@"; do nm=${e#E_}; b=$(grep -l "^$e " $D/b[0-9]*.cir | grep -v '\.v' | head -1); [ -n "$b" ] || { printf "%s\t-\tnot found\n" $e; continue; }
  o=$(grep "^$e " $b | awk '{print $2}'); bk=$(basename $b .cir)
  gv=$(awk -v O="$o" 'tolower($1)==tolower(O){print $3}' $NS)   # servo output in the static solution: beyond the +-8.9 V clamp the loop is railed
  if [ -n "$gv" ] && awk -v g="$gv" 'BEGIN{exit !(g>8.9||g<-8.9)}'; then printf "%s\t%s\t%.4g\tsaturated (static output beyond clamp)\t-\t-\t-\t-\n" $e $bk $gv; continue; fi
  awk -v E="$e" -v O="$o" -v NM="$nm" -v NS=$NS -v TS=$TS -v TSN=$tsn -v DTN=$dtn -v TSTOP=$TSTOP -v DT=$DT 'BEGIN{while((getline l<NS)>0){split(l,a," "); v[tolower(a[1])]=a[3]}}
    /^\.nodeset/ {next}
    $1==E {$2=NM"_a"; print; next}
    /^[Vv]/ {vs[tolower($2)]=1; vs[tolower($3)]=1}
    !/^[.*+]/ {for(i=2;i<=NF;i++) seen[tolower($i)]=1}
    /^\.options/ {for(n in seen) if((n in v) && !(n in vs) && n!=tolower(NM"_a")) printf ".ic v(%s)=%s\n", n, v[n]; print "VINJ " NM "_a " O " PWL(0 0 " TS " 0 " (TSN+(DTN>1e-7?DTN:1e-7)) " 1m)"}
    /^op$/ {print "tran " DT " " TSTOP " 0 " DT " uic"; print "wrdata " NM ".dat v(" O ") v(" NM "_f)"; next}
    /^print all/ {next} {print}' $b > $w/s_$nm.cir
  # start: transient op with every node pinned by .ic (no uic), so device-internal nodes start consistent; if that op fails, uic.
  sed 's/ uic$//' $w/s_$nm.cir > $w/s0_$nm.cir; mode=ic
  (cd $w && timeout 600 ngspice -b s0_$nm.cir > s_$nm.log 2>&1 || true)
  if [ "$(wc -l < $w/$nm.dat 2>/dev/null || echo 0)" -lt 100 ]; then mode=uic; rm -f $w/$nm.dat; (cd $w && timeout 600 ngspice -b s_$nm.cir > s_$nm.log 2>&1 || true); fi
  if [ ! -s $w/$nm.dat ]; then printf "%s\t%s\t-\ttran failed\n" $e $bk; continue; fi
  awk -v E=$e -v B=$bk -v TSN=$tsn -v DTN=$dtn 'NR==1{next} {t=$1; y=$2; f=$4; if(t<TSN){y0=y; f0=f; next}
      if(!s && t>TSN+2*(DTN>1e-7?DTN:1e-7)){y1=y-y0; s=1}; if(s){d=y-y0; if(d<mn){mn=d; tm=t}; if(d>mx) mx=d; if(prev!="" && (prev-y0)*(d)<0) zc[++nz]=t; prev=y}}
    END{fa=f0<0?-f0:f0; if(fa>8.8){printf "%s\t%s\t%.3f\tsaturated (clamp)\t-\t-\t-\t-\n",E,B,f0; exit}
      os=(y1>0)? -mn/y1 : -mx/y1; if(os<0) os=0;   # overshoot past the pre-step value, opposite to the initial kick y1 if(os<1e-4){printf "%s\t%s\t%.3f\tlinear\t0\t>=1\t>=76\t-\n",E,B,f0; exit}
      l=log(os); z=-l/sqrt(3.14159265^2+l*l); pm=atan2(2*z, sqrt(sqrt(1+4*z^4)-2*z*z))*57.29578; fr=(nz>=2)? 1/(2*(zc[2]-zc[1])) : 0
      printf "%s\t%s\t%.3f\tlinear\t%.3g\t%.3f\t%.1f\t%.3g\n",E,B,f0,os,z,pm,fr}' $w/$nm.dat
done
[ "${FSOT_KEEP:-0}" = 1 ] && echo $w >&2 || rm -rf $w
