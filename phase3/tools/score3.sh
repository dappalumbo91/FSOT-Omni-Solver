#!/usr/bin/env bash
# score3.sh <bs_leaf1 log> [targets.tsv]: leaf value = V(vL<id>)/G; reports rel error vs golden, and the leaf block's own
# error given the circuit's own S signals (value recomputed from b + k * x_measured) so upstream S error and leaf-block error separate.
set -euo pipefail
LOG=$1; T=${2:-$(dirname "$0")/../locked/leaves3_targets.tsv}
awk -F'\t' -v OFS='\t' 'NR==FNR { if ($0 ~ /^v\(/) { split($0, a, " = "); n=a[1]; gsub(/^v\(|\)$/, "", n); v[tolower(n)] = a[2] + 0 } next }
FNR==1 { print "id","name","circuit","golden","rel_vs_golden","x_meas","block_rel"; next }
{ id=$1; if (id == "Omb") { c = v["vomb"]/100; x = "-"; br = "-" }
  else { c = v["vl" tolower(id)] / $9; xm = v[tolower($7)] / $8; e = $11 + $12 * xm; x = sprintf("%.9g", xm); br = sprintf("%.2e", (c - e)/e) }
  r = (c - $4)/$4; if (r < 0) ar = -r; else ar = r; if (ar > w) w = ar
  printf "%s\t%s\t%.9g\t%.9g\t%+.2e\t%s\t%s\n", id, $2, c, $4, r, x, br }
END { printf "worst_abs_rel_vs_golden\t%.2e\n", w }' "$LOG" "$T"
