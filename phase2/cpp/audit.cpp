// audit.cpp — Phase 2 completeness audit: dump every seed, layer constant and every factor of the core law
// S = K (T1 + T2 + T3) for all 35 folds, straight from the pinned engine (FSOT-2.1-Cpp @3466e8d, engine.hpp).
// No value is computed outside the engine's own expressions except the factor split of T1/T3 (same operations, same order).
#include "fsot/engine.hpp"
#include <cstdio>
#include <string>
using R = long double; namespace m = fsot::m;
int main(int argc, char** argv) {
  const std::string out = argc > 1 ? argv[1] : "../results";
  fsot::Engine<R> F;
  FILE* c = std::fopen((out + "/audit_constants.tsv").c_str(), "w");
  std::fprintf(c, "name\tvalue\tsource\n");
  auto P = [&](const char* n, R v, const char* s) { std::fprintf(c, "%s\t%.18Le\t%s\n", n, v, s); };
  P("PI", F.PI, "engine.hpp:111"); P("E", F.E, "engine.hpp:112"); P("PHI", F.PHI, "engine.hpp:113"); P("GAMMA", F.GAMMA, "engine.hpp:114"); P("G_CAT", F.G_CAT, "engine.hpp:115");
  P("ALPHA", F.ALPHA, "engine.hpp:117"); P("PSI_CON", F.PSI_CON, "engine.hpp:118"); P("ETA_EFF", F.ETA_EFF, "engine.hpp:119"); P("BETA", F.BETA, "engine.hpp:120");
  P("GAMMA_C", F.GAMMA_C, "engine.hpp:121"); P("OMEGA", F.OMEGA, "engine.hpp:122"); P("THETA_S", F.THETA_S, "engine.hpp:123"); P("POOF", F.POOF, "engine.hpp:124");
  P("C_EFF", F.C_EFF, "engine.hpp:126-127"); P("A_BLEED", F.A_BLEED, "engine.hpp:128"); P("P_VAR", F.P_VAR, "engine.hpp:129"); P("B_IN", F.B_IN, "engine.hpp:130");
  P("A_IN", F.A_IN, "engine.hpp:131"); P("SUCTION", F.SUCTION, "engine.hpp:132"); P("CHAOS", F.CHAOS, "engine.hpp:133"); P("P_BASE", F.P_BASE, "engine.hpp:134");
  P("P_NEW", F.P_NEW, "engine.hpp:135"); P("C_FACTOR", F.C_FACTOR, "engine.hpp:136"); P("K", F.K, "engine.hpp:137"); P("C_COSM", F.C_COSM, "engine.hpp:138");
  std::fclose(c);
  FILE* d = std::fopen((out + "/audit_folds.tsv").c_str(), "w");
  std::fprintf(d, "domain\tgen\tx=5*5^(g/19)\tD_eff\tfrac(x)\tdist_to_half\tlook\thits\tobserved\ta1=NP/sqrtD\ta2=cos((psi+dpsi)/eta)\ta3=exp(-a*h/N+rho+Bin*dpsi)\tgrowth\ta4=1+Ceff*growth\ta5=1+Pnew*ln(D/25)\tquirk_exp\tquirk_cos\tquirkMod\tT1\tT2\tT3\tS_engine\tS_check\tT3/S\n");
  const auto& gens = fsot::nest_generations();
  for (int g = 0; g < (int)gens.size(); ++g) for (auto n : gens[g]) {
    const auto& dc = F.domain(n); auto s = F.default_input(); s.D_eff = dc.D_eff; s.delta_psi = dc.delta_psi; s.recent_hits = dc.hits; s.observed = dc.observed; s.delta_theta = 1;
    const R D = s.D_eff, dp = s.delta_psi, h = s.recent_hits, N = s.N, Pp = s.P;
    const R a1 = N * Pp / m::sqrt(D), a2 = m::cos((s.psi_con + dp) / F.ETA_EFF), a3 = m::exp(-s.alpha * h / N + s.rho + s.B_in * dp);
    const R gr = m::exp(s.alpha * (R(1) - h / N) * F.GAMMA / F.PHI), a4 = R(1) + gr * s.C_eff, a5 = R(1) + s.P_new * m::ln(D / R(25));
    const R qe = dc.observed ? m::exp(F.C_FACTOR * s.P_var) : R(1), qc = dc.observed ? m::cos(dp + s.P_var) : R(1);
    const R T1 = a1 * a2 * a3 * a4 * a5 * qe * qc, T2 = s.scale * s.amplitude + s.trend_bias;
    const R valve = s.beta * m::cos(dp) * (N * Pp / m::sqrt(D)) * (R(1) + s.chaos * (D - R(25)) / R(25)) * (R(1) + s.poof * m::cos(s.theta_s + F.PI) + s.suction * m::sin(s.theta_s));
    const R acoustic = R(1) + (s.A_bleed * m::sin(R(1)) * m::sin(R(1))) / F.PHI + (s.A_in * m::cos(R(1)) * m::cos(R(1))) / F.PHI;
    const R T3 = valve * acoustic * (R(1) + s.B_in * s.P_var);
    const R Se = F.compute_scalar(s), Sc = F.K * (T1 + T2 + T3);
    const R x = 5 * std::pow(5.0L, R(g) / 19), fr = x - std::floor(x);
    std::fprintf(d, "%s\t%d\t%.12Lf\t%d\t%.6Lf\t%.6Lf\t%.12Lf\t%d\t%d\t%.15Le\t%.15Le\t%.15Le\t%.15Le\t%.15Le\t%.15Le\t%.15Le\t%.15Le\t%.15Le\t%.15Le\t%.15Le\t%.6Le\t%.15Le\t%.15Le\t%.3Le\n",
      std::string(n).c_str(), g, x, dc.D_eff, fr, std::fabs(fr - 0.5L), dp, (int)h, (int)dc.observed, a1, a2, a3, gr, a4, a5, qe, qc, qe * qc, T1, T2, T3, Se, Sc, T3 / Se);
  }
  std::fclose(d); std::puts("AUDIT_DUMP_OK");
}
