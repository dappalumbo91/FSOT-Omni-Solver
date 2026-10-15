#!/usr/bin/env bash
# score7.sh <bs log> <netlist key> [locked7 dir] (fork of score5.sh; LOCK7 file names; GENpp scored against generator7 row BETA_pp): each compiled leaf (or, for GEN, each derived constant) = V(net)/gain,
# rel error vs the golden value (GEN: vs the engine constant).
set -euo pipefail; LOG=$1; K=$2; D=${3:-$(dirname "$0")/../locked7}
if [ "$K" = GEN ] || [ "$K" = GENpp ]; then T=$D/generator7.tsv; else T=$D/leaves7_targets.tsv; fi
awk -F'\t' -v K="$K" 'NR==FNR { if ($0 ~ /^v\(/) { split($0, a, " = "); n=a[1]; gsub(/^v\(|\)$/, "", n); v[tolower(n)] = a[2] + 0 } next }
FNR==1 { print "id\tname\tcircuit\ttarget\trel"; next }
{ if (K == "GEN" || K == "GENpp") { id=$1; nm=$1; tg=$2; net=$3; g=$4; if (K == "GENpp") { if (net !~ /^GENpp:/) next; sub(/^GENpp:/, "", net) } else if (net ~ /^GENpp:/) next } else { if ($5 != K) next; id=$1; nm=$3; tg=$7; net=$8; g=$9 }
  n=tolower(net); if (!(n in v)) { printf "%s\t%s\tMISSING\t%s\t-\n", id, nm, tg; miss++; next }
  c = v[n]/g; r = (tg == 0) ? c : (c - tg)/tg; ar = r < 0 ? -r : r; if (ar > w) { w = ar; wn = nm }
  printf "%s\t%s\t%.10g\t%.10g\t%+.3e\n", id, nm, c, tg, r; cnt++ }
END { printf "summary\t%s\tn=%d\tmissing=%d\tworst=%.3e\t%s\n", K, cnt, miss, w, wn }' "$LOG" "$T"
