// design9.hpp — LOCK9: fork of design7.hpp (LOCK7, unchanged). One change: the PTAT neper-reference ratio MPT is a variable (default 256 = LOCK7),
// set by derive9 (argument), for the LOCK9 base-engine ablation and lock. Everything else is design7 verbatim.
// design7.hpp — FSOT Omni-Solver LOCK7: fork of phase2/cpp/design2.hpp (LOCK2, unchanged). Base-engine changes, recorded in phase3/locked7/LOCK7.md:
//  (a) deeper baseline: every LOGR reference fraction is multiplied by KD = e^-3/0.7, so a LOGR that sat at ~0.7 of its smallest operating value
//      (n >= -9 mV) now sits at e^-3 of it (n <= -3 V_T); the T1 antilog runs I_in = 7 uA with I_A = 7 uA/(2.5535 e^3) (vx <= -3 V_T at |T1|max);
//      the h/N antilog keeps I_in = I_PT/0.7 and its TIA takes the factor 0.7/r_N. Summer constants follow from the same Refs (consts()).
//  (b) PTAT neper reference at ratio MPT = 256 (was 16): Delta-V_BE = V_T ln 256 = 143 mV, halving V_NEP's relative error per uV of mismatch
//      (the pair is specified matched, see netgen6).
// design2.hpp — FSOT Omni-Solver Phase 2: the full core law S = K(T1+T2+T3) for all 35 folds, the observer
// effect (quirkMod), T2, T3 (valve, acoustic, phase), a PTAT neper reference, and the readouts, as
// pure-analog component values derived in C++20 from the pinned engine (FSOT-2.1-Cpp @3466e8d).
// No FSOT rule is added or changed. Every value here is written to phase2/locked/ and hashed BEFORE simulation.
//
// Topology conventions (all blocks):
//  * LOGR (cascoded log-ratio): input current Ix into a transdiode Q1 (collector = op-amp virtual ground at 0 V,
//    base at 0 V => V_CB = 0); reference current Ir into Q2, whose collector is held at its own base by an
//    op-amp-driven 2N7000 cascode (V_CB = 0, gate current 0). Output n = V_T ln(Ir/Ix) <= 0 (Ir < Ix by design).
//    No beta error: every collector current is the input current exactly (bases are driven by op-amps or ground).
//  * CAL (cascoded antilog): transdiode Q3 carries Iin; Q4 base at Vx <= 0, collector held at Vx by an op-amp +
//    2N7000 cascode; Iout = Iin exp(Vx/V_T) is sunk from the node the 2N7000 drain sits on (a virtual ground).
//  * Summers: inverting, junction scale ("x1 bus": V_T per neper). Constants in nepers enter as fractions of the
//    PTAT neper reference V_NEP = 10 V_T (from a Delta-VBE cell), so every exponent tracks temperature exactly.
#pragma once
#include "fsot/engine.hpp"
#include <cmath>
#include <map>
#include <string>
#include <vector>

