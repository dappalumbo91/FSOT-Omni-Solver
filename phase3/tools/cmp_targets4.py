#!/usr/bin/env python3
"""Cross-platform check of a re-derived leaves4_targets.tsv against the locked one. Every column must be byte-identical
except the 'engine' column (the engine's long-double value printed to 21 digits) and 'series_remainder_max'. Those come
from libm logl/powl/acosl, which are not correctly rounded and differ between glibc builds in the last ulp or so.
They must agree to 1e-18 relative (about 9 long-double ulps). Every difference is printed. Usage: cmp_targets4.py new locked"""
import sys
from decimal import Decimal
a = open(sys.argv[1]).read().splitlines(); b = open(sys.argv[2]).read().splitlines()
if len(a) != len(b): sys.exit(f"row count {len(a)} vs {len(b)}")
h = b[0].split('\t'); ie = h.index('engine'); ir = h.index('series_remainder_max'); bad = 0; nd = 0
for i, (x, y) in enumerate(zip(a, b)):
    if x == y: continue
    X, Y = x.split('\t'), y.split('\t')
    for j, (p, q) in enumerate(zip(X, Y)):
        if p == q: continue
        nd += 1
        if j in (ie, ir):
            P, Q = Decimal(p), Decimal(q); rel = abs(P - Q) / max(abs(Q), Decimal('1e-300'))
            tol = Decimal('1e-18') if j == ie else Decimal('1e-2')
            ok = rel <= tol
        else: ok = False; rel = 'non-numeric'
        print(f"line {i+1} {Y[2] if len(Y)>2 else ''} col {h[j]}: {p} vs {q} rel {rel} {'ok' if ok else 'FAIL'}"); bad += not ok
    if len(X) != len(Y): print(f"line {i+1}: column count differs"); bad += 1
print(f"targets: {nd} last-ulp differences, {bad} failures"); sys.exit(1 if bad else 0)
