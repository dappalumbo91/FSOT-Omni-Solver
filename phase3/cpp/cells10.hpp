// cells10.hpp — LOCK10A: isolated-cell bench definitions shared by netgen9 (mode "cell") and cellpred (closed-form predictions).
// Every cell uses the exact netgen8 topology (LOGR, CAL, CALU, SUM, TIA, the PTAT V_NEP generator) at the operating points the 7.1
// compiler and the LOCK7 base actually use. Error e of a cell, in nepers:
//   LOGR: e = n/V_T - ln(I2/I1)              (I1 into the transdiode c1, I2 into the cascoded c2; resistor-defined nominal currents)
//   CAL/CALU: e = ln(I_out/I_in) - v_x/V_T   (I_out read on an ideal-resistor TIA: I_out = -V_out/R_T)
//   SUM: e = (V_out - V_ideal)/V_T           TIA: e = ln(-V_out/(I R_T))
//   VNEP: e = V_NEP/(10 V_T) - 1 (relative, not nepers)
// V_T = k T/q with ngspice-44's constants (CODATA 2014: k = 1.38064852e-23, q = 1.6021766208e-19), T = 300.15 K.
#pragma once
#include <cmath>
#include <string>
#include <vector>
namespace c10 {
inline constexpr double KQ = 1.38064852e-23 / 1.6021766208e-19, TK = 300.15, VT = KQ * TK;
inline constexpr double E3M = 0.049787068367863942979, KD = E3M / 0.7, VREF = 2.5, RSUM = 10e3;
struct In { double v_nep; double w; };   // summer input: a V_T-scaled source v_nep * V_T(T), weight w (R = RSUM/w)
struct Cell { std::string name, kind; double I1 = 0, I2 = 0; bool ptat1 = false; double vx = 0; std::vector<In> in; double I = 0, RT = 0; std::string what; };
inline std::vector<Cell> cells() {
  std::vector<Cell> c;
  auto L = [&](std::string n, double I1, double I2, bool p, std::string w) { Cell x; x.name = n; x.kind = "LOGR"; x.I1 = I1; x.I2 = I2; x.ptat1 = p; x.what = w; c.push_back(x); };
  auto A = [&](std::string n, std::string k, double Iin, double vx, std::string w) { Cell x; x.name = n; x.kind = k; x.I1 = Iin; x.vx = vx; x.RT = 1.0 / (Iin * std::exp(vx)); x.what = w; c.push_back(x); };
  L("LOGRa", 1e-6, 1e-6 * E3M, false, "7.1 signal LOGR: reference IIN = 1 uA on c1 (from VREF), signal IIN e^-3 on c2 (from VREF)");
  L("LOGRb", 1e-6, 1e-6 * 0.36787944117144233, false, "7.1 constant-generator base cell, m c = e: c1 IIN, c2 IIN/e");
  L("LOGRp", 1e-6, 1e-6 * E3M, true, "7.1 PTAT-referenced LOGR: c1 IIN from V_NEP (10 V_T / 0.2586 Mohm), c2 IIN e^-3 from VREF");
  L("LOGRk", 5e-6, 5e-6 * 0.7 * KD, false, "LOCK7 base channel LOGR_N: signal I_U = 5 uA on c1, reference 0.7 KD I_U on c2 (deeper baseline)");
  A("CALa", "CAL", 1e-6, -3, "7.1 cascoded antilog, I_in = IIN (a = 0), v_x = -3 V_T, I_out = IIN e^-3 = IUo");
  A("CALb", "CAL", 1e-6 * std::exp(1.5), -4.5, "7.1 cascoded antilog with absorption a = +1.5: I_in = IIN e^1.5, v_x = -4.5 V_T, I_out = IUo");
  A("CALc", "CAL", 1e-6 * E3M, -3, "7.1 cascoded antilog with absorption a = -3: I_in = IIN e^-3, v_x = -3 V_T, I_out = IIN e^-6");
  A("CALt", "CAL", 7e-6, -3, "LOCK7 base T1 antilog: I_in = mT1 I_A = 7 uA, v_x = -3 V_T (|T1| max)");
  A("CALUa", "CALU", 1e-6, -3, "uncascoded antilog (CALU) into a TIA, I_in = IIN, v_x = -3 V_T (no Early correction)");
  { Cell x; x.name = "SUMa"; x.kind = "SUM"; x.in = {{-3, 1}, {-3, 1}, {-10, 0.3}}; x.what = "summer, noise gain 3.3: two LOGR outputs (-3 V_T, weight 1) + 3-neper constant from -V_NEP"; c.push_back(x); }
  { Cell x; x.name = "SUMb"; x.kind = "SUM"; x.in = {{-3, 2}, {-3, 3}, {-3, 4}, {10, 3}}; x.what = "summer, noise gain 13: weights 2, 3, 4 + 30-neper constant from +V_NEP (output -3 V_T)"; c.push_back(x); }
  { Cell x; x.name = "TIAa"; x.kind = "TIA"; x.I = 1e-6 * E3M; x.RT = 1.0 / (1e-6 * E3M); x.what = "readout TIA, I = IUo, R_T = 1 V/IUo"; c.push_back(x); }
  { Cell x; x.name = "VNEP"; x.kind = "VNEP"; x.what = "LOCK7 base PTAT neper generator (cells A 64 uA : 64/256 uA, B 32 uA : 32/256 uA, V_NEP = (10/ln 256)(2 dV_B - dV_A))"; c.push_back(x); }
  return c; }
// physics switches (env FSOT_PHYS of netgen9 = the comma list of mechanisms switched OFF; "all" = every one):
//   hi: IKF -> infinity;  rb: RB = 0;  ib: BF = 1e15, ISE = 0 (no base current);  early: VAF -> infinity, RC = 0;  gain: ideal op-amp gain 1e10 (not 1e6);
//   vnep: V_NEP from an ideal source 10 k T/q (leaf netlists).  Bench runs: "none" = all off, "only_<m>" = all off except m ("rbib" = rb and ib on), "full".
inline const std::vector<std::string> MECH = {"hi", "rb", "ib", "early", "gain"};
}