namespace d2 {
using R = long double;
using Eng = fsot::Engine<R>;
inline constexpr R kB = 1.380649e-23L, qe = 1.602176634e-19L, T0 = 300.15L;
inline const R VT0 = kB * T0 / qe;
// ---- design units (locked) ----
inline constexpr R VR = 2.5L;        // precision reference [V]
inline constexpr R IU = 5e-6L;       // one FSOT unit in current mode [A]
inline constexpr R I25 = 5e-6L;      // D = 25 in the fold ladder [A] (ladder unit 0.2 uA per D)
inline constexpr R ISTEP = 0.2e-6L;  // one D_eff unit [A] (amendment 1: halved, high-injection budget)
inline constexpr R KS = 2.0L;        // S readout [V per unit]
inline constexpr R KT = 1.0L;        // |T1| monitor [V per unit]
inline constexpr R K1S = 40.0L;      // (1 - S_chem) readout [V per unit], negative polarity: V = -40 (1 - S)
inline constexpr R KOMB = 100.0L;    // Omega_b h^2 readout [V per unit]
inline constexpr R KAINV = 0.04L;    // 1/alpha_em readout [V per unit]
inline constexpr R KAS = 10.0L;      // alpha_s readout [V per unit]
inline constexpr R KANG = 1.0L;      // angle bus [V per rad]
inline constexpr R RON = 125.0L;     // CD4053 Ron nominal (ladder steering only; all other switches drive buffers)
inline R MPT = 256.0L;   // LOCK9: variable (derive9 argument 2); LOCK7 (b) value by default      // Delta-VBE current ratio of both PTAT cells (amendment 1)
inline constexpr R IA = 7e-6L / (2.5534867L * 20.085536923187667741L);   // LOCK7 (a): T1 antilog unit, I_in = mT1 IA = 7 uA       // T1 antilog unit [A] (amendment 1: high-injection budget)
inline constexpr R RSUM = 10e3L;     // summer feedback [ohm]
inline constexpr R IS0 = 0.1e-6L;    // seed-channel unit for 1/alpha (e^3 chain) [A]

struct Part { std::string name; R ideal; std::string unit, meaning; };

struct Fsot {
  Eng F;
  R PI, E, PHI, GAM, ALPHA, PSI, ETA, BETA, THS, POOF, CEFF, ABL, PVAR, BIN, AIN, SUC, CHAOS, PNEW, CF, K;
  R look_at, look_he, valve2, acoustic1, phase1, cs, lnbeta;
  Fsot() {
    PI = F.PI; E = F.E; PHI = F.PHI; GAM = F.GAMMA; ALPHA = F.ALPHA; PSI = F.PSI_CON; ETA = F.ETA_EFF; BETA = F.BETA;
    THS = F.THETA_S; POOF = F.POOF; CEFF = F.C_EFF; ABL = F.A_BLEED; PVAR = F.P_VAR; BIN = F.B_IN; AIN = F.A_IN;
    SUC = F.SUCTION; CHAOS = F.CHAOS; PNEW = F.P_NEW; CF = F.C_FACTOR; K = F.K;
    look_at = F.fold_look("Atomic_Physics"); look_he = F.fold_look("High_Energy_Physics");
    valve2 = R(1) + POOF * fsot::m::cos(THS + PI) + SUC * fsot::m::sin(THS);
    const R s1 = fsot::m::sin(R(1)), c1 = fsot::m::cos(R(1));
    acoustic1 = R(1) + ABL * s1 * s1 / PHI + AIN * c1 * c1 / PHI;     // delta_theta = 1
    phase1 = R(1) + BIN * PVAR;
    cs = (AIN - ABL) / PHI;                                          // acoustic = 1 + A_in/phi - cs sin^2(dt)
    lnbeta = -(fsot::m::pow(PI, PI) + (E - R(1)));                   // ln beta (engine.hpp:120)
  }
};

// reference ("r") fractions of each LOGR, in units of the block's own scale current. Ir < Ix over the domain.
struct Refs {
  // amendment 1: references moved to ~0.7 of the smallest operating value (cascode margin >= 8 mV kept),
  // which halves the high-injection (I/IKF) log-conformity error of every block.
  static constexpr R KD = 0.049787068367863942979L / 0.7L;   // LOCK7 (a): e^-3 / 0.7
  R N = 0.7L * KD, P = 0.7L * KD, Db = 7.5e-6L /*A, unchanged: D current on the cascoded side*/, cosy = 0.68L * KD, a4 = 1.4L * KD, a5 = 0.37L * KD, y = 0.03L * KD, w = 0.18L * KD, sp = 0.7L * KD,
    v = 0.41L * KD, vp = 0.66L * KD, ch = 0.72L * KD, v2 = 0.65L * KD, ac = 1.27L * KD, ph = 1.27L * KD, t = 0.7L * KD, tp = 0.6L * KD, sc = 0.7L * KD, am = 0.7L * KD,
    oS = 0.36L * KD, o1 = 0.32L * KD;
  R mT1 = 2.5534867L * 20.085536923187667741L, mT2 = 1.4L, mG = 1.4L;   // LOCK7 (a): mT1 = |T1|max e^3  // antilog input multiples (keep Vx <= -8 mV); T1 antilog unit is IA
};

inline std::vector<Part> parts(const Fsot& s) {
  const Refs r; std::vector<Part> p;
  auto add = [&](std::string n, R v, std::string u, std::string m) { p.push_back({n, v, u, m}); };
  const R RU = VR / IU, VNEP0 = 10 * VT0, RPT = VNEP0 / IU, RY = KANG / IU;
  add("R_inv", 10e3L, "ohm", "unity inverters, both arms");
  add("R_sum", RSUM, "ohm", "summer feedback (junction-scale log summers)");
  add("R_U", RU, "ohm", "one FSOT unit I_U = 5 uA from VR into a virtual ground");
  // fold ladder (shared thresholds, as Phase 1); two current copies (CD4053 poles): LOGR_Db and the chaos TIA
  { const R RS = 200e3L; R prev = 0;
    for (int k = 5; k <= 24; ++k) { const R gk = 19 * std::log((k + 0.5L) / 5) / std::log(5.0L); const R t = gk / 20;
      add("R_str_" + std::to_string(k), RS * (t - prev), "ohm", "threshold string: tap VR*g_k/20, g_k = 19 log5((k+1/2)/5)"); prev = t; }
    add("R_str_top", RS * (1 - prev), "ohm", "threshold string top"); }
  add("R_gunit", 10e3L, "ohm", "generation selector string unit");
  add("R_step", VR / ISTEP - RON, "ohm", "one D_eff unit (0.4 uA) minus nominal CD4053 Ron");
  add("R_base", VR / (5 * ISTEP), "ohm", "D_eff floor (5 units)");
  add("R_pullup", 10e3L, "ohm", "LM139 open-collector pull-up to +9 V");
  // PTAT neper reference: Q1 1 uA, Q2 64 uA, u = V_T ln 64 -> x (10/ln 64) -> V_NEP = 10 V_T
  // amendment 1: two Delta-VBE cells A (64 uA : 4 uA) and B (32 uA : 2 uA), same ratio 16;
  // V_NEP = (10/ln MPT)(2 dV_B - dV_A) cancels every error proportional to collector current (I/IKF, r_b I_B) to first order.
  add("R_ptA_hi", VR / (64e-6L), "ohm", "PTAT cell A transdiode 64 uA"); add("R_ptA_lo", VR / (64e-6L / MPT), "ohm", "PTAT cell A cascoded 64 uA/MPT");
  add("R_ptB_hi", VR / (32e-6L), "ohm", "PTAT cell B transdiode 32 uA"); add("R_ptB_lo", VR / (32e-6L / MPT), "ohm", "PTAT cell B cascoded 32 uA/MPT");
  add("R_ptg_B", 10e3L / (2 * 10 / std::log(MPT)), "ohm", "V_NEP summer: weight 2*10/ln MPT on n_B");
  add("R_ptg_A", 10e3L / (10 / std::log(MPT)), "ohm", "V_NEP summer: weight 10/ln MPT on -n_A");
  // look selector (delta_psi): angle bus from VR (1 V/rad) and neper bus from -V_NEP
  { const R tot = 100e3L, a = s.look_at, h = s.look_he;
    add("R_lkl_a", tot * a * KANG / VR, "ohm", "lin look divider: bottom -> tap e/pi rad");
    add("R_lkl_h", tot * (h - a) * KANG / VR, "ohm", "lin look divider: -> tap 1-POOF/pi rad");
    add("R_lkl_1", tot * (1 - h) * KANG / VR, "ohm", "lin look divider: -> tap 1 rad");
    add("R_lkl_t", tot * (1 - KANG / VR), "ohm", "lin look divider: top to VR");
    add("R_lkn_a", tot * a, "ohm", "neper look divider from -V_NEP: bottom -> e/pi");
    add("R_lkn_h", tot * (h - a), "ohm", "neper look divider -> 1-POOF/pi");
    add("R_lkn_1", tot * (1 - h), "ohm", "neper look divider -> 1 (= -V_NEP)"); }
  add("R_dt_b", 100e3L * KANG / VR, "ohm", "delta_theta = 1 rad divider bottom (input ratio)");
  add("R_dt_t", 100e3L * (1 - KANG / VR), "ohm", "delta_theta divider top");
  // inputs as current ratios
  add("R_N", RU / 1, "ohm", "N = 1 input: I_N = N I_U"); add("R_P", RU / 1, "ohm", "P = 1 input");
  add("R_Nr", RU / r.N, "ohm", "LOGR_N reference 0.5 I_U"); add("R_Pr", RU / r.P, "ohm", "LOGR_P reference");
  add("R_Dbr", VR / r.Db, "ohm", "LOGR_Db transdiode reference 7.5 uA (D = 37.5); D current on the cascoded side");
  add("R_sc", RU / 1, "ohm", "T2 scale = 1 input"); add("R_am", RU / 1, "ohm", "T2 amplitude = 1 input");
  add("R_scr", RU / r.sc, "ohm", "LOGR_sc reference"); add("R_amr", RU / r.am, "ohm", "LOGR_am reference");
  add("R_tb", RU, "ohm", "trend_bias input: v_tb = -tb*VR through R_U into the S node");
  // h/N : CAL(I_in = 2 I_PT, Vx = n_N) -> I_PT/N -> TIA (R_pt) -> V_NEP/N -> hits switch -> alpha*h divider
  add("R_pt", RPT, "ohm", "PTAT unit I_PT = V_NEP/R_pt = 5 uA at 27 C");
  add("R_hN_in", RPT * 0.7L, "ohm", "h/N antilog input I_PT/0.7 (LOCK7: unchanged current)");
  add("R_hN_T", RPT * 0.7L / r.N, "ohm", "h/N TIA: V = V_NEP/N (LOCK7: x 0.7/r_N = e^3, restores the LOGR_N reference)");
  add("R_al_b", 10.0L, "ohm", "alpha divider bottom"); add("R_al_t", 10.0L * (1 - s.ALPHA) / s.ALPHA, "ohm", "alpha divider top: ratio alpha (x h, hits=1)");
  // a2: cos y, y = (psi+dpsi)(pi-1) - pi on the angle bus (1 V/rad); cos y = 1 - y^2/2 + y^4/24 - y^6/720
  add("R_y_dp", RSUM / (s.PI - 1), "ohm", "y summer: delta_psi weight (pi-1) = 1/eta");
  add("R_y_c", RSUM / ((s.PI - s.PSI * (s.PI - 1)) / VR * KANG), "ohm", "y summer: constant pi - psi_con(pi-1) rad from -VR");
  add("R_fw", 10e3L, "ohm", "precision full-wave unit R"); add("R_fw2", 20e3L, "ohm", "precision full-wave 2R");
  add("R_ang", RY, "ohm", "angle -> current: 1 rad <-> I_U");
  add("R_yr", RU / r.y, "ohm", "LOGR_y reference 0.03 I_U");
  add("R_cosr", RU / r.cosy, "ohm", "LOGR_cos reference 0.5 I_U");
  add("R_pT", 200e3L, "ohm", "positive-term TIA feedback and its copy resistor into the LOGR input (1:1)");
  // observer: w = dpsi + P_var - pi/2 ; |sin w| = |w|(1 - w^2/6 + w^4/120 - w^6/5040)
  add("R_w_dp", RSUM, "ohm", "w summer: delta_psi weight 1");
  add("R_w_c", RSUM / ((s.PI / 2 - s.PVAR) / VR * KANG), "ohm", "w summer: constant pi/2 - P_var from -VR");
  add("R_wr", RU / r.w, "ohm", "LOGR_w reference 0.125 I_U"); add("R_spr", RU / r.sp, "ohm", "LOGR_sp reference");
  // T3: v = pi/2 - dpsi (cos dpsi = sin v), delta_theta sine, chaos, valve, acoustic, phase
  add("R_v_dp", RSUM, "ohm", "v summer: delta_psi weight 1");
  add("R_v_c", RSUM / ((s.PI / 2) / VR * KANG), "ohm", "v summer: constant pi/2 from -VR");
  add("R_vr", RU / r.v, "ohm", "LOGR_v reference"); add("R_vpr", RU / r.vp, "ohm", "LOGR_vp reference");
  add("R_tr", RU / r.t, "ohm", "LOGR_t reference"); add("R_tpr", RU / r.tp, "ohm", "LOGR_tp reference");
  add("R_TD", 200e3L, "ohm", "chaos: D-current TIA (V_D = -R_TD * 0.2 uA * D)");
  add("R_chD", 200e3L * ISTEP * 25 / (std::fabs(s.CHAOS) * IU), "ohm", "chaos: |CHAOS| D/25 I_U sunk from LOGR_ch (CHAOS < 0)");
  add("R_ch0", RU / (1 - s.CHAOS), "ohm", "chaos: (1 - CHAOS) I_U");
  add("R_chr", RU / r.ch, "ohm", "LOGR_ch reference");
  add("R_v2", RU / s.valve2, "ohm", "valve 1 + POOF cos(theta_s+pi) + SUCTION sin theta_s (current ratio)");
  add("R_v2r", RU / r.v2, "ohm", "LOGR_v2 reference");
  add("R_ac0", RU / (1 + s.AIN / s.PHI), "ohm", "acoustic: (1 + A_in/phi) I_U; minus (A_in - A_bleed)/phi sin^2(dt) I_U (antilog)");
  add("R_acr", RU / r.ac, "ohm", "LOGR_ac reference");
  add("R_ph", RU / s.phase1, "ohm", "phase 1 + B_in P_var (current ratio)"); add("R_phr", RU / r.ph, "ohm", "LOGR_ph reference");
  // a4: growth antilog (I_in = 2 C_eff I_U) -> TIA -> + I_U into LOGR_a4
  add("R_g_in", RU / (r.mG * s.CEFF), "ohm", "growth antilog input 1.4 C_eff I_U");
  add("R_a4r", RU / r.a4, "ohm", "LOGR_a4 reference I_U");
  // a5: I_PT0 (1 + P_new ln(D/25)) from n_Db = V_T ln(D/30)
  add("R_x", RPT / (10 * s.PNEW), "ohm", "a5: n_Db -> P_new ln(D/37.5) I_PT : R_x = V_T/(P_new I_PT)");
  add("R_x12", RPT / (s.PNEW * std::log(r.Db / ISTEP / 25)), "ohm", "a5: + P_new ln(1.5) I_PT from V_NEP (D/37.5 -> D/25)");
  add("R_a5r", RPT / r.a5, "ohm", "LOGR_a5 reference 0.37 I_PT");
  // T1/T2/T3 antilog inputs, TIAs, S node
  add("R_T1in", VR / (r.mT1 * IA), "ohm", "T1 antilog input 3.5 I_A (I_A = 2 uA)"); add("R_T2in", RU / r.mT2, "ohm", "T2 antilog input 1.4 I_U");
  add("R_T3in", RU, "ohm", "T3 antilog input I_U");
  add("R_T1T", KT / IA, "ohm", "|T1| TIA: 1 V per unit (I_A per unit)"); add("R_s1", KT / IU, "ohm", "signed T1 -> S node: I_U per unit");
  add("R_SF", KS * s.K / IU, "ohm", "S node feedback: V_S = KS*K*(T1+T2+T3)");
  // power-series antilog inputs I_U
  add("R_pin", RU, "ohm", "power-term antilog input I_U");
  // readouts (omb mode)
  add("R_1s0", RU * s.K, "ohm", "1-S node: I_U/K source");
  add("R_1sF", K1S * s.K / IU, "ohm", "1-S TIA: V = -40 (1 - S_chem)");
  add("R_oS", KS / IU, "ohm", "|S_cosm| (2 V/unit) -> |S| I_U"); add("R_oSr", RU / r.oS, "ohm", "LOGR_oS reference");
  add("R_o1", K1S / (10 * IU), "ohm", "40(1-S) V -> 10 (1-S) I_U"); add("R_o1r", RU / r.o1, "ohm", "LOGR_o1 reference");
  add("R_omin", RU, "ohm", "Omega antilog input I_U"); add("R_omT", KOMB / (10 * IU), "ohm", "Omega TIA: 100 V per unit");
  // constants mode: alpha_s = e^-1/pi, 1/alpha = e^3 (3phi+2) - 1 + e^-1, e^3 = (4 e^(1 - ln4))^3 (three e^1 stages)
  add("R_as_in", s.PI * RU, "ohm", "alpha_s antilog input I_U/pi (pi as a ratio)"); add("R_asT", KAS / IU, "ohm", "alpha_s TIA 10 V/unit");
  add("R_e1in", VR / (64 * IS0), "ohm", "e-chain stage 1 input 64 I_s");
  add("R_eT", 100e3L, "ohm", "e-chain inter-stage TIA feedback (and 1:1 copy into the next stage)");
  add("R_e3c", 100e3L / (3 * s.PHI + 2), "ohm", "stage-3 input copy: x(3 phi + 2) (phi^4 = 3 phi + 2)");
  add("R_em1", VR / IS0, "ohm", "1/alpha: the -1 term (I_s sourced)"); add("R_eiin", VR / IS0, "ohm", "1/alpha: e^-1 antilog input I_s");
  add("R_aiT", KAINV / IS0, "ohm", "1/alpha TIA: 0.04 V per unit");
  return p;
}

// summer constants in nepers (supplied as fractions of V_NEP: weight |C|/10 from +V_NEP if C < 0, from -V_NEP if C > 0)
inline std::map<std::string, R> consts(const Fsot& s) {
  const Refs r; std::map<std::string, R> c; auto L = [](R x) { return std::log(x); };
  const R Dc = -0.5L * L(r.Db / ISTEP / 25) - L(5.0L);  // D^{-1/2}: -1/2 ln(D/25) - ln 5 = -1/2 n_Db/V_T - 1/2 ln 1.5 - ln 5
  c["T1"] = L(r.N) + L(r.P) + Dc + L(r.cosy) + L(r.a5) + L(r.a4) - L(r.mT1);         // ungated part; + rho, B_in dpsi, -alpha h/N
  c["T1obs"] = s.CF * s.PVAR + L(r.w) + L(r.sp);                                    // gated with `observed`
  c["T2"] = L(r.sc) + L(r.am) - L(r.mT2);
  c["T3"] = s.lnbeta + L(r.v) + L(r.vp) + L(r.N) + L(r.P) + Dc + L(r.ch) + L(r.v2) + L(r.ac) + L(r.ph) - 0;  // CAL I_in = I_U
  c["G"] = -L(r.mG);                                                                // growth: + alpha gamma/phi (1 - h/N)
  c["sin2"] = 2 * L(r.t) + 2 * L(r.tp) + L(s.cs);
  c["Om"] = L(r.oS) + L(r.o1);
  const R fac[] = {1, 1, 2, 6, 24, 120, 720, 5040, 40320, 362880, 3628800, 39916800};
  for (int k = 1; k <= 3; ++k) c["cy" + std::to_string(k)] = 2 * k * L(r.y) - L(fac[2 * k]);
  for (int k = 1; k <= 3; ++k) c["sw" + std::to_string(k)] = 2 * k * L(r.w) - L(fac[2 * k + 1]);
  for (int k = 1; k <= 4; ++k) c["sv" + std::to_string(k)] = 2 * k * L(r.v) - L(fac[2 * k + 1]);
  for (int k = 1; k <= 5; ++k) c["st" + std::to_string(k)] = 2 * k * L(r.t) - L(fac[2 * k + 1]);
  c["e1"] = 1 - L(4.0L);  // one e^1 stage: x e/4 (e = 4 e^(1 - ln 4))
  c["em1"] = -1;          // e^-1
  return c;
}
}  // namespace d2
