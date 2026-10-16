#!/usr/bin/env bash
# make_seed.sh <variant> <temp> <fold> <obs 0|1>: converged .op of one fold -> all node voltages (solver initial guess)
set -e; cd "$(dirname "$0")"; v=$1; t=$2; f=$3; o=$4
../cpp/netgen2 fold $v 0 netlists/seed.cir $t opdump $f
timeout 900 ngspice -b netlists/seed.cir 2>&1 | grep -E "^[a-z0-9_.]+ += " > out/seed_${o}_${v}_t${t}.ns || true
rm -f netlists/seed.cir; wc -l out/seed_${o}_${v}_t${t}.ns
