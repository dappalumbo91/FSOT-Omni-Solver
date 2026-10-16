// hubbench2.cpp — FSOT Omni-Solver LOCK9H Amendment 9H.1: fork of hubbench.cpp (LOCK9H, unchanged). Two defects found by the first LOCK9H run
// (results disclosed in locked9h1/LOCK9H1.md): (1) R_a was sized so that I_a = I_r at the nominal coefficient, which makes every mode's rate
// 1/T0 independent of the coefficient; fixed: R_a = (readout scale)/I_r, so I_a = coef I_r exactly as the compiled voltage dictates
// (all three R_a = 1 Mohm; the coefficient enters only through the compiled voltage). (2) .ic cannot set a node driven by an ideal VCVS
// (the integrator outputs), so the operating point started at the steady state; fixed: capacitor IC= and .tran ... uic.
// Original header follows.
// hubbench.cpp — FSOT Omni-Solver LOCK9H: 0-D hub relaxation bench (transient analog computer) and its C++20 long double reference.
// Hub (vendor/fsot_dynamics.py, sha256 837817ba3b04..., not part of the pinned engine) reduced to 0-D modes on the engine constants:
//   S mode       dS/dt = -gamma_rel (S - S_eq),  S(0) = 0, S_eq = domain_scalar("Cosmology")      (scalar_transport_rhs, v = dS = d2S = 0)
//   dark Fluid   dv/dt = -mu(D_eff(Fluid_Dynamics)) v,  v(0) = 1                                    (momentum_rhs, rho = 1, k = 1: d2v = -v, unobserved)
//   observed D25 dv/dt = -mu(25) v + J_obs,  v(0) = 1                                               (momentum_rhs, rho = 1, k = 1, observed)
// Time unit: one hub unit = T0 = 10 ms of circuit time; 8 units simulated.
// SPICE names are case-insensitive: every node and element name below is unique ignoring case.
// Circuit: per mode an inverting integrator (ideal op-amp, C), a state inverter, and a one-quadrant translinear multiplier (four 2N3904
// log/antilog transistors, V_BC = 0) I_out = I_a I_x / I_r feeding the integrator; I_a = (compiled coefficient voltage) / R_a, where the
// coefficient voltages are the LOCK8/8.1 compiled C_H readouts (10 gamma_rel, 10 mu_Fluid, 10 mu_D25, 100 J_obs) and the C_S Cosmology channel
// (2 S_eq), held at their stage-1 ideal 27 C solve values (hub_stage1.tsv). Variant "exact" replaces them by the exact target voltages, so the
// bench's own error is separable from the coefficient compile error. All component values: C++20 long double from the engine constants.
// Usage: hubbench values <outdir>                       -> hubbench_values.tsv, hubref_cpp.tsv (exact solution + RK4, dt = 1e-3 unit)
//        hubbench net <values.tsv> <stage1.tsv> compiled|exact <out.cir> <data.out>
#include "fsot/engine.hpp"
#include <cstdio>
#include <fstream>
#include <map>
#include <sstream>
#include <string>
using R = long double; namespace mth = fsot::m;
static const R T0 = 0.01L, IR = 1e-5L, VREF = 2.5L, VS = 2;   // design units: 10 ms per hub unit, 10 uA multiplier reference, 2.5 V reference, 2 V per state unit
int main(int argc, char** argv) {
  if (argc < 3) { std::fprintf(stderr, "usage: see header\n"); return 2; }
  const std::string mode = argv[1];
  if (mode == "values") {
    fsot::Engine<R> E; const std::string od = argv[2];
    const R gam = (E.CHAOS < 0 ? -E.CHAOS : E.CHAOS) + E.PSI_CON * E.POOF; const int DF = E.domain("Fluid_Dynamics").D_eff;
    const R Dm = (R(DF) > 25 ? R(DF) - 25 : 25 - R(DF)) / 25, AC = E.CHAOS < 0 ? -E.CHAOS : E.CHAOS;
    const R muF = AC * Dm + E.A_BLEED * E.POOF, mu25 = E.A_BLEED * E.POOF, J = E.C_FACTOR * mth::cos(R(1) + E.THETA_S), Seq = E.domain_scalar("Cosmology");
    // multiplier: I_x = V_state / R_x = I_r at one state unit; I_a = I_r at the nominal coefficient; I_r = VREF / R_r
    const R Rr = VREF / IR, Rx = VS / IR;   // -VS C du/dt = (I_a/I_r) (VS u / R_x) -> rate (I_a/I_r)/(R_x C) = coef/T0 at I_a = coef I_r
    const R Ca = T0 / Rx;   // = 50 nF
    const R Ra_g = 10 / IR, Ra_F = 10 / IR, Ra_25 = 10 / IR;   // 9H.1: readout scale 10: V = 10 coef -> I_a = coef I_r
    // J: I_j into the integrator node; -VS C du/dt = I_out - I_j -> du/dt contribution I_j/(VS C) = J/T0 -> I_j = VS C J / T0; V_J = 100 J -> R_J = 100 T0 / (VS C)
    const R RJ = 100 * T0 / (VS * Ca);
    std::ofstream v(od + "/hubbench_values.tsv"); char b[512];
    auto P = [&](const char* n, R x, const char* u, const char* m) { std::snprintf(b, sizeof b, "%s\t%.18Lg\t%s\t%s\n", n, x, u, m); v << b; };
    P("gamma_rel", gam, "1/unit", "|CHAOS| + PSI_CON POOF (fsot_dynamics.py:143)"); P("mu_Fluid", muF, "1/unit", "|CHAOS| |D_eff(Fluid_Dynamics) - 25|/25 + A_BLEED POOF (:84)");
    P("D_eff_Fluid", DF, "-", "engine domain table"); P("mu_D25", mu25, "1/unit", "A_BLEED POOF (:84, D = 25)"); P("J_obs", J, "1/unit", "C_FACTOR cos(1 + THETA_S) (:97)");
    P("S_eq", Seq, "-", "domain_scalar(Cosmology) (:308)"); P("T0", T0, "s", "circuit time per hub unit"); P("VREF", VREF, "V", "multiplier reference");
    P("R_r", Rr, "ohm", "I_r = VREF/R_r = 10 uA"); P("R_x", Rx, "ohm", "I_x = V_state/R_x, 2 V per state unit"); P("C_int", Ca, "F", "T0/R_x: rate = (I_a/I_r)/(R_x C) per T0");
    P("R_a_gamma", Ra_g, "ohm", "I_a = V(zA1xo)/R_a = gamma_rel I_r, V = 10 gamma_rel"); P("R_a_muF", Ra_F, "ohm", "I_a = V(zA96xo)/R_a = mu_Fluid I_r");
    P("R_a_mu25", Ra_25, "ohm", "I_a = V(zT101xo)/R_a = mu_D25 I_r"); P("R_J", RJ, "ohm", "I_j = V(zT104xo)/R_J, V = 100 J_obs");
    P("V_gamma_exact", 10 * gam, "V", "exact readout"); P("V_muF_exact", 10 * muF, "V", "exact readout"); P("V_mu25_exact", 10 * mu25, "V", "exact readout");
    P("V_J_exact", 100 * J, "V", "exact readout"); P("V_Seq_exact", 2 * Seq, "V", "exact C_S channel output (V = 2 S)");
    // reference: exact solution and classical RK4 (dt = 1e-3 unit) in long double
    std::ofstream r(od + "/hubref_cpp.tsv"); r << "t\tS_exact\tS_rk4\tvF_exact\tvF_rk4\tv25_exact\tv25_rk4\n";
    R S = 0, vF = 1, v25 = 1; const int N = 1000; const R h = R(1) / N;
    auto fS = [&](R s) { return -gam * (s - Seq); }; auto fF = [&](R x) { return -muF * x; }; auto f25 = [&](R x) { return -mu25 * x + J; };
    auto rk = [&](auto f, R& x) { R k1 = f(x), k2 = f(x + h / 2 * k1), k3 = f(x + h / 2 * k2), k4 = f(x + h * k3); x += h / 6 * (k1 + 2 * k2 + 2 * k3 + k4); };
    for (int i = 0; i <= 8 * N; ++i) {
      if (i % (N / 4) == 0) { const R t = R(i) / N; const R vinf = J / mu25;
        std::snprintf(b, sizeof b, "%.4Lf\t%.18Le\t%.18Le\t%.18Le\t%.18Le\t%.18Le\t%.18Le\n", t, Seq * (1 - mth::exp(-gam * t)), S, mth::exp(-muF * t), vF, vinf + (1 - vinf) * mth::exp(-mu25 * t), v25); r << b; }
      rk(fS, S); rk(fF, vF); rk(f25, v25); }
    std::printf("HUBBENCH2_VALUES_OK\n"); return 0; }
  if (mode == "net" && argc >= 7) {
    std::map<std::string, R> V; { std::ifstream f(argv[2]); std::string l; while (std::getline(f, l)) { std::istringstream t(l); std::string n, x; std::getline(t, n, '\t'); std::getline(t, x, '\t'); V[n] = std::stold(x); } }
    std::map<std::string, R> S1; { std::ifstream f(argv[3]); std::string l; while (std::getline(f, l)) { if (l.empty() || l[0] == '#') continue; std::istringstream t(l); std::string n, x; std::getline(t, n, '\t'); std::getline(t, x, '\t'); S1[n] = std::stold(x); } }
    const bool ex = std::string(argv[4]) == "exact"; auto src = [&](const char* s1, const char* exn) { return ex ? V.at(exn) : S1.at(s1); };
    const R Vg = src("v(za1xo)", "V_gamma_exact"), VF = src("v(za96xo)", "V_muF_exact"), V25 = src("v(zt101xo)", "V_mu25_exact"), VJ = src("v(zt104xo)", "V_J_exact"), VSq = src("v(vsdh)", "V_Seq_exact");
    std::ostringstream N; char b[512];
    N << "* hubbench2 (LOCK9H Amendment 9H.1) " << argv[4] << " coefficients\n";
    N << ".model QH NPN(IS=6.734e-15 XTI=3 EG=1.11 VAF=74.03 BF=416.4 NE=1.259 ISE=6.734f IKF=66.78m XTB=1.5 BR=.7371 NC=2 ISC=0 IKR=0 RC=1 CJC=3.638p MJC=.3085 VJC=.75 FC=.5 CJE=4.493p MJE=.2593 VJE=.75 TR=239.5n TF=301.2p ITF=.4 VTF=4 XTF=2 RB=10)\n";
    auto OA = [&](const std::string& n, const std::string& pl, const std::string& mi, const std::string& o) { N << "E_" << n << " " << o << " 0 " << pl << " " << mi << " 1e6\n"; };
    auto Rs = [&](const std::string& n, const std::string& a, const std::string& c, R x) { std::snprintf(b, sizeof b, "R_%s %s %s %.15Lg\n", n.c_str(), a.c_str(), c.c_str(), x); N << b; };
    auto Vs = [&](const std::string& n, const std::string& a, R x) { std::snprintf(b, sizeof b, "V_%s %s 0 DC %.15Lg\n", n.c_str(), a.c_str(), x); N << b; };
    Vs("ref", "vref", V.at("VREF")); Vs("g", "vg", Vg); Vs("F", "vF", VF); Vs("25", "v25", V25); Vs("J", "vJ", VJ); Vs("Sq", "vSq", VSq);
    // shared reference log cell: V(lr) = -V_T ln(I_r/I_S)
    Rs("r", "vref", "nr", V.at("R_r")); N << "Q_r nr 0 lr QH\n"; OA("r", "0", "nr", "lr");
    auto mode1 = [&](const std::string& m, const std::string& vc, R Ra, bool J, R ic) {
      const std::string o = "o" + m, s = "s" + m, p = "p" + m;   // integrator out (= -VS u), summing node, state (= +VS u)
      OA("i" + m, "0", s, o); std::snprintf(b, sizeof b, "C_%s %s %s %.15Lg IC=%.15Lg\n", m.c_str(), o.c_str(), s.c_str(), V.at("C_int"), ic); N << b;   // 9H.1: IC on the capacitor (V(s) = 0)
      Rs("n1" + m, o, "ni" + m, 1e5L); Rs("n2" + m, "ni" + m, p, 1e5L); OA("n" + m, "0", "ni" + m, p);   // state inverter
      Rs("a" + m, vc, "na" + m, Ra); N << "Q_a" << m << " na" << m << " 0 la" << m << " QH\n"; OA("a" + m, "0", "na" + m, "la" + m);   // coefficient log
      Rs("x" + m, p, "nx" + m, V.at("R_x")); N << "Q_x" << m << " nx" << m << " 0 lx" << m << " QH\n"; OA("x" + m, "0", "nx" + m, "lx" + m);   // state log
      Rs("s1" + m, "la" + m, "nu1" + m, 1e4L); Rs("s2" + m, "lx" + m, "nu1" + m, 1e4L); Rs("s3" + m, "nu1" + m, "u1" + m, 1e4L); OA("u1" + m, "0", "nu1" + m, "u1" + m);   // u1 = -(la + lx)
      Rs("t1" + m, "u1" + m, "nu2" + m, 1e4L); Rs("t2" + m, "lr", "nu2" + m, 1e4L); Rs("t3" + m, "nu2" + m, "u2" + m, 1e4L); OA("u2" + m, "0", "nu2" + m, "u2" + m);   // u2 = la + lx - lr
      N << "Q_o" << m << " " << s << " 0 u2" << m << " QH\n";   // antilog: I_C = I_a I_x / I_r out of the integrator node
      if (J) Rs("J" + m, "vJ", s, V.at("R_J")); };
    mode1("S", "vg", V.at("R_a_gamma"), false, VSq); mode1("F", "vF", V.at("R_a_muF"), false, -VS); mode1("25", "v25", V.at("R_a_mu25"), true, -VS);
    // S output: V(outS) = -(V(oS) - V(vSq)) = VS (u + S_eq) = 2 S  (u = S - S_eq; V(oS) = -2u, V(vSq) = 2 S_eq)
    Rs("q1", "vSq", "nq", 1e5L); Rs("q2", "nq", "vSqn", 1e5L); OA("q", "0", "nq", "vSqn");
    Rs("w1", "oS", "nw", 1e5L); Rs("w2", "vSqn", "nw", 1e5L); Rs("w3", "nw", "outS", 1e5L); OA("w", "0", "nw", "outS");
    // initial conditions: S(0) = 0 -> u(0) = -S_eq -> V(oS) = VS S_eq = V(vSq); v(0) = 1 -> V(oF) = V(o25) = -VS
    N << ".options reltol=1e-7 abstol=1e-15 vntol=1e-9 method=gear maxord=2\n.temp 27\n.tran 5u 80m 0 5u uic\n.control\nrun\nwrdata " << argv[6] << " v(outS) v(pF) v(p25) v(oS)\n.endc\n.end\n";
    std::ofstream(argv[5]) << N.str(); std::printf("HUBBENCH2_NET_OK\n"); return 0; }
  return 2; }
