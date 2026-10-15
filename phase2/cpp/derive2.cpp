// derive2.cpp — writes the LOCKED Phase-2 design: component values (exact + E24 realisation), summer weights,
// per-fold FSOT targets (every factor), readout targets and the domain margins of every log/antilog block.
// Run BEFORE any simulation; outputs are hashed into phase2/locked/SHA256SUMS and recorded in phase2/locked/LOCK.md.
#include "design2.hpp"
#include "../../cpp/design.hpp"  // ckt::realise (E24 series realisation, Phase 1, unchanged)
#include <cstdio>
using namespace d2;
int main(int argc, char** argv) {
  const std::string out = argc > 1 ? argv[1] : "../locked";
  Fsot s; Refs r; auto P = parts(s); auto C = consts(s);
  auto addp = [&](std::string n, R v, std::string m) { P.push_back({n, v, "ohm", m}); };
  auto wR = [&](R w) { return RSUM / w; };
  auto cR = [&](R c) { return RSUM * 10 / std::fabs(c); };
  // summer input resistors (weights) — inverting, junction scale
  for (std::string n : {"N", "P", "cos", "a4", "a5", "w", "sp"}) addp("R_T1_" + n, wR(1), "T1 summer: weight 1 on n_" + n);
  addp("R_T1_Db", wR(0.5L), "T1 summer: weight 1/2 on n_Db (D^-1/2)");
  addp("R_T1_rho", wR(R(1) / 10), "T1 summer: rho = 1 nepers from -V_NEP (weight rho/10)");
  addp("R_T1_dp", wR(s.BIN / 10), "T1 summer: B_in delta_psi from the -delta_psi V_NEP bus (weight B_in/10)");
  addp("R_T1_ahN", wR(R(1) / 10), "T1 summer: -alpha h/N from +alpha (h/N) V_NEP (weight 1/10)");
  addp("R_T1_c", cR(C["T1"]), "T1 summer constant (nepers): refs + D^-1/2 + antilog multiple");
  addp("R_T1_co", cR(C["T1obs"]), "T1 summer gated observer constant: C_F P_var + LOGR refs");
  addp("R_G_al", wR(s.GAM / (10 * s.PHI)), "growth summer: alpha gamma/phi from -alpha V_NEP");
  addp("R_G_ahN", wR(s.GAM / (10 * s.PHI)), "growth summer: -alpha gamma/phi h/N from +alpha (h/N) V_NEP");
  addp("R_G_c", cR(C["G"]), "growth summer constant -ln 2 (antilog input 2 C_eff I_U)");
  for (std::string n : {"sc", "am"}) addp("R_T2_" + n, wR(1), "T2 summer: weight 1 on n_" + n);
  addp("R_T2_c", cR(C["T2"]), "T2 summer constant");
  for (std::string n : {"v", "vp", "N", "P", "ch", "v2", "ac", "ph"}) addp("R_T3_" + n, wR(1), "T3 summer: weight 1 on n_" + n);
  addp("R_T3_Db", wR(0.5L), "T3 summer: weight 1/2 on n_Db");
  addp("R_T3_c", cR(C["T3"]), "T3 summer constant incl. ln beta = -(pi^pi + e - 1) nepers");
  for (int k = 1; k <= 3; ++k) { addp("R_cy" + std::to_string(k) + "_n", wR(2 * k), "cos-y power y^" + std::to_string(2 * k) + ": weight 2k on n_y"); addp("R_cy" + std::to_string(k) + "_c", cR(C["cy" + std::to_string(k)]), "cos-y power constant 2k ln r_y - ln (2k)!"); }
  for (int k = 1; k <= 3; ++k) { addp("R_sw" + std::to_string(k) + "_n", wR(2 * k), "observer sine power w^" + std::to_string(2 * k)); addp("R_sw" + std::to_string(k) + "_c", cR(C["sw" + std::to_string(k)]), "constant 2k ln r_w - ln (2k+1)!"); }
  for (int k = 1; k <= 4; ++k) { addp("R_sv" + std::to_string(k) + "_n", wR(2 * k), "T3 cos(dpsi) sine power v^" + std::to_string(2 * k)); addp("R_sv" + std::to_string(k) + "_c", cR(C["sv" + std::to_string(k)]), "constant"); }
  for (int k = 1; k <= 5; ++k) { addp("R_st" + std::to_string(k) + "_n", wR(2 * k), "acoustic sine power dt^" + std::to_string(2 * k)); addp("R_st" + std::to_string(k) + "_c", cR(C["st" + std::to_string(k)]), "constant"); }
  addp("R_s2_n", wR(2), "sin^2: weight 2 on n_t and n_tp"); addp("R_s2_c", cR(C["sin2"]), "sin^2 constant 4 ln 0.5 + ln((A_in-A_bleed)/phi)");
  addp("R_Om_n", wR(1), "Omega summer weight 1"); addp("R_Om_c", cR(C["Om"]), "Omega summer constant");
  addp("R_e1_c", cR(C["e1"]), "one e^1 stage: exponent (1 - ln 3) nepers (x e/3)");
  addp("R_em1_c", cR(C["em1"]), "e^-1 exponent: -1 neper");
  // write values
  std::FILE* f = std::fopen((out + "/design2_values.tsv").c_str(), "w");
  std::fprintf(f, "name\tideal\tE24_a\tE24_b\tE24_c\tE24_rel\tunit\tmeaning\n");
  for (auto& p : P) { auto e = ckt::realise(p.ideal); std::fprintf(f, "%s\t%.12Lg\t%.6Lg\t%.6Lg\t%.6Lg\t%.3Le\t%s\t%s\n", p.name.c_str(), p.ideal, e.a, e.b, e.c, e.rel, p.unit.c_str(), p.meaning.c_str()); }
  std::fclose(f);
  // constants + summer signs
  std::FILE* c = std::fopen((out + "/design2_consts.tsv").c_str(), "w");
  std::fprintf(c, "name\tnepers\tsource_bus\n");
  for (auto& [k, v] : C) std::fprintf(c, "%s\t%.15Le\t%s\n", k.c_str(), v, v < 0 ? "+V_NEP" : "-V_NEP");
  std::fclose(c);
  // per-fold targets (all 35) + domain margins of every log block (cascode 2N7000 needs n < 0; locked criterion n <= -8 mV at 27 C)
  std::FILE* t = std::fopen((out + "/folds35_targets.tsv").c_str(), "w");
  std::fprintf(t, "domain\tgen\tD\tlook\thits\tobserved\ty\tw\tv\ta2\tquirk\tT1\tT2\tT3\tlnT3\tS\n");
  std::FILE* m = std::fopen((out + "/domain_margins.tsv").c_str(), "w");
  std::fprintf(m, "domain\tblock\tIr_over_Ix\tn_mV_at_27C\tok\n");
  int g = 0, bad = 0;
  for (auto& grp : fsot::nest_generations()) { for (auto n : grp) { auto& d = s.F.domain(n);
    const R D = d.D_eff, dp = d.delta_psi, h = d.hits; const bool ob = d.observed;
    const R y = (s.PSI + dp) * (s.PI - 1) - s.PI, w = dp + s.PVAR - s.PI / 2, v = s.PI / 2 - dp;
    const R a2 = fsot::m::cos((s.PSI + dp) / s.ETA);
    const R q = ob ? fsot::m::exp(s.CF * s.PVAR) * fsot::m::cos(dp + s.PVAR) : R(1);
    auto si = s.F.default_input(); si.D_eff = D; si.delta_psi = dp; si.recent_hits = h; si.observed = ob;
    const R Seng = s.F.compute_scalar(si);
    const R gr = fsot::m::exp(s.ALPHA * (1 - h) * s.GAM / s.PHI);
    const R T1 = (1 / fsot::m::sqrt(D)) * a2 * fsot::m::exp(-s.ALPHA * h + 1 + s.BIN * dp) * (1 + gr * s.CEFF) * (1 + s.PNEW * fsot::m::ln(D / 25)) * q;
    const R T3 = s.BETA * fsot::m::cos(dp) / fsot::m::sqrt(D) * (1 + s.CHAOS * (D - 25) / 25) * s.valve2 * s.acoustic1 * s.phase1;
    std::fprintf(t, "%s\t%d\t%d\t%.12Lf\t%d\t%d\t%.12Lf\t%.12Lf\t%.12Lf\t%.15Le\t%.15Le\t%.15Le\t%.15Le\t%.15Le\t%.15Le\t%.15Le\n", std::string(n).c_str(), g, (int)D, dp, (int)h, (int)ob, y, w, v, a2, q, T1, R(1), T3, fsot::m::ln(T3), Seng);
    if (std::fabs(s.K * (T1 + 1 + T3) - Seng) > 1e-15L) { std::fprintf(stderr, "factor rebuild mismatch %s\n", std::string(n).c_str()); return 2; }
    auto chk = [&](const char* b, R ratio) { const R nm = VT0 * std::log(ratio) * 1000; const bool ok = nm <= -8; if (!ok) ++bad; std::fprintf(m, "%s\t%s\t%.6Lf\t%.3Lf\t%d\n", std::string(n).c_str(), b, ratio, nm, (int)ok); };
    const R ps = 1 - w * w / 6 + w * w * w * w / 120 - std::pow(w, 6) / 5040;
    const R cy = 1 - y * y / 2 + y * y * y * y / 24 - std::pow(y, 6) / 720;
    chk("Db", D * ISTEP / r.Db); chk("cos", r.cosy / cy); chk("y", r.y / std::fabs(y)); chk("a4", r.a4 / (1 + gr * s.CEFF));
    chk("a5", r.a5 / (1 + s.PNEW * std::log(D / 25)));
    if (ob) { chk("w", r.w / std::fabs(w)); chk("sp", r.sp / ps); }
    chk("v", r.v / v); chk("ch", r.ch / (1 + s.CHAOS * (D - 25) / 25));
    chk("T1_antilog", std::fabs(T1) / r.mT1); chk("T3_antilog", T3);
  } ++g; }
  { auto chk2 = [&](const char* b, R ratio) { const R nm = VT0 * std::log(ratio) * 1000; const bool ok = nm <= -8; if (!ok) ++bad; std::fprintf(m, "*\t%s\t%.6Lf\t%.3Lf\t%d\n", b, ratio, nm, (int)ok); };
    chk2("N", r.N); chk2("v2", r.v2 / s.valve2); chk2("ac", r.ac / s.acoustic1); chk2("ph", r.ph / s.phase1); chk2("t", r.t); chk2("tp", r.tp / std::sin(1.0L));
    chk2("T2_antilog", 1 / r.mT2); chk2("growth_antilog", fsot::m::exp(s.ALPHA * s.GAM / s.PHI) / r.mG); chk2("e1_stage", s.E / 4); chk2("PTAT", 1 / MPT);
    chk2("T1_antilog_max", 2.5534867L / r.mT1);
    chk2("oS", r.oS / std::fabs(s.F.S_COSM)); chk2("o1", r.o1 / (10 * (1 - s.F.S_CHEM))); chk2("Om_antilog", 10 * std::fabs(s.F.S_COSM) * (1 - s.F.S_CHEM)); }
  std::fclose(t); std::fclose(m);
  std::FILE* q = std::fopen((out + "/targets2.tsv").c_str(), "w");
  std::fprintf(q, "name\tvalue\tsource\n");
  auto row = [&](const char* n, R v, const char* src) { std::fprintf(q, "%s\t%.15Le\t%s\n", n, v, src); };
  row("K", s.K, "engine.hpp:137"); row("S_chem", s.F.S_CHEM, "engine.hpp:148"); row("S_cosm", s.F.S_COSM, "engine.hpp:146"); row("S_quant", s.F.S_QUANT, "engine.hpp:147");
  row("one_minus_S_chem", 1 - s.F.S_CHEM, "1 - S_chem");
  row("Omega_b_h2", fsot::m::fabs(s.F.S_COSM) * (1 - s.F.S_CHEM), "closed_forms.gen.inc:15-16");
  row("alpha_s(M_Z)", 1 / (s.E * s.PI), "closed_forms.gen.inc:7-8");
  row("1/alpha_em", fsot::m::ipow(s.E, 3) * fsot::m::ipow(s.PHI, 4) - s.PSI, "closed_forms.gen.inc:70-71");
  row("e3", fsot::m::ipow(s.E, 3), "e^3");
  row("V_NEP_27C", 10 * VT0, "10 kT/q at 300.15 K");
  row("valve2", s.valve2, "engine.hpp:214-215"); row("acoustic1", s.acoustic1, "engine.hpp:216-217"); row("phase1", s.phase1, "engine.hpp:218");
  std::fclose(q);
  std::printf("DERIVE2_OK parts=%zu domain_violations=%d\n", P.size(), bad);
  return bad ? 3 : 0;
}
