#!/usr/bin/env bash
# mcstats.sh <class>: mean / sd / worst |rel| of the MC result tables; Omega_b h^2 bar pass rate (|rel| <= 6.7e-3); D exact rate.
cd "$(dirname "$0")/../results"; c=$1
st() { awk -F'\t' -v c=$2 -v nm="$3" -v bar="${4:-0}" '$c!="nan" && $c!="inf" && $c!="" {x=$c+0; n++; s+=x; ss+=x*x; a=x<0?-x:x; if(a>w)w=a; if(bar>0 && a<=bar)p++} $c=="nan"||$c=="inf"{f++}
  END{m=s/n; printf "%-28s n=%d fail=%d mean=%+.3e sd=%.3e worst|rel|=%.3e%s\n", nm, n, f+0, m, sqrt(ss/n-m*m), w, (bar>0?sprintf(" pass(|rel|<=%g)=%d/%d",bar,p,n):"")}' $1; }
f=mc_omb_${c}_t27.tsv; st $f 4 "S_chem" ; st $f 6 "S_cosm"; st $f 8 "1-S_chem"; st $f 10 "Omega_b h^2" 6.7e-3
for fo in Atomic_Physics High_Energy_Physics Quantum_Mechanics Fluid_Dynamics; do f=mc_fold_${c}_t27_$fo.tsv; st $f 6 "S($fo)"; st $f 12 "lnT3 diff($fo) [Np]"
  awk -F'\t' 'int($13+0.5)==int($14+0.5){d++} END{printf "%-28s D exact %d/%d\n", "D('"$fo"')", d, NR}' $f; done
