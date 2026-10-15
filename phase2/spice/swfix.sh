#!/usr/bin/env bash
# swfix.sh <in.cir> <solution.ns> > out.cir : small-signal (ac/noise) netlist with every voltage-controlled switch replaced by its
# resolved resistor (RON or ROFF of its own .model) at the operating point. ngspice's SW noise model reports both poles of a
# CD4053 SPDT as if open (1e12 Ohm thermal noise on the common node), so the switch must be resolved before .noise.
awk -v NS="$2" 'BEGIN{while((getline l<NS)>0){split(l,a," "); v[a[1]]=a[3]} v["0"]=0}
/^\.model SW/{m=tolower($2); match($0,/RON=[^ )]+/); ron[m]=substr($0,RSTART+4,RLENGTH-4); match($0,/ROFF=[^ )]+/); roff[m]=substr($0,RSTART+5,RLENGTH-5); print; next}
{L[++n]=$0} END{for(i=1;i<=n;i++){ $0=L[i]; if($1 ~ /^[Ss]/){ c=v[tolower($4)]-v[tolower($5)]; m=tolower($6); printf "R%s %s %s %s\n", $1, $2, $3, (c>0?ron[m]:roff[m]) } else print }}' "$1"
