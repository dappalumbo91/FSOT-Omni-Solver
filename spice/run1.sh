#!/usr/bin/env bash
# run1.sh <variant> <seed> [temp]: ideal-op dump (solver initial guess) -> TL082 .op with nodeset. Output out/<tag>.log
# Harness amendment after lock (no value or topology change): 60 s timeout per ngspice call; if the nodeset .op does not
# converge, one retry without nodeset; the attempt used is recorded as "SOLVER: ..." at the end of the log.
set -e; cd "$(dirname "$0")"; v=$1; s=$2; t=${3:-27}; tag=slice_${v}_s${s}_t${t}
ok() { grep -q "v(vsq) = " out/$tag.log && ! grep -q "failed" out/$tag.log; }
../cpp/netgen slice $v $s netlists/$tag.dump.cir $t dump
timeout 60 ngspice -b netlists/$tag.dump.cir 2>&1 | grep -E "^[a-z0-9_.]+ = " | grep -v "#" > out/$tag.ns || true
if [ "$v" = "ideal_ideal" ]; then ../cpp/netgen slice $v $s netlists/$tag.cir $t op; else ../cpp/netgen slice $v $s netlists/$tag.cir $t op out/$tag.ns; fi
timeout 60 ngspice -b netlists/$tag.cir > out/$tag.log 2>&1 || true
if ok; then echo "SOLVER: nodeset" >> out/$tag.log; else
  ../cpp/netgen slice $v $s netlists/$tag.cir $t op; timeout 60 ngspice -b netlists/$tag.cir > out/$tag.log 2>&1 || true
  if ok; then echo "SOLVER: plain" >> out/$tag.log; else echo "SOLVER: FAILED" >> out/$tag.log; fi
fi
rm -f netlists/$tag.dump.cir out/$tag.ns
[ "$s" -gt 0 ] && rm -f netlists/$tag.cir || true
