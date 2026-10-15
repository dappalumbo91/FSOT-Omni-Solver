"""Compare a regenerated nominal ngspice score table against the committed one (rel tol 2e-4: ngspice versions differ slightly)."""
import sys, csv, math
def load(p): return {r["log"]: r for r in csv.DictReader(open(p), delimiter="\t")}
a, b = load(sys.argv[1]), load(sys.argv[2]); tol = float(sys.argv[3]) if len(sys.argv) > 3 else 2e-4; bad = 0
for k, ra in a.items():
    rb = b.get(k)
    if rb is None: print("missing", k); bad += 1; continue
    for c, va in ra.items():
        if c in ("log", "ripple_max_V") or va in ("", "nan"): continue
        x, y = float(va), float(rb[c]); d = abs(x - y) / max(abs(x), 1e-12)
        if d > tol: print(f"{k} {c}: committed {x} regenerated {y} rel {d:.2e}"); bad += 1
print("NOMINAL_MATCH" if not bad else f"NOMINAL_MISMATCH {bad}"); sys.exit(1 if bad else 0)
