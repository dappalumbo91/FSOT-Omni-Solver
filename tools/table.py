"""Analysis only: results table (C++ vs ngspice nominal vs MC spread) and significant figures held vs measurement bars."""
import os as _os
ROOT = _os.path.dirname(_os.path.dirname(_os.path.abspath(__file__)))
import csv, math, subprocess
R = ROOT
mc = {}
for r in csv.DictReader(open(f"{R}/results/mc_summary.tsv"), delimiter="\t"): mc[(r["variant"], r["quantity"])] = r
o = subprocess.run(["../cpp/score", "out/slice_e24_tl082_s0_t27.log"], cwd=f"{R}/spice", capture_output=True, text=True).stdout.splitlines()
nom = dict(zip(o[0].split("\t"), o[1].split("\t")))
# measurement bar (relative 1 sigma). Observables: published; S/f rows: requirement derived from the downstream observable bar
bars = {"alpha_s(M_Z)": (0.0009 / 0.1180, "PDG 0.1180(9)"), "Omega_b_h2": (0.00015 / 0.02237, "Planck18 0.02237(15)"),
        "m_H/m_W": (0.0014 / 1.5578, "PDG24 ~1.5578(14)"), "1/alpha_em": (2.1e-8 / 137.035999177, "CODATA 137.035999177(21)"),
        "S_chem": (0.00015 / 0.02237 * 0.0447 / 0.955, "via Omega_b h2 (x21 gain)"), "S_quant": (0.0014 / 1.5578, "via m_H/m_W"),
        "S_cosm": (0.00015 / 0.02237, "via Omega_b h2 (|S_cosm| enters x1)"), "f_D5": (0.0014 / 1.5578 * 0.950 / 0.530, "via S_quant"),
        "f_D6": (0.00015 / 0.02237 * 0.0447 / 0.535, "via S_chem->Omega_b h2"), "f_D25": (0.00015 / 0.02237 * 0.502 / 0.922, "via S_cosm")}
rows = []
print("| quantity | C++ engine | ngspice nominal (E24+TL082) | nominal rel err | MC-A sd (cheap) | MC-B sd (precision) | MC-C sd (precision+matched) | MC-C bias | bar (rel) | sig figs held A/B/C | bar needs | meets bar? |")
print("|---|---|---|---|---|---|---|---|---|---|---|---|")
for q in ["S_quant", "S_chem", "S_cosm", "f_D5", "f_D6", "f_D25", "m_H/m_W", "Omega_b_h2", "alpha_s(M_Z)", "1/alpha_em"]:
    T = float(mc[("mcA", q)]["FSOT"]); n = float(nom[q]); ne = (n - T) / abs(T)
    sf = []
    for v in "ABC":
        r = mc[("mc" + v, q)]; b = (float(r["mean"]) - T) / abs(T); s = float(r["rel_sd"]); e = math.hypot(b, s)
        sf.append(max(0, math.floor(-math.log10(e))) if e > 0 else 99)
    bC = (float(mc[("mcC", q)]["mean"]) - T) / abs(T); eC = math.hypot(bC, float(mc[("mcC", q)]["rel_sd"]))
    bar, src = bars[q]; need = math.ceil(-math.log10(bar)); ok = "yes" if eC < bar else ("2-sigma no" if 2 * eC > bar and eC < bar else "no")
    ok = "yes (1 sd)" if eC < bar else "no"
    print(f"| {q} | {T:.10g} | {n:.7g} | {ne:+.2e} | {float(mc[('mcA', q)]['rel_sd']):.2e} | {float(mc[('mcB', q)]['rel_sd']):.2e} | {float(mc[('mcC', q)]['rel_sd']):.2e} | {bC:+.2e} | {bar:.1e} ({src}) | {sf[0]}/{sf[1]}/{sf[2]} | {need} | {ok} |")
