// cellpred.cpp — LOCK10A: closed-form predictions of every isolated cell's error (cells10.hpp), per mechanism, from the 2N3904 Gummel-Poon card
// netgen uses (IS 6.734f, BF 416.4, NE 1.259, ISE 6.734f, IKF 66.78m, VAF 74.03, RB 10, RC 1, BR 0.7371) and the ideal op-amp gain A = 1e6.
// Written and hashed BEFORE the bench runs. Each mechanism is predicted alone (all others off), as the bench's "only_<m>" runs minus "none":
//   none : the residual with every mechanism off: the "-1" of I_S(e^x - 1) and, for CALU, the reverse B-C current I_S/BR (e^{V_BC/V_T} - 1)
//   hi   : q_b = 1 + I_C/IKF exactly at V_BC = 0  -> LOGR h = ln((1+I2/K)/(1+I1/K)); CAL -ln((1+Io/K)/(1+Ii/K))
//   rbib : base current through RB: V_B'E = V_BE - I_B RB, I_B = I_C/BF + ISE((I_C/I_S + 1)^{1/NE} - 1) -> LOGR (I_B2 - I_B1) RB/V_T, CAL (I_B3 - I_B4) RB/V_T
//   ib   : base current without RB: 0 (every collector current is set by its node; bases are driven)
//   early: VAF with RC: V_B'C' = I_C RC (cascoded / transdiode), V_x + I_C RC (CALU) -> ln(q1 ratio), q1 = 1/(1 - V_B'C'/VAF)
//   round: netlist print rounding (netgen8: 6 significant digits); 0 for the bench cells (17-digit values), VNEP: the base resistors as printed
//   gain : finite A = 1e6: virtual-ground offsets V_BE/A change resistor-defined input currents, the TIA reads A/(1+A), a summer
//          V_o = -sum G_i V_i / (G_f + (G_f + sum G_i)/A)
// VNEP: e = (V_NEP - 10 V_T)/(10 V_T) from the LOGR errors of cells A, B and the generator's summer and inverter (closed form).
// Usage: cellpred <out.tsv>
#include "cells10.hpp"
#include <cmath>
#include <cstdio>
#include <functional>
#include <string>
using R = long double;
static const R IS = 6.734e-15L, BF = 416.4L, NE = 1.259L, ISE = 6.734e-15L, K = 66.78e-3L, VAF = 74.03L, RB = 10, RC = 1, BR = 0.7371L, A = 1e6L;
static const R VT = (R)1.38064852e-23L / 1.6021766208e-19L * 300.15L;
static R Ib(R Ic) { return Ic / BF + ISE * (std::pow(Ic / IS + 1, 1 / NE) - 1); }
static R vbe(R Ic) { return VT * std::log(Ic / IS + 1); }
// LOGR error per mechanism (I1 transdiode c1 from Vs1, I2 cascoded c2 from Vs2)
static R logr(const std::string& m, R I1, R I2, R Vs1, R Vs2) {
  if (m == "none") return std::log((I2 + IS) / (I1 + IS)) - std::log(I2 / I1);
  if (m == "hi") return std::log((1 + I2 / K) / (1 + I1 / K));
  if (m == "rbib") return (Ib(I2) - Ib(I1)) * RB / VT;
  if (m == "ib") return 0;
  if (m == "early") return std::log((1 - I1 * RC / VAF) / (1 - I2 * RC / VAF));
  if (m == "gain") { R n = VT * std::log(I2 / I1); return std::log(1 - n / (A * Vs2)) - std::log(1 - vbe(I1) / (A * Vs1)); }
  return NAN; }
