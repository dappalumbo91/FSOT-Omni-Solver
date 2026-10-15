// derive.cpp — writes the locked design (component values + FSOT targets). Run BEFORE any simulation.
#include "design.hpp"
#include <cstdio>
#include <fstream>
using namespace ckt;
int main(int argc, char** argv) {
  const std::string out = argc > 1 ? argv[1] : "../locked";
  Fsot s;
  std::FILE* t = std::fopen((out + "/fsot_targets.tsv").c_str(), "w");
  std::fprintf(t, "# FSOT targets from FSOT-2.1-Cpp@3466e8d engine (long double). Source rows: closed_forms.gen.inc\nname\tvalue\tsource\n");
  auto row = [&](const char* n, R v, const char* src) { std::fprintf(t, "%s\t%.15Le\t%s\n", n, v, src); };
  row("K", s.K, "engine.hpp:137"); row("P_new", s.PNEW, "engine.hpp:135"); row("psi_con", s.PSI, "engine.hpp:118");
  row("A_med", s.A_med, "engine.hpp:206-208 (look=1,hits=0)"); row("O_obs", s.O_obs, "engine.hpp:210");
  row("c_med", s.c_med, "K*A/5"); row("c_obs", s.c_obs, "K*A*O/5");
  row("T3_rel_max", s.T3_rel_max, "engine.hpp:214-219, max over 35 folds");
  row("S_quant", s.S_QM, "engine.hpp:147 (QM, D=5, observed)"); row("S_chem", s.S_CH, "engine.hpp:148 (Chemistry, D=6, observed)");
  row("S_cosm", s.S_CO, "engine.hpp:146 (Cosmology, D=25, medium)");
  row("f_D5", s.f(5), "f(D)"); row("f_D6", s.f(6), "f(D)"); row("f_D25", s.f(25), "f(D)");
  row("alpha_s(M_Z)", s.alpha_s, "closed_forms.gen.inc:7-8"); row("1/alpha_em", s.inv_alpha, "closed_forms.gen.inc:70-71");
  row("Omega_b_h2", s.Omb, "closed_forms.gen.inc:15-16"); row("m_H/m_W", s.mHmW, "closed_forms.gen.inc:121-122");
  row("V_T0", VT0, "kB*300.15/q (SI 2019)");
  // check: core-law reduction reproduces the engine for every look=1 fold
  R maxdev = 0;
  for (auto& d : s.F.DOMAINS) if (d.delta_psi == R(1) && d.hits == 0) maxdev = std::max(maxdev, std::fabs(s.S_of(d.D_eff, d.observed) - s.F.domain_scalar(d.name)));
  row("reduction_maxdev", maxdev, "|K + c f(D) - engine S|, 33 look=1 folds");
  std::fclose(t);
  // fold table
  std::FILE* fo = std::fopen((out + "/folds.tsv").c_str(), "w");
  std::fprintf(fo, "domain\tgen\tD_eff\tobserved\tlook\thits\tS_engine\n");
  int g = 0; for (auto& grp : fsot::nest_generations()) { for (auto n : grp) { auto& d = s.F.domain(n); std::fprintf(fo, "%s\t%d\t%d\t%d\t%.6Lf\t%d\t%.15Lf\n", std::string(n).c_str(), g, d.D_eff, (int)d.observed, d.delta_psi, d.hits, s.F.domain_scalar(n)); } ++g; }
  std::fclose(fo);
  // rounding thresholds and margins
  std::FILE* th = std::fopen((out + "/ladder_thresholds.tsv").c_str(), "w");
  std::fprintf(th, "k\tg_k\tV_tap\tnearest_int_g\tmargin_gen\tmargin_mV\n");
  for (int k = 5; k <= 24; ++k) { R gk = s.gk[k - 5]; R ng = std::round(gk); std::fprintf(th, "%d\t%.9Lf\t%.9Lf\t%d\t%.6Lf\t%.3Lf\n", k, gk, gk * KG, (int)ng, std::fabs(gk - ng), 1000 * std::fabs(gk - ng) * KG); }
  std::fprintf(th, "# direct (x-domain) rounding margins: g, x = 5*5^(g/19), D, |x - (D +- 1/2)|/x\n");
  for (int gg = 0; gg < 20; ++gg) { R x = s.xgen[gg]; R m = std::fabs(std::fabs(x - std::round(x)) - 0.5L); std::fprintf(th, "#x\t%d\t%.6Lf\t%d\t%.5Lf\n", gg, x, s.Dgen[gg], m / x); }
  std::fclose(th);
  // parts
  std::FILE* pf = std::fopen((out + "/design_values.tsv").c_str(), "w");
  std::fprintf(pf, "name\tideal\tE24_a\tE24_b\tE24_c\trealised\trel_err\tunit\tmeaning\n");
  for (auto& p : parts(s)) { auto r = realise(p.ideal); std::fprintf(pf, "%s\t%.9Lg\t%.6Lg\t%.6Lg\t%.6Lg\t%.9Lg\t%.3Le\t%s\t%s\n", p.name.c_str(), p.ideal, r.a, r.b, r.c, r.val, r.rel, p.unit.c_str(), p.meaning.c_str()); }
  std::fclose(pf);
  std::printf("S_QM %.12Lf S_CH %.12Lf S_CO %.12Lf maxdev %.3Le T3rel %.3Le\n", s.S_QM, s.S_CH, s.S_CO, maxdev, s.T3_rel_max);
}
