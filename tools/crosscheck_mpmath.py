"""Python cross-check (mpmath, 50 digits) of the locked C++ targets. Re-implements engine.hpp:110-137 seeds/layers and
compute_scalar (engine.hpp:201-221) independently, plus the closed forms used in the slice. Analysis only."""
import os as _os
ROOT = _os.path.dirname(_os.path.dirname(_os.path.abspath(__file__)))
from mpmath import mp, mpf, pi, e, sqrt, log, exp, sin, cos, fabs
mp.dps = 50
T = {}
for l in open(ROOT + "/locked/fsot_targets.tsv"):
    if l[0] in "#n" or not l.strip(): continue
    n, v, _ = l.split("\t", 2); T[n] = mpf(v)
PHI = (1 + sqrt(5)) / 2; GAMMA = mpf("0.57721566490153286060651209008240243104215933593992"); G_CAT = mpf("0.91596559417721901505460351493238411077414937428167")
ALPHA = log(pi) / (e * PHI**13); PSI_CON = 1 - exp(-1); ETA_EFF = 1 / (pi - 1); BETA = 1 / exp(pi**pi + (e - 1)); GAMMA_C = -log(2) / PHI
OMEGA = sin(pi / e) * sqrt(2); THETA_S = sin(PSI_CON * ETA_EFF); POOF = exp((-log(pi) / e) / (ETA_EFF * log(PHI)))
C_EFF = (1 - POOF * sin(THETA_S)) * (1 + (1 / pi**4) * G_CAT / (pi * PHI)); A_BLEED = sin(pi / e) * PHI / sqrt(2); P_VAR = -cos(THETA_S + pi)
B_IN = C_EFF * (1 - sin(THETA_S) / PHI); A_IN = A_BLEED * (1 + cos(THETA_S) / PHI); SUCTION = POOF * (-cos(THETA_S - pi)); CHAOS = GAMMA_C / OMEGA
P_NEW = GAMMA / e * sqrt(2); C_FACTOR = C_EFF * P_NEW; K = PHI * (GAMMA / e) * sqrt(2) / log(pi) * (1 - 1 / pi**4)
def S(D, obs, dp=mpf(1), hits=0):
    N = P = mpf(1); D = mpf(D); dt = mpf(1); rho = 1
    growth = exp(ALPHA * (1 - hits / N) * GAMMA / PHI)
    base = (N * P / sqrt(D)) * cos((PSI_CON + dp) / ETA_EFF) * exp(-ALPHA * hits / N + rho + B_IN * dp) * (1 + growth * C_EFF)
    T1 = base * (1 + P_NEW * log(D / 25))
    if obs: T1 *= exp(C_FACTOR * P_VAR) * cos(dp + P_VAR)
    valve = BETA * cos(dp) * (N * P / sqrt(D)) * (1 + CHAOS * (D - 25) / 25) * (1 + POOF * cos(THETA_S + pi) + SUCTION * sin(THETA_S))
    T3 = valve * (1 + A_BLEED * sin(dt)**2 / PHI + A_IN * cos(dt)**2 / PHI) * (1 + B_IN * P_VAR)
    return K * (T1 + 1 + T3), K * (T1 + 1)
f = lambda D: (mpf(D) / 25) ** mpf(-0.5) * (1 + P_NEW * log(mpf(D) / 25))
sq, sq0 = S(5, True); sc, sc0 = S(6, True); sm, sm0 = S(25, False)
rows = [("K", K), ("P_new", P_NEW), ("psi_con", PSI_CON), ("S_quant", sq), ("S_chem", sc), ("S_cosm", sm), ("f_D5", f(5)), ("f_D6", f(6)), ("f_D25", f(25)),
        ("alpha_s(M_Z)", 1 / (e * pi)), ("1/alpha_em", e**3 * PHI**4 - PSI_CON), ("Omega_b_h2", fabs(sm) * (1 - sc)), ("m_H/m_W", sq * (1 + PSI_CON))]
print("quantity\tmpmath(50 digits)\tlocked C++ (long double)\trel diff")
for n, v in rows: print(f"{n}\t{mp.nstr(v, 16)}\t{mp.nstr(T[n], 16)}\t{mp.nstr((T[n] - v) / v, 3)}")
print("identity phi^4 = 3phi+2:", mp.nstr(PHI**4 - (3 * PHI + 2), 3))
print("T3 dropped (circuit) -> |S - K(T1+1)|/|S|:", mp.nstr(max(fabs((a - b) / a) for a, b in [(sq, sq0), (sc, sc0), (sm, sm0)]), 3))
print("D_eff(g) = round(5*5^(g/19)):", [int(mp.nint(5 * mpf(5) ** (mpf(g) / 19))) for g in range(20)])