static R cal(const std::string& m, R Ii, R vx, R Vs, bool unc) {
  const R v = vx, Io0 = Ii * std::exp(v);
  if (m == "none") { if (!unc) return std::log((Ii + IS) * std::exp(v) - IS) - std::log(Ii) - v;
    return std::log(1 + IS * (1 - std::exp(v)) / (BR * Ii * std::exp(v))); }
  if (m == "hi") { R Io = Io0; for (int i = 0; i < 50; ++i) Io = Ii * std::exp(v) * (1 + Ii / K) / (1 + Io / K); return std::log(Io / Ii) - v; }
  if (m == "rbib") return (Ib(Ii) - Ib(Io0)) * RB / VT;
  if (m == "ib") return 0;
  if (m == "early") { const R vbc4 = unc ? v * VT + Io0 * RC : Io0 * RC; return std::log((1 - vbc4 / VAF) / (1 - Ii * RC / VAF)); }
  if (m == "gain") return std::log(1 - vbe(Ii) / (A * Vs)) + std::log(A / (1 + A));
  return NAN; }
int main(int argc, char** argv) {
  if (argc < 2) return 2;
  std::FILE* o = std::fopen(argv[1], "w"); std::fprintf(o, "cell\tmechanism\tpredicted_e\tunit\n");
  const char* MS[] = {"none", "hi", "rbib", "ib", "early", "gain", "round"};
  for (auto& c : c10::cells()) {
    for (const char* m : MS) { R e = 0; const char* u = "neper";
      if (c.kind == "LOGR") e = logr(m, c.I1, c.I2, c.ptat1 ? 10 * VT : c10::VREF, c10::VREF);
      else if (c.kind == "CAL" || c.kind == "CALU") e = cal(m, c.I1, c.vx, c10::VREF, c.kind == "CALU");
      else if (c.kind == "SUM") { if (std::string(m) == "gain") { R num = 0, gs = 0, Gf = 1 / (R)c10::RSUM; for (auto& x : c.in) { R G = x.w / (R)c10::RSUM; num += G * x.v_nep * VT; gs += G; }
          R vo = -num / (Gf + (Gf + gs) / A), vi = -num / Gf; e = (vo - vi) / VT; } }
      else if (c.kind == "TIA") { if (std::string(m) == "gain") e = std::log(A / (1 + A)); }
      if (std::string(m) == "round" && c.kind != "VNEP") e = 0;
      else if (c.kind == "VNEP") { u = "relative";
        const R M = 256, IA1 = 64e-6L, IB1 = 32e-6L, wB = 20 / std::log(M), wA = 10 / std::log(M);
        const R eA = logr(m, IA1, IA1 / M, c10::VREF, c10::VREF), eB = logr(m, IB1, IB1 / M, c10::VREF, c10::VREF);
        R nA = VT * (-std::log(M) + eA), nB = VT * (-std::log(M) + eB);
        if (std::string(m) == "gain") { const R Gf = 1e-4L, GB = wB * Gf, GA = wA * Gf; const R nAn = -nA / (1 + 2 / A);
          const R vn = -(GB * nB + GA * nAn) / (Gf + (Gf + GA + GB) / A); e = vn / (10 * VT) - 1; }
        else if (std::string(m) == "round") { // netgen8 prints the generator resistors at 6 significant digits (%g)
          auto pr = [](R x) { char b[64]; std::snprintf(b, sizeof b, "%g", (double)x); return std::stold(b); };
          const R RgB = 10e3L / wB, RgA = 10e3L / wA, Rah = 2.5L / IA1, Ral = 2.5L / (IA1 / M), Rbh = 2.5L / IB1, Rbl = 2.5L / (IB1 / M);
          const R eA2 = std::log((Rah / pr(Rah)) / (Ral / pr(Ral))) * -1, eB2 = std::log((Rbh / pr(Rbh)) / (Rbl / pr(Rbl))) * -1;   // n/V_T = ln(I2/I1), I = 2.5/R
          const R wBp = 10e3L / pr(RgB), wAp = 10e3L / pr(RgA);
          e = (-(wBp * (-std::log(M) + eB2) - wAp * (-std::log(M) + eA2))) / 10 - 1; }
        else e = -(wB * eB - wA * eA) / 10; }
      std::fprintf(o, "%s\t%s\t%.6Le\t%s\n", c.name.c_str(), m, e, u); } }
  std::fclose(o); std::printf("CELLPRED_OK\n"); return 0; }
