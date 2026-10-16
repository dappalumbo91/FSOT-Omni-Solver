// tapc7.cpp — LOCK7: fork of tapc6.cpp (LOCK6, unchanged). Same taps and n; adds, per n, the Early-corrected divide-back input
//   R_ldi_n = R_lcT (1 + n V_T0 / VAF),  VAF = 74.03 V (2N3904 model), V_T0 = kT/q at 300.15 K:
// the divide-back Q4 has its base at -n V_T and its collector on the engine's virtual-ground summing node, V_CB = n V_T, so its current is
// high by (1 + n V_T/VAF) against the matched transdiode (V_CB = 0); the input current is lowered by that factor at the nominal temperature.
// tapc6.cpp — FSOT Omni-Solver Phase 3d (LOCK6): log-carry of the base-engine tiny taps. Deriver only (writes values; netgen5 builds the circuit).
// For every tap of every fold whose nominal term t (units of I_U = 5 uA; same formulas as netgen4 / taps5.tsv, here in long double from the locked
// phase2/locked/folds35_targets.tsv) is below 1e-4, the term is carried as a log-domain voltage and lifted by an exact integer number of nepers
//   n = floor(-3 - ln t)   (so the scaled term t e^n lies in (e^-4, e^-3], the LOCK5 headroom band of the compiled leaves),
// read at that scale on a TIA (R_lcT = 1 V / (I_U e^-3): 0.37..1 V), and divided back by e^-n in a matched antilog (base at -n V_T from the same
// PTAT bus) before it is injected into the engine's summing node. Constant resistor from the V_NEP = 10 V_T bus: R_lcn = R_SUM 10 / n.
// Charge-integrator cross-check (bench only): C_int = 1 nF, T_int = 1 s.
// Usage: tapc6 <folds35_targets.tsv> <outdir>
#include "../../cpp/design.hpp"
#include <cmath>
#include <cstdio>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
using R = long double;
static R fact(int n) { R r = 1; for (int i = 2; i <= n; ++i) r *= i; return r; }
int main(int argc, char** argv) {
  if (argc < 3) { std::fprintf(stderr, "usage: tapc7 folds35_targets.tsv outdir\n"); return 1; }
  const R VT0 = 1.380649e-23L * 300.15L / 1.602176634e-19L, VAF = 74.03L; const R IU = 5e-6L, RSUM = 10e3L, E3M = std::exp(R(-3)), RlcT = 1.0L / (IU * E3M);
  std::ifstream f(argv[1]); std::string line; std::getline(f, line); std::string out = argv[2];
  std::FILE* t = std::fopen((out + "/taps7.tsv").c_str(), "w"); std::fprintf(t, "fold\ttap\tterm_in_I_U\tln_term\tn_nepers\tscaled_term\tmargin_to_next_integer\tR_lcn\n");
  std::FILE* v = std::fopen((out + "/design7_taps_values.tsv").c_str(), "w"); std::fprintf(v, "name\tideal\tE24_a\tE24_b\tE24_c\tE24_rel\tunit\tmeaning\n");
  auto part = [&](const std::string& nm, R x, const char* u, const std::string& m) { auto e = ckt::realise(x); std::fprintf(v, "%s\t%.12Lg\t%.6Lg\t%.6Lg\t%.6Lg\t%.3Le\t%s\t%s\n", nm.c_str(), x, e.a, e.b, e.c, e.rel, u, m.c_str()); };
  part("R_lcT", RlcT, "ohm", "log-carry TIA feedback and divide-back input: 1 V per I_U e^-3");
  std::fprintf(v, "C_int\t1e-09\t-\t-\t-\t-\tF\tcharge-integrator capacitor (C0G), bench only\nT_int\t1\t-\t-\t-\t-\ts\tcharge-integration time, bench only\n");
  int nt = 0; std::vector<int> seen(64, 0);
  while (std::getline(f, line)) { std::istringstream s(line); std::vector<std::string> c; std::string x; while (std::getline(s, x, '\t')) c.push_back(x);
    const R y = std::stold(c[6]), w = std::stold(c[7]), vv = std::stold(c[8]), T3 = std::stold(c[13]);
    std::vector<std::pair<std::string, R>> tp;
    for (int k = 1; k <= 3; ++k) tp.push_back({"cy" + std::to_string(k), std::pow(y, 2 * k) / fact(2 * k)});
    for (int k = 1; k <= 3; ++k) tp.push_back({"sw" + std::to_string(k), std::pow(w, 2 * k) / fact(2 * k + 1)});
    for (int k = 1; k <= 4; ++k) tp.push_back({"sv" + std::to_string(k), std::pow(vv, 2 * k) / fact(2 * k + 1)});
    for (int k = 1; k <= 5; ++k) tp.push_back({"st" + std::to_string(k), R(1) / fact(2 * k + 1)});
    tp.push_back({"a3", std::fabs(T3)});
    for (auto& [nm, term] : tp) { if (!(term < 1e-4L)) continue;
      const R L = std::log(term), nr = -3 - L; const int n = (int)std::floor(nr); const R marg = std::min(nr - n, n + 1 - nr);
      const R Rn = RSUM * 10 / n; ++nt;
      std::fprintf(t, "%s\t%s\t%.12Le\t%.12Lf\t%d\t%.9Lf\t%.6Lf\t%.9Lg\n", c[0].c_str(), nm.c_str(), term, L, n, term * std::exp(R(n)), marg, Rn);
      if (n > 0 && n < 64 && !seen[n]++) { part("R_lcn_" + std::to_string(n), Rn, "ohm", "log-carry: " + std::to_string(n) + " nepers from V_NEP (R_SUM 10/n)");
        part("R_ldi_" + std::to_string(n), RlcT * (1 + n * VT0 / VAF), "ohm", "divide-back input, Early-corrected: R_lcT (1 + " + std::to_string(n) + " V_T/VAF)"); } } }
  std::fclose(t); std::fclose(v); std::printf("TAPC7_OK tiny_taps=%d\n", nt); return 0;
}
