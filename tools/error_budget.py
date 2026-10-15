"""First-order analytic error budget for S_chem (D=6, observed channel), per tolerance class, from the block equations
(f = I_out/I0 with I_out = I_Q3 exp(-u/2V_T), I_Q3 = I0 + u/R_x, vf = R_Tf I_out, S = (R_Sf/R_Sx) vf + (R_Sf/R_S0) VR/2... in units).
Analysis only; compared against the MC sd in results/mc_summary.tsv."""
import os as _os
ROOT = _os.path.dirname(_os.path.dirname(_os.path.abspath(__file__)))
import math, csv
VT = 0.0258649; I0 = 10e-6; P = 0.3003022766703715; D = 6; K = 0.4201087636498879; c = 0.4588176410079425
L = math.log(D / 25); f = (D / 25) ** -0.5 * (1 + P * L); S = K + c * f
Rx = VT / (P * I0); IQ3 = I0 * (1 + P * L)
dlnf_du = (-0.5 + P / (1 + P * L)) / VT          # per volt of error in u
classes = {"A (TL082, 1%, unmatched 2N3904)": dict(tol=0.01, vos=3e-3, vbe=1e-3, vr=0.005),
           "B (chopper op-amp, 0.1%, matched pairs)": dict(tol=0.001, vos=25e-6, vbe=1e-4, vr=0.001),
           "C (B + 0.02% / identity trims)": dict(tol=0.0002, vos=2e-6, vbe=1e-5, vr=0.0005)}
sf, cf_S, K_S = 1.0, c * f / S, K / S                # d ln S per d ln(R_Sf); per d ln(c f); per d ln K-term
out = open(ROOT + "/results/error_budget_S_chem.tsv", "w")
out.write("class\tterm\tmechanism\trel_sigma_in_S\n")
for name, k in classes.items():
    su = k["tol"] / math.sqrt(3); svr = k["vr"] / math.sqrt(3); spair = math.sqrt(2) * k["vbe"]
    terms = [
        ("BJT pair Q3/Q4 dVbe", "exp(dV/V_T) in antilog", cf_S * spair / VT),
        ("BJT pair Q1/Q2 dVbe", "u error x dlnf/du (near-cancels at D=6)", cf_S * abs(dlnf_du) * spair),
        ("Vos at d1 (op-amp C)", "Vos/R_x into I_Q3", cf_S * k["vos"] / Rx / IQ3),
        ("Vos op-amp T", "TIA output offset", cf_S * k["vos"] / 1.0 / f),
        ("Vos op-amp S", "noise gain 2.25 / (2 V per unit)", 2.25 * k["vos"] / 2.0 / S),
        ("Vos inverter FI", "x2 into c f term", cf_S * 2 * k["vos"] / f),
        ("R_I0", "sets I0 of Q3 (not of the ratio)", cf_S * su * I0 / IQ3),
        ("R_x", "P_new u term", cf_S * su * abs(I0 * P * L) / IQ3),
        ("R_Tf", "TIA gain", cf_S * su),
        ("R_Sx, R_S0, R_Sf, R_finv x2", "summer ratios", math.sqrt((cf_S * su) ** 2 + (K_S * su) ** 2 + su ** 2 + 2 * (cf_S * su) ** 2)),
        ("VR", "I0 and K term", math.hypot(cf_S * svr, K_S * svr)),
    ]
    tot = math.sqrt(sum(t[2] ** 2 for t in terms))
    for t in terms: out.write(f"{name}\t{t[0]}\t{t[1]}\t{t[2]:.2e}\n")
    out.write(f"{name}\tRSS\t\t{tot:.2e}\n")
    print(name, "RSS", f"{tot:.2e}", "top:", ", ".join(f"{t[0]} {t[2]:.1e}" for t in sorted(terms, key=lambda t: -t[2])[:3]))
