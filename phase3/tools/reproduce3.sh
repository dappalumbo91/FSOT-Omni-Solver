#!/usr/bin/env bash
# reproduce3.sh <FSOT-2.1-Cpp checkout at 3466e8d>: Phase 3 leaf tier 1 (CI). Needs phase2 tools built (reproduce2.sh).
#  verify LOCK3 -> leaf1 block solve at 27 C and 0 C -> every leaf block within 2e-5 of its own inputs, every leaf within 5e-3 of golden.
set -euo pipefail; cd "$(dirname "$0")/.."; ENG=${1:?path to FSOT-2.1-Cpp at 3466e8d}
tools/verify_lock3.sh "$ENG/include" "$ENG/golden/golden_AEB2AD.tsv" | tail -1
fail=0
for t in 27 0; do (cd ../phase2/spice && mkdir -p out && ./run_bs.sh leaf1 ideal_ideal 0 $t)
  L=../phase2/spice/out/bs_leaf1_ideal_ideal_s0_t$t.log
  grep -q "SOLVER: blocks" $L && echo "PASS converged leaf1 ${t}C" || { echo "FAIL leaf1 ${t}C: $(tail -1 $L)"; fail=1; continue; }
  tools/score3.sh $L | tee /tmp/score3_t$t.tsv
  awk -F'\t' -v t=$t 'NR>1 && $1!~/worst/ { g=$5+0; if (g<0) g=-g; b=($7=="-")?0:$7+0; if (b<0) b=-b
    if (g>5e-3 || b>2e-5) { print "FAIL " $2 " " t "C rel_vs_golden=" $5 " block_rel=" $7; bad=1 } } END { exit bad }' /tmp/score3_t$t.tsv \
    && echo "PASS leaf tier 1 ${t}C (|rel vs golden| <= 5e-3, |block rel| <= 2e-5)" || fail=1; done
[ $fail = 0 ] && echo REPRODUCE3_OK || { echo REPRODUCE3_FAIL; exit 1; }
