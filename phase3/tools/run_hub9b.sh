#!/usr/bin/env bash
# run_hub9b.sh : LOCK9H Amendment 9H.1 0-D hub relaxation bench. Builds hubbench2 (detmath), generates the compiled- and exact-coefficient transient netlists
# from phase3/locked9h1, runs ngspice (batch), the Python reference (tools/hubpy.py, env FSOT_VENDOR) and the comparison (tools/hubcmp.py).
# Outputs: phase2/spice/out/hub9b_{compiled,exact}.{cir,dat,log}, phase3/results/hub9b_compare.tsv, hub9b_summary.tsv
set -e; cd "$(dirname "$0")/.."; ENG=${FSOT_ENG:-/tmp/eng3466}; F="-std=c++20 -O2 -ffp-contract=off"; t=$(mktemp -d)
g++ $F -fno-builtin -c cpp/detmath.cpp -o $t/detmath.o; g++ $F -I"$ENG/include" -o cpp/hubbench2 cpp/hubbench2.cpp $t/detmath.o
cpp/hubbench2 values $t >/dev/null; cmp $t/hubbench_values.tsv locked9h1/hubbench_values.tsv; cmp $t/hubref_cpp.tsv locked9h1/hubref_cpp.tsv
O=../phase2/spice/out
for v in compiled exact; do cpp/hubbench2 net locked9h1/hubbench_values.tsv locked9h1/hub_stage1.tsv $v $O/hub9b_$v.cir $O/hub9b_$v.dat >/dev/null
  ngspice -b $O/hub9b_$v.cir > $O/hub9b_$v.log 2>&1; done
python3 tools/hubpy.py $t/hubref_py.tsv >/dev/null
python3 tools/hubcmp.py locked9h1 $t/hubref_py.tsv $O/hub9b_compiled.dat $O/hub9b_exact.dat $t && cp $t/hub9_compare.tsv results/hub9b_compare.tsv && cp $t/hub9_summary.tsv results/hub9b_summary.tsv; rm -rf $t
