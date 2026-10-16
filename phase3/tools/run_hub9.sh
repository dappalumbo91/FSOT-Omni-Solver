#!/usr/bin/env bash
# run_hub9.sh : LOCK9H 0-D hub relaxation bench. Builds hubbench (detmath), generates the compiled- and exact-coefficient transient netlists
# from phase3/locked9h, runs ngspice (batch), the Python reference (tools/hubpy.py, env FSOT_VENDOR) and the comparison (tools/hubcmp.py).
# Outputs: phase2/spice/out/hub9_{compiled,exact}.{cir,dat,log}, phase3/results/hub9_compare.tsv, hub9_summary.tsv
set -e; cd "$(dirname "$0")/.."; ENG=${FSOT_ENG:-/tmp/eng3466}; F="-std=c++20 -O2 -ffp-contract=off"; t=$(mktemp -d)
g++ $F -fno-builtin -c cpp/detmath.cpp -o $t/detmath.o; g++ $F -I"$ENG/include" -o cpp/hubbench cpp/hubbench.cpp $t/detmath.o
cpp/hubbench values $t >/dev/null; cmp $t/hubbench_values.tsv locked9h/hubbench_values.tsv; cmp $t/hubref_cpp.tsv locked9h/hubref_cpp.tsv
O=../phase2/spice/out
for v in compiled exact; do cpp/hubbench net locked9h/hubbench_values.tsv locked9h/hub_stage1.tsv $v $O/hub9_$v.cir $O/hub9_$v.dat >/dev/null
  ngspice -b $O/hub9_$v.cir > $O/hub9_$v.log 2>&1; done
python3 tools/hubpy.py $t/hubref_py.tsv >/dev/null
python3 tools/hubcmp.py locked9h $t/hubref_py.tsv $O/hub9_compiled.dat $O/hub9_exact.dat results; rm -rf $t
