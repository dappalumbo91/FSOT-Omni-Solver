#!/usr/bin/env python3
# consts7.py <locked dir> <suffix>: summer constants (nepers from V_NEP) feeding each antilog, cascoded vs uncascoded, per netlist.
# |c| = R_SUM*10/R_const with R_SUM = 10k (T-network: R_eq = Ra + Rc + Ra Rc/Rb; strings: sum of pieces). Report only; no thresholds.
import sys, glob, os, re, statistics as st
d, sfx = sys.argv[1], sys.argv[2]
print("netlist\tn_casc\tmean_abs_c_casc\tmax_abs_c_casc\tn_unc\tmean_abs_c_unc\tmax_abs_c_unc")
for rf in sorted(glob.glob(f"{d}/recipe{sfx}_*.txt")):
    k = os.path.basename(rf)[len(f"recipe{sfx}_"):-4]
    P = {}
    for l in open(f"{d}/design{sfx}_{k}_values.tsv").readlines()[1:]:
        c = l.split("\t"); P[c[0]] = float(c[1])
    def rc(pref):
        n = "P_" + pref + "c"
        if n in P: return P[n]
        if n + "_ta" in P: a, b, cc = P[n + "_ta"], P[n + "_tb"], P[n + "_tc"]; return a + cc + a * cc / b
        s = [v for kk, v in P.items() if re.fullmatch(re.escape(n) + r"_\d+", kk)]
        return sum(s) if s else None
    cs, cu = [], []
    for l in open(rf):
        c = l.rstrip("\n").split("\t")
        if c[0] in ("CAL", "CALU"):
            r = rc(c[2][:-1]); v = 0.0 if r is None else 1e5 / r
            (cs if c[0] == "CAL" else cu).append(v)
    f = lambda x: (len(x), st.mean(x) if x else 0, max(x) if x else 0)
    a, b = f(cs), f(cu); print(f"{k}\t{a[0]}\t{a[1]:.3f}\t{a[2]:.3f}\t{b[0]}\t{b[1]:.3f}\t{b[2]:.3f}")
