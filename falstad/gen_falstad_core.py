"""Falstad CircuitJS export of the core-law block (fold D fixed by one resistor) from the LOCKED values
(locked/design_values.tsv, realised E24 series sums). Same topology as cpp/netgen.cpp core()+sstage().
Falstad-only convergence aid: 1 kOhm isolation resistor in series with each log/antilog op-amp output (inside the loop, no DC effect;
Falstad op-amps are static and Newton diverges at t=0 without it). Prints ?ctz= URLs. Presentation only: no value is computed here except D-select resistor VR/(D*0.4 uA)."""
import csv, json, os
from lzstring_uri import compressToEncodedURIComponent
HERE = os.path.dirname(os.path.abspath(__file__)); BASE = "https://www.falstad.com/circuit/circuitjs.html"
P = {r["name"]: float(r["realised"]) for r in csv.DictReader(open(os.path.join(HERE, "..", "locked", "design_values.tsv")), delimiter="\t")}
T = {}
for l in open(os.path.join(HERE, "..", "locked", "fsot_targets.tsv")):
    if l.startswith("#") or l.startswith("name") or not l.strip(): continue
    n, v, _ = l.split("\t", 2); T[n] = float(v)
GAIN = float(os.environ.get("FGAIN", "100000")); RS = float(os.environ.get("FRS", "1000")); CAPS = os.environ.get("FCAPS", "1") == "1"
def build(D, obs, label, upto=9):
    L = ["$ 1 5e-8 10.20027730826997 50 5 50 5e-11"]
    def lab(x, y, n, dx=0, dy=16):
        L.append(f"g {x} {y} {x} {y+16} 0 0" if n == "GND" else f"207 {x} {y} {x+dx} {y+dy} 4 {n}")
    def R(x, y, a, b, val):  # vertical resistor, nodes a (top) b (bottom)
        L.append(f"r {x} {y} {x} {y+64} 0 {val!r}"); lab(x, y, a, 0, -16); lab(x, y + 64, b, 0, 16)
    def C(x, y, a, b, val):
        L.append(f"c {x} {y} {x} {y+64} 0 {val!r} 0 0.001"); lab(x, y, a, 0, -16); lab(x, y + 64, b, 0, 16)
    def OA(x, y, minus, plus, out, rs=0):  # (-) top, (+) bottom; rs: series output resistor (inside the loop)
        L.append(f"a {x} {y} {x+64} {y} 8 7.5 -7.5 1000000 0 0 {GAIN}")
        L.append(f"w {x} {y-16} {x-32} {y-16} 0"); lab(x - 32, y - 16, minus, -32, 0)
        L.append(f"w {x} {y+16} {x-32} {y+16} 0"); lab(x - 32, y + 16, plus, -32, 0)
        if rs: L.append(f"r {x+64} {y} {x+128} {y} 0 {rs}"); lab(x + 128, y, out, 32, 0)
        else: L.append(f"w {x+64} {y} {x+96} {y} 0"); lab(x + 96, y, out, 32, 0)
    def Q(x, y, c, b, e):  # NPN: base (x,y), collector (x+32,y-16), emitter (x+32,y+16)
        L.append(f"t {x} {y} {x+32} {y} 0 1 0 0 100 default")
        L.append(f"w {x} {y} {x-32} {y} 0"); lab(x - 32, y, b, -32, 0)
        L.append(f"w {x+32} {y-16} {x+32} {y-48} 0"); lab(x + 32, y - 48, c, 0, -16)
        L.append(f"w {x+32} {y+16} {x+32} {y+48} 0"); lab(x + 32, y + 48, e, 0, 16)
    # constant generator: VR and -VR rails
    L.append("R 48 48 48 16 0 0 40 2.5 0 0 0.5"); lab(48, 48, "vr")
    L.append("R 112 48 112 16 0 0 40 -2.5 0 0 0.5"); lab(112, 48, "vrn")
    # fold select: D units of 0.4 uA into LOGR1 Q2 (stands in for the comparator ladder)
    R(176, 32, "vr", "c2", 2.5 / (D * 0.4e-6))
    # LOGR1: u = V_T ln(D/25)
    R(240, 32, "vr", "c1", P["R_25"]); Q(336, 224, "c1", "GND", "e"); OA(304, 400, "c1", "GND", "e", RS)
    if CAPS: C(304, 32, "e", "c1", 1e-9); C(368, 32, "c2", "GND", 1e-9)
    Q(496, 224, "c2", "u", "e"); OA(528, 400, "GND", "c2", "u", RS)
    if upto < 2: return "\n".join(L) + "\n", 0
    # antilog: I0 (1 + P_new u) e^{-u/2}
    R(624, 32, "vr", "d1", P["R_I0"]); R(688, 32, "d1", "u", P["R_x"]); R(752, 32, "u", "b1", P["R_half"]); R(816, 32, "b1", "GND", P["R_half"])
    Q(656, 224, "d1", "b1", "ea"); Q(816, 224, "d2", "GND", "ea"); OA(752, 400, "d1", "GND", "ea", RS)
    if CAPS: C(880, 32, "ea", "d1", 1e-9)
    OA(976, 400, "d2", "GND", "vf"); R(944, 32, "vf", "d2", P["R_Tf"]); C(1008, 32, "vf", "d2", 1e-10)
    if upto < 3: return "\n".join(L) + "\n", 0
    # S = K + c f(D)
    if obs:
        OA(1200, 560, "nfi", "GND", "vfn"); R(1072, 32, "vf", "nfi", P["R_finv"]); R(1136, 32, "nfi", "vfn", P["R_finv"])
        R(1200, 32, "vfn", "ns", P["R_Sx_obs"])
    else:
        R(1200, 32, "vf", "ns", P["R_Sx_med"])
    R(1264, 32, "vrn", "ns", P["R_S0"]); R(1328, 32, "vS", "ns", P["R_Sf"]); OA(1200, 400, "ns", "GND", "vS")
    S = T["K"] + (T["c_obs"] if obs else T["c_med"]) * ((D / 25) ** -0.5 * (1 + T["P_new"] * __import__("math").log(D / 25)))
    L.append(f"x 176 640 368 643 4 14 FSOT\\score-law\\sblock:\\s{label}\\s(D_eff={D},\\s{'specimen' if obs else 'medium'})")
    L.append(f"x 176 672 368 675 4 14 vS\\s=\\s2\\sV\\sx\\sS;\\sFSOT\\sengine\\s(C\\splusplus):\\sS\\s=\\s{S:.6f}\\s->\\svS\\s=\\s{2*S:.5f}\\sV")
    return "\n".join(L) + "\n", 2 * S
out = {}
for name, D, obs, label in [("fsot_core_chem_D6", 6, True, "Chemistry"), ("fsot_core_cosm_D25", 25, False, "Cosmology"), ("fsot_core_qm_D5", 5, True, "Quantum_Mechanics")]:
    t, vexp = build(D, obs, label); open(os.path.join(HERE, name + ".txt"), "w").write(t)
    url = BASE + "?ctz=" + compressToEncodedURIComponent(t); open(os.path.join(HERE, name + ".url"), "w").write(url + "\n")
    out[name] = {"url": url, "expected_vS": vexp}
json.dump(out, open(os.path.join(HERE, "urls.json"), "w"), indent=1)
print(json.dumps({k: (len(v["url"]), v["expected_vS"]) for k, v in out.items()}))
