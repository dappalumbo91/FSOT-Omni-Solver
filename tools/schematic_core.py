"""Schematic PNG of the FSOT core-law block (one fold channel). Values read from locked/design_values.tsv (realised E24 strings)."""
import os as _os
ROOT = _os.path.dirname(_os.path.dirname(_os.path.abspath(__file__)))
import csv, schemdraw, schemdraw.elements as e
P = {r["name"]: r for r in csv.DictReader(open(ROOT + "/locked/design_values.tsv"), delimiter="\t")}
def v(n):
    x = float(P[n]["realised"]); return (f"{x/1e6:.4g} MΩ" if x >= 1e6 else f"{x/1e3:.4g} kΩ" if x >= 1e3 else f"{x:.4g} Ω")
schemdraw.config(fontsize=9, unit=2.5)
d = schemdraw.Drawing(show=False)
def W(*pts):
    for a, b in zip(pts, pts[1:]): d.add(e.Line().at(a).to(b))
def dot(p, t="", loc="top"): d.add(e.Dot().at(p).label(t, loc=loc))
def gnd(p, dx=-0.5): d.add(e.Line().at(p).right(dx) if dx > 0 else e.Line().at(p).left(-dx)); d.add(e.Ground())
# ---------- Stage 1: LOGR1, u = V_T ln(I_D / I_25) ----------
Q1 = d.add(e.BjtNpn(circle=True).right().anchor("base").at((2, 0)).label("Q1", loc="left"))
Q2 = d.add(e.BjtNpn(circle=True).right().reverse().anchor("base").at((6, 0)).label("Q2", loc="right"))
gnd(Q1.base, -0.6)
W(Q1.emitter, (Q1.emitter[0], -1.5), (Q2.emitter[0], -1.5), Q2.emitter); dot((4, -1.5), "e", "bottom")
c1 = (Q1.collector[0], 1.8); c2 = (Q2.collector[0], 1.8); W(Q1.collector, c1); W(Q2.collector, c2)
dot(c1, "c1", "left"); dot(c2, "c2", "right")
d.add(e.Resistor().at(c1).up().label("R_25\n" + v("R_25") + "\nI_25 = 10 µA", loc="top")); d.add(e.Vdd().label("VR"))
d.add(e.SourceI().at((c2[0], 4.3)).down().reverse().label("I_D = D_eff × 0.4 µA\n(fold ladder)", loc="right")); d.add(e.Vdd().at((c2[0], 4.3)))
B = d.add(e.Opamp(leads=True).anchor("in2").at((7.4, 1.8)).right().label("B", loc="center"))
W(c2, B.in2); gnd(B.in1, -0.5)
u = (B.out[0] + 0.8, B.out[1]); W(B.out, u); dot(u, "u = V_T·ln(D/25)", "top")
W(u, (u[0], 0), Q2.base)
A = d.add(e.Opamp(leads=True).anchor("out").at((4, -3.6)).right().label("A", loc="center"))
W((4, -1.5), A.out); gnd(A.in2, -0.5); W(A.in1, (0, A.in1[1]), (0, 1.8), c1)
# ---------- Stage 2: antilog, I = I0 (1 + P_new u/V_T·V_T) exp(-u/(2 V_T)) ----------
ux = (14.5, 0); W((u[0], 0), ux); dot(ux)
d.add(e.Resistor().at(ux).up().label("R_x " + v("R_x") + "\n= V_T/(P_new·I0)", loc="top")); d1 = d.here; dot(d1, "d1", "right")
d.add(e.Resistor().at(d1).up().label("R_I0\n" + v("R_I0"), loc="top")); d.add(e.Vdd().label("VR"))
d.add(e.Resistor().at(ux).down().label("R_half\n" + v("R_half"), loc="bottom")); b1 = d.here; dot(b1, "u/2", "left")
d.add(e.Resistor().at(b1).down().label("R_half\n" + v("R_half"), loc="bottom")); d.add(e.Ground())
Q3 = d.add(e.BjtNpn(circle=True).right().anchor("base").at((16.7, b1[1])).label("Q3", loc="left"))
Q4 = d.add(e.BjtNpn(circle=True).right().reverse().anchor("base").at((20.9, b1[1])).label("Q4", loc="right"))
gnd(Q4.base, 0.6); W(b1, Q3.base)
W(Q3.collector, (Q3.collector[0], d1[1]), d1)
W(Q3.emitter, (Q3.emitter[0], -4.2), (Q4.emitter[0], -4.2), Q4.emitter); xa = (Q3.emitter[0] + Q4.emitter[0]) / 2; dot((xa, -4.2), "ea", "bottom")
C = d.add(e.Opamp(leads=True).anchor("out").at((xa + 1.6, -6.8)).right().label("C", loc="center"))
W((xa, -4.2), (xa + 1.6, -4.2), C.out); gnd(C.in2, -0.5)
W(C.in1, (15.9, C.in1[1]), (15.9, d1[1]))
d2 = (Q4.collector[0], d1[1]); W(Q4.collector, d2); dot(d2, "d2:  I0·f(D)", "top")
# ---------- Stage 3: TIA, vf = R_Tf I0 f = 1 V × f(D) ----------
T = d.add(e.Opamp(leads=True).anchor("in1").at((d2[0] + 2.0, d2[1])).right().label("T", loc="center"))
W(d2, T.in1); gnd(T.in2, -0.5)
vf = (T.out[0] + 0.8, T.out[1]); W(T.out, vf); dot(vf, "vf = 1 V × f(D)", "bottom")
x0 = d2[0] + 1.2; W((x0, d2[1]), (x0, 5.0)); d.add(e.Resistor().at((x0, 5.0)).to((vf[0], 5.0)).label("R_Tf " + v("R_Tf"), loc="top")); W((vf[0], 5.0), vf)
# ---------- Stage 4: S = K + c f(D), vS = 2 V × S ----------
ns = (vf[0] + 3.6, vf[1]); d.add(e.Resistor().at(vf).to(ns).label("R_Sx: " + v("R_Sx_med") + " medium (c<0)\nor " + v("R_Sx_obs") + " obs, after ×(−1)", loc="top")); dot(ns)
S = d.add(e.Opamp(leads=True).anchor("in1").at((ns[0] + 1.8, ns[1])).right().label("S", loc="center"))
W(ns, S.in1); gnd(S.in2, -0.5)
vs = (S.out[0] + 0.8, S.out[1]); W(S.out, vs); dot(vs, "vS = 2 V × S\nS = K + c·f(D)", "right")
W(ns, (ns[0], 5.0)); d.add(e.Resistor().at((ns[0], 5.0)).to((vs[0], 5.0)).label("R_Sf " + v("R_Sf"), loc="top")); W((vs[0], 5.0), vs)
d.add(e.Resistor().at(ns).down().label("R_S0\n" + v("R_S0") + "\n(K term)", loc="top")); d.add(e.Vss().label("−VR", loc="bottom"))
d.add(e.Label().at((-0.5, -10.2)).label(
    "FSOT core law, one fold channel:  S = K + c·f(D),   f(D) = (D/25)^(−1/2)·(1 + P_new·ln(D/25)),   c_med = K·A/5,   c_obs = c_med·O     (T2 = 1;  |T3/S| ≤ 1.3e-17 omitted)\n"
    "Fold ladder (not drawn): 20 LM339 comparators on the g-string (0.125 V/generation), thresholds g_k = 19·log5((k+½)/5), each steering 0.4 µA (R_step " + v("R_step") + " from VR) via CD4053 into c2  ⇒  I_D = round(5·5^(g/19)) × 0.4 µA.\n"
    "Pure analog: TL082 op-amps A, B, C, T, S;  2N3904 matched pairs Q1/Q2, Q3/Q4;  trimmed VR = 2.5 V;  ±9 V rails.   Compensation (not drawn): 1 nF e→c1, 1 nF c2→GND, 1 nF ea→d1, 100 pF across R_Tf.\n"
    "All component values derived in C++20 from the FSOT engine and sha256-locked (locked/LOCK.md) before simulation.",
    loc="right", halign="left"))
d.save(ROOT + "/png/core_block_schematic.png", dpi=140)
