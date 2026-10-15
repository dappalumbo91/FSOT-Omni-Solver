// design.hpp — FSOT analog-solver Phase 1: FSOT targets and every component value, derived in C++20
// from the pinned engine (FSOT-2.1-Cpp @3466e8d, include/fsot/engine.hpp, closed_forms.gen.inc).
// No FSOT rule is added or changed. Circuit scale choices (reference, unit currents, volts-per-unit)
// are design units, fixed here and locked with everything else BEFORE any simulation.
#pragma once
#include "fsot/engine.hpp"
#include <array>
#include <cmath>
#include <string>
#include <vector>

namespace ckt {
using R = long double;
using Eng = fsot::Engine<R>;

// ---- standard physics (SI 2019 exact) -------------------------------------------------------------
inline constexpr R kB = 1.380649e-23L, qe = 1.602176634e-19L, T0 = 300.15L;  // 27 C = SPICE TNOM
inline const R VT0 = kB * T0 / qe;                                              // thermal voltage [V]

// ---- design units (locked) ---------------------------------------------------------------------
inline constexpr R VR = 2.5L;       // precision reference [V] (LM4040-2.5 / TL431 class)
inline constexpr R VSUP = 9.0L;     // +-9 V supplies (two 9 V batteries, as in the Chua build)
inline constexpr R I0 = 10e-6L;     // core unit current [A]: "1 FSOT unit" in current mode (low: BJT high-injection < 2e-4)
inline constexpr R IU = 0.2e-6L;    // seed-channel unit current [A] (e^3 antilog peaks at 27.5 uA)
inline constexpr R ISTEP = 0.4e-6L;  // fold ladder: one D_eff unit [A]  (D=25 <-> 10 uA = I0)
inline constexpr R KS = 2.0L;       // S readout [V per unit S]
inline constexpr R KG = 0.125L;     // generation input [V per generation g] (= VR/20)
inline constexpr R KF = 1.0L;       // f(D) monitor [V per unit]
inline constexpr R KOMB = 100.0L;   // Omega_b h^2 readout [V per unit]
inline constexpr R KAINV = 0.04L;   // 1/alpha_em readout [V per unit]
inline constexpr R KAS = 10.0L;     // alpha_s(M_Z) readout [V per unit]
inline constexpr R KMHW = 2.0L;     // m_H/m_W readout [V per unit]
inline constexpr R RON = 125.0L;    // CD4066 on-resistance, nominal at 18 V supply span [ohm] (datasheet typ)

struct Fsot {
  Eng F;
  int G = 20;                        // nest generations (engine.hpp:33-55)
  R K, PNEW, PSI, A_med, O_obs, c_med, c_obs, T3_rel_max;
  std::vector<int> Dgen;             // D_eff(g), g = 0..19
  std::vector<R> xgen;               // 5*5^{g/19} before rounding
  std::array<R, 20> gk{};            // rounding thresholds in g for D = k+1/2, k = 5..24
  R S_QM, S_CH, S_CO, alpha_s, inv_alpha, Omb, mHmW;
  Fsot() {
    K = F.K; PNEW = F.P_NEW; PSI = F.PSI_CON;
    // Core law for look = 1, hits = 0 folds (engine.hpp:202-221):
    // S = K [ T1 + T2 + T3 ],  T2 = 1,  T1 = A * O^obs * D^{-1/2} (1 + P_new ln(D/25)),  T3 ~ beta ~ 1e-17
    const R dp = 1;
    const R growth = fsot::m::exp(F.ALPHA * F.GAMMA / F.PHI);
    A_med = fsot::m::cos((F.PSI_CON + dp) / F.ETA_EFF) * fsot::m::exp(R(1) + F.B_IN * dp) * (R(1) + growth * F.C_EFF);
    O_obs = fsot::m::exp(F.C_FACTOR * F.P_VAR) * fsot::m::cos(dp + F.P_VAR);
    c_med = K * A_med / R(5);       // S = K + c * f(D),  f(D) = (D/25)^{-1/2} (1 + P_new ln(D/25))
    c_obs = c_med * O_obs;
    for (int g = 0; g < G; ++g) { xgen.push_back(5 * std::pow(5.0L, R(g) / 19)); Dgen.push_back((int)std::nearbyint(5.0 * std::pow(5.0, double(g) / 19))); }
    for (int k = 5; k <= 24; ++k) gk[k - 5] = 19 * std::log((k + 0.5L) / 5) / std::log(5.0L);
    S_QM = F.S_QUANT; S_CH = F.S_CHEM; S_CO = F.S_COSM;
    alpha_s = R(1) / (F.E * F.PI);
    inv_alpha = fsot::m::ipow(F.E, 3) * fsot::m::ipow(F.PHI, 4) - F.PSI_CON;
    Omb = fsot::m::fabs(F.S_COSM) * (R(1) - F.S_CHEM);
    mHmW = F.S_QUANT * F.PSI_CON + F.S_QUANT;
    // largest |T3/(T1+T2)| over the 33 look=1 folds (the term the circuit sets to zero)
    T3_rel_max = 0;
    for (auto& d : F.DOMAINS) {
      auto s = F.default_input(); s.D_eff = d.D_eff; s.delta_psi = d.delta_psi; s.recent_hits = d.hits; s.observed = d.observed;
      const R Sfull = F.compute_scalar(s);
      auto s0 = s; s0.beta = 0; const R Snot3 = F.compute_scalar(s0);
      T3_rel_max = std::max(T3_rel_max, fsot::m::fabs((Sfull - Snot3) / Sfull));
    }
  }
  R f(R D) const { return std::pow(D / 25, R(-0.5)) * (1 + PNEW * std::log(D / 25)); }
  R S_of(int D, bool obs) const { return K + (obs ? c_obs : c_med) * f(D); }
};

// ---- E24 realisation: best single value or series pair (both E24, 1 ohm .. 10 Mohm) ---------------
inline const std::vector<R>& e24() {
  static std::vector<R> v = [] {
    const int b[] = {10, 11, 12, 13, 15, 16, 18, 20, 22, 24, 27, 30, 33, 36, 39, 43, 47, 51, 56, 62, 68, 75, 82, 91};
    std::vector<R> o; for (R dec = 0.1L; dec < 1e6L; dec *= 10) for (int x : b) o.push_back(std::round(x * dec * 100) / 100);
    o.push_back(1e7L); return o; }();
  return v;
}
struct Real { R a, b, c, val, rel; };
// Series realisation from E24 values: single, pair, or (if the pair is worse than 5e-4) a triple.
inline Real realise(R x) {
  Real best{0, 0, 0, 0, 1e9};
  auto consider = [&](R a, R b, R c) { R v = a + b + c, e = std::fabs(v - x) / x; if (e < best.rel - 1e-15L) best = {a, b, c, v, e}; };
  const auto& E = e24();
  for (R a : E) { if (a > x * 1.01L) break; consider(a, 0, 0);
    for (R b : E) { if (b > a || a + b > x * 1.01L) break; consider(a, b, 0); } }
  for (R a : E) if (a >= x) { consider(a, 0, 0); break; }
  if (best.rel > 5e-4L) {
    Real pair = best;
    for (R a : E) { if (a > x) break; for (R b : E) { if (b > a || a + b > x) break; R rem = x - a - b;
      for (R c : E) { if (c > b) break; if (c > rem * 1.01L) break; consider(a, b, c); } } }
    if (best.rel > pair.rel) best = pair;
  }
  return best;
}
struct Part { std::string name; R ideal; std::string unit, meaning; };

// Every component value of the Phase-1 slice, as a formula of FSOT quantities and the locked design units.
inline std::vector<Part> parts(const Fsot& s) {
  const R VT = VT0, pi = s.F.PI, phi = s.F.PHI;
  std::vector<Part> p;
  auto add = [&](std::string n, R v, std::string u, std::string m) { p.push_back({n, v, u, m}); };
  // references
  add("R_inv", 10e3L, "ohm", "-VR inverter, both arms (ratio 1 exactly: -VR = -2.5 V)");
  // fold ladder (shared threshold string, 200 kohm total)
  const R RS = 200e3L; R prev = 0;
  for (int k = 5; k <= 24; ++k) {
    const R t = s.gk[k - 5] / 20;  // tap voltage / VR = g_k * KG / VR
    add("R_str_" + std::to_string(k), RS * (t - prev), "ohm", "threshold string segment: tap at VR*g_k/20, g_k = 19 log5((k+1/2)/5), k=" + std::to_string(k));
    prev = t;
  }
  add("R_str_top", RS * (1 - prev), "ohm", "threshold string top segment to VR");
  add("R_gunit", 10e3L, "ohm", "generation selector: VR*g/20 from g*R_gunit over (20-g)*R_gunit");
  add("R_step", VR / ISTEP - RON, "ohm", "one D_eff unit = 0.4 uA from VR, minus nominal CD4053 Ron (SPDT current steering: sum node or ground)");
  add("R_base", VR / (5 * ISTEP), "ohm", "D_eff floor 5 units = 2 uA from VR");
  add("R_25", VR / I0, "ohm", "D = 25 reference current 10 uA (LOGR1 Q1)");
  // core law: f(D) = (D/25)^{-1/2} (1 + P_new ln(D/25)) as I0*w*exp(-u/2) with u = ln(D/25)
  add("R_I0", VR / I0, "ohm", "antilog Q3 collector current I0 = 10 uA (one FSOT unit)");
  add("R_x", VT / (s.PNEW * I0), "ohm", "V_T*u -> P_new*u*I0 : R_x = V_T/(P_new I0)");
  add("R_half", 20.0L, "ohm", "1/2 divider (both arms): exponent -u/2 for D^{-1/2}");
  add("R_Tf", KF / I0, "ohm", "f(D) TIA: 1 V per unit f");
  add("R_Sf", 10e3L, "ohm", "S summer feedback");
  add("R_Sx_obs", 10e3L / (KS * s.c_obs), "ohm", "S summer, specimen fold: weight KS*c_obs, c_obs = K*A*O/5");
  add("R_Sx_med", 10e3L / (KS * std::fabs(s.c_med)), "ohm", "S summer, medium fold: weight KS*|c_med|, c_med = K*A/5");
  add("R_S0", 10e3L / (KS * s.K / VR), "ohm", "S summer constant K from -VR (T2 = 1 term)");
  add("R_finv", 10e3L, "ohm", "f inverter for specimen (observed) folds, both arms");
  // m_H/m_W = S_quant (1 + psi_con)
  add("R_mg", 10e3L, "ohm", "m_H/m_W non-inverting gain, ground leg");
  add("R_mf", 10e3L * s.PSI, "ohm", "m_H/m_W gain 1+psi_con: R_mf = psi_con*R_mg");
  // |S_cosm| precision full-wave
  add("R_fw", 10e3L, "ohm", "full-wave rectifier unit (R)"); add("R_fw2", 20e3L, "ohm", "full-wave rectifier 2R");
  // (1 - S_chem) x 40 V
  add("R_c_in", 10e3L, "ohm", "1-S amp input (weight 20)"); add("R_c_f", 200e3L, "ohm", "1-S amp feedback");
  add("R_c_ref", 200e3L / (2 * KS * 20 / (2 * VR)), "ohm", "1-S amp constant from -VR: weight 20*KS/VR = 16");
  // Omega_b h^2 = |S_cosm| (1 - S_chem): translinear (log-antilog) multiplier
  add("R_om_a", KS / I0, "ohm", "|S|*KS -> |S|*I0 into LOGR5 Q1");
  add("R_om_ref", VR / I0, "ohm", "LOGR5 Q2 reference I0 (S = 1)");
  add("R_om_b", 20 * KS / I0, "ohm", "40(1-S) V -> (1-S)*I0 into antilog Q1");
  add("R_om_T", KOMB / I0, "ohm", "Omega_b h^2 TIA: 100 V per unit");
  // alpha_s = e^{-1}/pi : antilog with reference current I0/pi, exponent -1
  add("R_as_pi", pi * (VR - VT) / I0, "ohm", "antilog Q1 current I0/pi (pi as a resistor ratio); Q1 collector held at its base (+V_T): V_CB = 0");
  add("R_vt_bot", 20.0L, "ohm", "V_T divider bottom (exponent 1 neper from VR)");
  add("R_vt_top", 20.0L * (VR / VT - 1), "ohm", "V_T divider top: ratio V_T/VR");
  add("R_as_T", KAS / I0, "ohm", "alpha_s TIA: 10 V per unit");
  // 1/alpha = e^3 phi^4 - psi_con = e^3 (3 phi + 2) - 1 + e^{-1}    (phi^4 = 3 phi + 2, exact identity)
  add("R_a3phi", (VR + 3 * VT) / (3 * phi * IU), "ohm", "3*phi*IU into e^3 antilog Q1 (collector held at base, -3V_T)");
  add("R_a2", (VR + 3 * VT) / (2 * IU), "ohm", "2*IU into e^3 antilog Q1 (collector held at base, -3V_T)");
  add("R_3vt_bot", 50.0L, "ohm", "3 V_T divider bottom (exponent 3 nepers from -VR)");
  add("R_3vt_top", 50.0L * (VR / (3 * VT) - 1), "ohm", "3 V_T divider top: ratio 3V_T/VR");
  add("R_au", (VR - VT) / IU, "ohm", "IU into e^{-1} antilog Q1 (collector held at base, +V_T)");
  add("R_inj", VR / IU, "ohm", "the -1 term: IU injected into the 1/alpha TIA node");
  add("R_ainv_T", KAINV / IU, "ohm", "1/alpha TIA: 0.04 V per unit");
  return p;
}
}  // namespace ckt
