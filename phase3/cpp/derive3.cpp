// derive3.cpp — FSOT Omni-Solver Phase 3, leaf tier 1: the closed forms fed directly by S_cosm, S_quant, S_chem
// (FSOT-2.1-Cpp 3466e8d closed_forms_core.gen.inc wave1/2/3/5). Each leaf is  b + k*x  with x one of the circuit's own
// S-derived signals (|S_cosm|, S_quant, 1-S_chem, 1-S_quant, |S_cosm|*S_quant); b and k are FSOT constants from the engine.
// Writes the LOCKED values (exact + E24) BEFORE any simulation: phase3/locked/design3_values.tsv, design3_consts.tsv,
// leaves3_targets.tsv. No FSOT rule is added or changed: every leaf value is cross-checked against the authority golden table.
#include "../../phase2/cpp/design2.hpp"
#include "../../cpp/design.hpp"
#include <cstdio>
#include <fstream>
#include <sstream>
using namespace d2;
int main(int argc, char** argv) {
  const std::string out = argc > 1 ? argv[1] : "../locked", golden = argc > 2 ? argv[2] : "";
  Fsot s; auto& F = s.F; const R RF = 100e3L;
  const R absSc = fsot::m::fabs(F.S_COSM), Sq = F.S_QUANT, oSc = 1 - F.S_CHEM, oSq = 1 - F.S_QUANT;
  const R PB = F.P_BASE, CC = F.C_COSM, G = F.G_CAT, lnpi = fsot::m::ln(s.PI), lnphi = fsot::m::ln(s.PHI);
  // circuit signals: node, volts per unit (sign included)
  struct X { const char* node; R s; R val; };
  std::map<std::string, X> xs = {{"absSc", {"vabs", 2, absSc}}, {"Sq", {"vSq", 2, Sq}}, {"oSc", {"v1sc", -40, oSc}}, {"oSq", {"v1sq", -40, oSq}}, {"pSq", {"vprod", 10, absSc * Sq}}};
  struct L { const char* id; const char* name; R b, k; const char* x; };
  std::vector<L> Ls = {
    {"H0", "H0", 100, -100 * s.ABL / s.AIN, "absSc"},                       // 100 (1 + S_cosm A_bleed/A_in), S_cosm < 0
    {"TCMB", "T_CMB", s.PHI * s.PHI, PB, "absSc"},
    {"ns", "n_s", 1, -CC * fsot::m::pow(s.PHI, R(1) / s.PI), "absSc"},          // 1 + S_cosm C_cosm phi^(1/pi)
    {"MWZ", "M_W/M_Z", -PB, lnpi, "Sq"},
    {"OL", "Omega_Lambda", s.GAM * s.GAM, 1 / s.E, "Sq"},
    {"ODM", "Omega_DM_h2", 0, s.PHI * s.AIN, "oSc"},
    {"s8", "sigma_8", fsot::m::fabs(s.CHAOS), 1, "pSq"},
    {"Vus", "|V_us|", s.THS / s.PHI, 1, "oSq"},
    {"Vcb", "|V_cb|", 0, 1 / s.CEFF - 1, "Sq"},
    {"Is2", "Ising2D_gamma", lnpi, 1 / lnphi, "oSq"},
    {"mtW", "m_t/m_W", s.PSI, 1 / fsot::m::fabs(s.CHAOS), "absSc"},
    {"mHW", "m_H/m_W", 0, s.PSI + 1, "Sq"},
    {"C2", "Twin_prime_C2", fsot::m::pow(s.GAM, R(4)) / fsot::m::pow(G, R(4)), 1, "absSc"}};
  std::map<std::string, R> gold; if (!golden.empty()) { std::ifstream g(golden); std::string l;
    while (std::getline(g, l)) { std::istringstream t(l); std::string a, n, v; std::getline(t, a, '\t'); std::getline(t, n, '\t'); std::getline(t, v, '\t'); if (a.rfind("sec.", 0) == 0) gold[n] = std::stold(v); } }
  std::vector<Part> P; auto add = [&](std::string n, R v, std::string m) { P.push_back({n, v, "ohm", m}); };
  add("R_Lf", RF, "leaf summer feedback");
  std::FILE* t = std::fopen((out + "/leaves3_targets.tsv").c_str(), "w");
  std::fprintf(t, "id\tname\tvalue\tgolden\trel_vs_golden\tx\tx_node\tx_V_per_unit\tG_V_per_unit\tconst_bus\tb\tk\n");
  int bad = 0;
  for (auto& l : Ls) { const X& x = xs.at(l.x); const R v = l.b + l.k * x.val;
    R g = 1; while (fsot::m::fabs(v) * g * 10 < 8) g *= 10; while (fsot::m::fabs(v) * g >= 8) g /= 10;
    const R sg = -((x.s * l.k) > 0 ? 1 : -1); const R Gs = sg * g;                       // inverting summer: sign(G) = -sign(s k)
    add(std::string("R_L") + l.id + "_x", RF * fsot::m::fabs(x.s) / (g * fsot::m::fabs(l.k)), std::string(l.name) + ": x input (" + l.x + ")");
    std::string bus = "none";
    if (l.b != 0) { bus = (Gs * l.b > 0) ? "vrn" : "vr"; add(std::string("R_L") + l.id + "_c", RF * VR / fsot::m::fabs(Gs * l.b), std::string(l.name) + ": constant b from " + bus); }
    const R gv = gold.count(l.name) ? gold[l.name] : NAN; const R rel = (v - gv) / gv; if (!(fsot::m::fabs(rel) < 1e-15L)) ++bad;
    std::fprintf(t, "%s\t%s\t%.15Lg\t%.15Lg\t%.2Le\t%s\t%s\t%.6Lg\t%.6Lg\t%s\t%.15Lg\t%.15Lg\n", l.id, l.name, v, gv, rel, l.x, x.node, x.s, Gs, bus.c_str(), l.b, l.k); }
  // Omega_b_h2 (Phase 2 block) listed for completeness
  { const R v = absSc * oSc; const R gv = gold["Omega_b_h2"]; std::fprintf(t, "Omb\tOmega_b_h2\t%.15Lg\t%.15Lg\t%.2Le\tpSc\tvomb\t-\t100\tnone\t0\t1\n", v, gv, (v - gv) / gv); }
  std::fclose(t);
  // product block |S_cosm| * S_quant (same topology as the Phase-2 Omega_b h^2 block): LOGR(|S| from vabs), LOGR(S_quant), sum, CAL, TIA
  const R rS = 0.36L, rq = 0.6L;   // references: n <= -8 mV over the domain (|S_cosm| 0.502, S_quant 0.950)
  add("R_pS", KS / IU, "|S_cosm| (2 V/unit) -> I_U per unit"); add("R_pSr", 500e3L / rS, "LOGR_pS reference");
  add("R_pq", KS / IU, "S_quant (2 V/unit) -> I_U per unit"); add("R_pqr", 500e3L / rq, "LOGR_pq reference");
  add("R_p_n", RSUM, "product summer weight 1"); const R cP = fsot::m::ln(rS) + fsot::m::ln(rq); add("R_p_c", RSUM * 10 / fsot::m::fabs(cP), "product summer constant");
  add("R_pin", 500e3L, "product antilog input I_U"); add("R_pT", 10 / IU, "product TIA: 10 V per unit");
  std::FILE* f = std::fopen((out + "/design3_values.tsv").c_str(), "w");
  std::fprintf(f, "name\tideal\tE24_a\tE24_b\tE24_c\tE24_rel\tunit\tmeaning\n");
  for (auto& p : P) { auto e = ckt::realise(p.ideal); std::fprintf(f, "%s\t%.12Lg\t%.6Lg\t%.6Lg\t%.6Lg\t%.3Le\t%s\t%s\n", p.name.c_str(), p.ideal, e.a, e.b, e.c, e.rel, p.unit.c_str(), p.meaning.c_str()); }
  std::fclose(f);
  std::FILE* c = std::fopen((out + "/design3_consts.tsv").c_str(), "w"); std::fprintf(c, "name\tnepers\tsource_bus\nP\t%.15Le\t%s\n", cP, cP < 0 ? "+V_NEP" : "-V_NEP"); std::fclose(c);
  std::printf("DERIVE3_OK leaves=%zu parts=%zu golden_mismatch=%d\n", Ls.size(), P.size(), bad);
  return bad ? 1 : 0;
}
