#!/usr/bin/env bash
# temp_walk.sh <fold> <obs> <T1> <T2> ...: solver aid only. Converged ideal-op solution at 27 C is walked in temperature
# steps; each step's node voltages are the .nodeset initial guess of the next. Writes out/seed_<obs>_ideal_ideal_t<T>_<fold>.ns
set -e; cd "$(dirname "$0")"; f=$1; o=$2; shift 2; prev=${FSOT_TW_START:-out/seed_${o}_ideal_ideal_t27.ns}
for t in "$@"; do FSOT_P2_NODESET=$prev ../cpp/netgen2 fold ideal_ideal 0 netlists/tw.cir $t opdump $f
  timeout 900 ngspice -b netlists/tw.cir 2>&1 | grep -E "^[a-z0-9_.]+ += " > out/seed_${o}_ideal_ideal_t${t}_$f.ns || true
  n=$(wc -l < out/seed_${o}_ideal_ideal_t${t}_$f.ns); echo "$f T=$t nodes=$n"; [ "$n" -gt 0 ] && prev=out/seed_${o}_ideal_ideal_t${t}_$f.ns; done
