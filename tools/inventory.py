"""Inventory of the FSOT C++ engine closed forms (analysis only, not derivation): classify every row of
closed_forms.gen.inc by the analog operation classes it needs. Output: results/inventory.tsv + summary."""
import os as _os
ROOT = _os.path.dirname(_os.path.dirname(_os.path.abspath(__file__)))
import re, collections, json
SRC = _os.environ.get("FSOT_CPP_DIR", ROOT + "/third_party/FSOT-2.1-Cpp") + "/include/fsot/closed_forms.gen.inc"
L = open(SRC).read().splitlines()
rows = []; sect = None; pending = None
for i, l in enumerate(L, 1):
    m = re.match(r"\s*Results (\w+)\(\) const", l)
    if m: sect = m.group(1)
    for mm in re.finditer(r'mk\("([^"]+)"', l):
        # expression: the mk third arg, or the preceding "v = ..." line(s)
        expr = l
        if re.search(r'mk\("[^"]+", "[^"]*", v[,)]', l) or re.search(r'mk\("[^"]+", "[^"]*", v1|v2|v3|consistency|t\)', l):
            j = i - 2
            while j >= 0 and not re.match(r"\s*(R )?v\d? = ", L[j]) and "const R" not in L[j]: j -= 1
            expr = L[j] + " " + l
        rows.append((sect, mm.group(1), i, expr))
LAYER = ["ALPHA", "PSI_CON", "ETA_EFF", "BETA", "GAMMA_C", "OMEGA", "THETA_S", "POOF", "C_EFF", "A_BLEED", "P_VAR", "B_IN", "A_IN",
         "SUCTION", "CHAOS", "P_BASE", "P_NEW", "C_FACTOR", "K", "C_COSM"]
def classify(e):
    c = set()
    if re.search(r"S_COSM|S_QUANT|S_CHEM|domain_scalar|derived_D_eff", e): c.add("fold(S_*)")
    if re.search(r"m::floor|nearbyint", e): c.add("rounding")
    if re.search(r"m::(exp|ln)\(", e): c.add("exp/log")
    if re.search(r"m::(cos|sin|acos)\(", e): c.add("trig")
    if re.search(r"m::pow\(", e): c.add("real power")
    if re.search(r"m::sqrt\(", e): c.add("sqrt")
    if re.search(r"m::ipow\(", e): c.add("integer power")
    if re.search(r"m::fabs\(", e): c.add("abs")
    if any(re.search(r"\b" + k + r"\b", e) for k in LAYER): c.add("layer const")
    if re.search(r"lit<R>\(\"0\.529177\"\)|a0", e): c.add("measured input")
    return c
out = []; cnt = collections.Counter(); fold_rows = []
for sect, name, line, e in rows:
    c = classify(e); out.append((sect, name, line, ",".join(sorted(c)) or "rational", e.strip()[:160]))
    for k in c or {"rational"}: cnt[k] += 1
    if "fold(S_*)" in c: fold_rows.append(f"{name} ({sect}, closed_forms.gen.inc:{line})")
with open("../results/inventory.tsv", "w") as f:
    f.write("section\trow\tline\tclasses\texpr\n")
    for r in out: f.write("\t".join(map(str, r)) + "\n")
mono = collections.Counter(); 
for r in out:
    cl = set(r[3].split(","))
    if "fold(S_*)" in cl: mono["needs fold (core law S)"] += 1
    elif "rounding" in cl: mono["needs rounding (floor)"] += 1
    elif cl & {"exp/log", "trig", "real power", "sqrt"}: mono["seed algebra with transcendental ops"] += 1
    else: mono["seed algebra, rational + integer powers only"] += 1
print(json.dumps({"rows": len(out), "class_counts": cnt, "tiers": mono, "fold_rows": fold_rows}, indent=1))
