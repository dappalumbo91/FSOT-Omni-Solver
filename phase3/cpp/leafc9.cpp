// leafc9.cpp — FSOT Omni-Solver LOCK9: fork of leafc8b.cpp (Amendment 8.1, unchanged; built with detmath.cpp). Adds switches that revert
// one LOCK7/7.1 compiler change at a time to its LOCK5 form (ablation; env FSOT_L9_ABL = comma list, recorded in phase3/locked9a/LOCK9A.md):
//   flip  : signal LOGRs in the LOCK5 orientation (signal on the transdiode c1 at IIN, reference on the cascoded c2 at IIN e^-3; t = -1)
//   abs   : no antilog input absorption (a = 0, I_in = IIN)
//   base  : constant-generator base cells at m c >= 1.4 (LOCK5) instead of m c >= e
//   early : no Early correction of uncascoded antilog inputs
//   rho   : no range sizing (rho = 1)
// Output: suffix 9 (design9_, recipe9_, leaves9_targets, generator9, netlists9).
// leafc8b.cpp — FSOT Omni-Solver LOCK8 Amendment 8.1 (host-independent derivation): fork of leafc8.cpp (LOCK8, unchanged). Recorded in
// phase3/locked8b/LOCK8b.md. Built and linked ONLY with phase3/cpp/detmath.cpp (deterministic expl/logl/powl/sinl/cosl/sincosl/acosl/log10l/pow
// using IEEE + - * / only), so no value depends on the CPU's x87 transcendental microcode or on the glibc version. Two output changes:
//  (a) each part's ideal value is snapped to the 12 significant digits it is printed with (decimal round trip) BEFORE the E24 realisation, so
//      cpp/design.hpp realise() decides "best.rel > 5e-4" on the printed value: the parts 10000/0.15 = 66666.666... sat exactly on that
//      threshold (two E24 pieces 62000 + 4700 give rel = 5.000e-4) and took 2 or 3 pieces depending on the last bit (CI runs 38823805490,
//      38824108037: locked7c/design7_2_M_values.tsv line 595, P_zS131xc). On the printed value they always take 2 pieces (rel 4.99999999997e-4).
//  (b) node values in leaves8_targets.tsv (engine column) and generator8.tsv (engine column, the GEN scoring target) are printed with 12
//      significant digits instead of 18 (18 digits exposed 1-ulp differences; a 1-ulp exp/log perturbation moved Chain_consistency_%, a
//      cancellation with condition ~1e4, in digit 15; scoring needs < 1e-6). Golden values read from golden_AEB2AD.tsv keep 21 digits; the 16
//      partC targets (computed here, not read) are printed with 12 (an -O0 build moved gamma_rel by 1 ulp in digit 20).
// leafc8.cpp — FSOT Omni-Solver LOCK8 (leaves, part C): fork of leafc7c.cpp (LOCK7 Amendment 7.2, unchanged). Additions only; every LOCK7c
// netlist is unchanged (checked by verify_lock8.sh). Recorded in phase3/locked8/LOCK8.md:
//  (1) domain_scalar("X") is the S output of a full-core-law fold channel for domain X (recipe line DCHAN <prefix> <domain>, netgen7), V = 2 S,
//      exactly as S_COSM/S_QUANT/S_CHEM in T1. The 13 domain_scalar forms (7 domains) compile into netlist C_S.
//  (2) fsot_correct(m, X) = m (1 + |S_X| ALPHA) (engine.hpp:240, Ledger B correction, not a prediction): the factor F_X = 1 + |S_X| ALPHA for the
//      7 domains above plus Cosmology, Quantum_Mechanics, Chemistry (netlist C_S; m is the measured input, a resistor gain, unit here).
//  (3) hub dynamics coefficients (vendor/fsot_dynamics.py, sha256 837817ba..., not the pinned engine): gamma_rel, kappa, mu(D_eff of
//      Fluid_Dynamics), J_obs (observed, delta_psi = 1), c_s^2 (rho = 1), netlist C_H; targets are the same formulas in long double on the engine constants.
// Output files: suffix 8 (design8_, recipe8_, leaves8_targets, generator8, netlists8) in phase3/locked8.
// leafc7c.cpp — LOCK7 Amendment 7.2: fork of leafc7b.cpp (Amendment 7.1, unchanged). Reproducibility only: the antilog input exponent a is
// snapped to exactly 0 when |a| < 1e-12 (a = vx0 + 3 = 0 in exact arithmetic for a readout at its nominal current; leafc7/leafc7b produced
// a ~ 1e-19 on some CI hosts, so the byte-identical re-derivation of LOCK7 failed intermittently at design7_2_M_values.tsv line 35, E24_rel column).
// Output files keep the LOCK7 names, in phase3/locked7c.
// leafc7b.cpp — LOCK7 Amendment 7.1: fork of leafc7.cpp (LOCK7, unchanged). Two changes, recorded in phase3/locked7b/LOCK7b.md:
//  (a) signal LOGR currents: reference IIN (transdiode c1), signal IIN e^-3 nominal (cascoded c2) instead of IIN e^+-1.5. Same n = -3 V_T,
//      same t = +1 and same constant ln av + 3, but the high-injection error (I_c1 - I_c2)/IKF per LOGR falls from 6.4e-5 to 1.4e-5 neper
//      (LOCK7 ideal errors were 2-3x LOCK5's).
//  (b) the readout antilog of a range-sized node (7.5) is designed for av rho as well (LOCK7 sized only the readout gain, so its antilog base
//      reached +22 mV at 46x the nominal Chain_consistency and the block failed in 10 of 12 MC runs).
// Output files keep the LOCK7 names (design7_, recipe7_, leaves7_targets, generator7, netlists7) in phase3/locked7b, so netgen6 mode leafc7
// (unchanged) reads them with FSOT_P7_LOCKED=../../phase3/locked7b.
// leafc7.cpp — FSOT Omni-Solver LOCK7: fork of leafc5.cpp (LOCK5, unchanged). Changes, each recorded in phase3/locked7/LOCK7.md:
//  (1) near-zero summer constants by ratio choice. Signal LOGRs are flipped: the signal current enters the cascoded side c2 at IIN e^-1.5
//      (nominal) and the reference enters the transdiode c1 at IIN e^+1.5, so n = V_T ln(Isig/Iref) = -3 V_T nominal (same headroom as LOCK5)
//      but t = +1 and the LOGR constant is ln av + 3 (LOCK5: t = -1, ln av - 3). A product of logs with net weight W then needs the antilog
//      constant -3 + 3W (LOCK5: -3 - 3W). Each antilog absorbs a further a nepers into its input resistor (I_in = IIN e^a,
//      a = clamp(c0, max(vx0 + 3, -3), 1.5): the cascode keeps vx <= -3 V_T, I_in stays within [IIN e^-3, IIN e^1.5]).
//      Constant-generator base cells keep the LOCK5 orientation (constant -ln m per unit weight; flipping them would add 3 nepers per unit weight,
//      and the generator raises them to powers up to 13) but m is the smallest integer with m c >= e: >= 1 neper of cascode headroom
//      (LOCK5: m c >= 1.4, i.e. 0.34 neper minimum; gamma had 0.55, G_cat 0.60: 2_X mcB seed 3 failed there).
//      The V_NEP bus itself is the LOCK7 base engine's (ratio 256, matched pair; design7.hpp, netgen6).
//  (2) Early-effect correction on uncascoded antilogs (CALU): Q4's collector sits on a virtual ground with its base at vx < 0, V_CB = -vx, so
//      its current is high by (1 + |vx|/VAF), VAF = 74.03 V (the 2N3904 model); the input resistor is raised by that factor at the nominal vx.
//  (3) range sizing for resolution: a heavily cancelling linear sum (largest term / result kappa > 1e3) is designed for a result rho times
//      its nominal, rho = 10^ceil(log10(3 sqrt2 1e-3 kappa)) (3 sigma of a two-term difference at 1e-3 relative term error), and the factor
//      is carried through |x|, its LOGR and the readout (Chain_consistency_%: rho = 100).
//  (4) netlist GENpp (exploration): BETA with pi^pi built as pi^3 exp((pi - 3) ln pi), an exact identity, next to GEN's exp(pi ln pi).
// Output files carry the suffix 7.
// leafc5.cpp — FSOT Omni-Solver Phase 3c (LOCK5): fork of leafc4b.cpp (LOCK4 Amendment 4.5, unchanged). Changes, each recorded in LOCK5.md:
//  (1) cascode headroom: every signal LOGR runs Ir = Ix e^-3 (was Ix/2) and every antilog runs its output at IUo = IIN e^-3 (was IIN/2), so a
//      log-domain base sits at -3 V_T nominal (was -V_T ln 2) and the 2N7000 cascode keeps its drain above its source over +-3 nepers of error;
//  (2) exact algebraic rewrites of engine expressions that remove a cancellation (table REWRITE, each checked against golden to 1e-15);
//  (3) fabs() of a linear (non-log-type) argument is a precision full-wave rectifier (recipe FWR), so a sign flip under tolerances is |x|.
// Output files carry the suffix 5 (design5_, recipe5_, leaves5_targets, generator5, netlists5).
// leafc.cpp — FSOT Omni-Solver Phase 3b: the leaf COMPILER. It turns each FSOT closed form (the engine's own C++ expression,
// extracted by leafmap from FSOT-2.1-Cpp 3466e8d closed_forms_core.gen.inc) into pure-analog blocks: cascoded LOGR/CAL,
// inverting summers (log domain, V_T per neper, constants from the PTAT bus V_NEP = 10 V_T), current-summing TIAs (linear
// domain, constants from V_REF), Taylor-series trig (every term kept until it is below 1e-17 of the result), and comparator
// ladders for floor(). The derived constants (ALPHA ... C_COSM) are NOT resistor values here: they are computed by the
// CONSTANT GENERATOR, i.e. their own core.hpp definitions compiled into the same blocks from four base reference ratios
// (pi, phi, gamma, G_cat as current ratios through LOGRs) and e (1 neper = V_NEP/10). Nothing is fitted; no FSOT rule changes.
//
// Usage: leafc <leaves.tsv from leafmap> <golden_AEB2AD.tsv> <outdir>
// Writes, per netlist <k>: recipe4_<k>.txt (primitive list read by netgen2 mode leafc) and design4_<k>_values.tsv (ideal + E24),
// and leaves4_targets.tsv (every leaf: tier, group, engine value, golden, output node, gain). Written and hashed BEFORE simulation.
#include "design7.hpp"
#include "../../cpp/design.hpp"
#include <algorithm>
#include <cstdio>
#include <fstream>
#include <functional>
#include <set>
#include <sstream>
using namespace d2;
namespace mth = fsot::m;
static R LN(R x) { return mth::ln(x); }
static R AB(R x) { return x < 0 ? -x : x; }
static R p10(R x) { return std::pow(10.0L, std::floor(std::log10(x))); }   // largest power of ten <= x

// ---------------- expression DAG ----------------
struct Nd { std::string op; R v = 0; std::string id; std::vector<int> a; int ip = 0; std::vector<std::pair<R, int>> terms; R k0 = 0; };
static std::vector<Nd> D; static std::map<std::string, int> HC;
static std::string key(const Nd& n) { std::ostringstream s; s.precision(21); s << n.op << "|" << n.id << "|" << n.ip << "|"; if (n.op == "num") s << n.v; for (int x : n.a) s << x << ","; for (auto& [c, x] : n.terms) s << c << "*" << x << ";"; s << n.k0; return s.str(); }
static std::map<std::string, R> ENGV;   // identifier values from the engine
static const fsot::Engine<R>* ENGF = nullptr; static std::map<std::string, std::string> DPREF;   // LOCK8: engine (domain_scalar, fsot_correct); domain -> channel prefix
static R ipw(R x, int n) { if (n < 0) return 1 / ipw(x, -n); R r = 1; while (n--) r *= x; return r; }
static R evalop(const Nd& n) {
  auto A = [&](int i) { return D[n.a[i]].v; };
  if (n.op == "num") return n.v; if (n.op == "id") return ENGV.at(n.id);
  if (n.op == "add") return A(0) + A(1); if (n.op == "sub") return A(0) - A(1); if (n.op == "mul") return A(0) * A(1); if (n.op == "div") return A(0) / A(1);
  if (n.op == "neg") return -A(0); if (n.op == "ipow") return ipw(A(0), n.ip); if (n.op == "pow") return mth::pow(A(0), A(1)); if (n.op == "sqrt") return mth::sqrt(A(0));
  if (n.op == "log") return LN(A(0)); if (n.op == "exp") return mth::exp(A(0)); if (n.op == "sin") return mth::sin(A(0)); if (n.op == "cos") return mth::cos(A(0));
  if (n.op == "acos") return mth::acos(A(0)); if (n.op == "fabs") return AB(A(0)); if (n.op == "floor") return mth::floor(A(0));
  if (n.op == "lsum") { R s = n.k0; for (auto& [c, x] : n.terms) s += c * D[x].v; return s; }
  throw std::runtime_error("op " + n.op);
}
static int mk(Nd n) {
  // literal-only arithmetic folds to a number (a resistor ratio); exp/log of literals are kept: the circuit forms them from the
  // PTAT neper bus (e^x = x nepers, 1 neper = V_NEP/10), so e enters only through the bus.
  bool lit = n.op != "id" && n.op != "num" && n.op != "lsum" && n.op != "exp" && n.op != "log"; for (int x : n.a) if (D[x].op != "num") lit = false;
  if (lit) { R v = evalop(n); Nd m; m.op = "num"; m.v = v; n = m; }      // literal-only arithmetic folds to a number (resistor ratio)
  const std::string k = key(n); auto it = HC.find(k); if (it != HC.end()) return it->second;
  n.v = evalop(n); D.push_back(n); return HC[k] = (int)D.size() - 1; }
static int num(R v) { Nd n; n.op = "num"; n.v = v; return mk(n); }
static int op1(const std::string& o, int a) { Nd n; n.op = o; n.a = {a}; return mk(n); }
static int op2(const std::string& o, int a, int b) { Nd n; n.op = o; n.a = {a, b}; return mk(n); }
static std::map<std::string, int> DEF;  // derived constants -> their compiled definition (constant generator)
// ---------------- parser for the generated C++ expressions ----------------
struct Ps { std::string s; size_t i = 0;
  void ws() { while (i < s.size() && isspace((unsigned char)s[i])) ++i; }
  bool eat(const std::string& t) { ws(); if (s.compare(i, t.size(), t) == 0) { i += t.size(); return true; } return false; }
  void need(const std::string& t) { if (!eat(t)) throw std::runtime_error("expected '" + t + "' at " + std::to_string(i) + " in " + s); }
  std::string ident() { ws(); size_t j = i; while (j < s.size() && (isalnum((unsigned char)s[j]) || s[j] == '_' || s[j] == ':')) ++j; std::string r = s.substr(i, j - i); i = j; return r; }
  int expr() { int a = term(); for (;;) { if (eat("+")) a = op2("add", a, term()); else if (s[i] == '-' && (ws(), eat("-"))) a = op2("sub", a, term()); else return a; } }
  int term() { int a = unary(); for (;;) { if (eat("*")) a = op2("mul", a, unary()); else if (eat("/")) a = op2("div", a, unary()); else return a; } }
  int unary() { if (eat("-")) return op1("neg", unary()); return prim(); }
  int prim() { ws();
    if (eat("(")) { int a = expr(); need(")"); return a; }
    if (isdigit((unsigned char)s[i]) || s[i] == '.') { size_t j = i; while (j < s.size() && (isdigit((unsigned char)s[j]) || s[j] == '.' || s[j] == 'e')) ++j; R v = std::stold(s.substr(i, j - i)); i = j; return num(v); }
    std::string id = ident(); if (id.empty()) throw std::runtime_error("bad token at " + std::to_string(i) + " in " + s);
    if (id == "R") { need("("); int a = expr(); need(")"); return a; }
    if (id == "domain_scalar") { need("("); need("\""); size_t j = s.find('"', i); std::string d = s.substr(i, j - i); i = j + 1; need(")");   // LOCK8 (1)
      Nd x; x.op = "id"; x.id = "S_D_" + d; if (!ENGV.count(x.id)) ENGV[x.id] = ENGF->domain_scalar(d); return mk(x); }
    if (id == "T::lit") { need("("); need("\""); size_t j = s.find('"', i); R v = std::stold(s.substr(i, j - i)); i = j + 1; need(")"); return num(v); }
    if (id == "xexp") { need("("); int a = expr(); need(","); expr(); need(","); expr(); need(")"); return a; }   // authority mode: the literal
    if (id == "ipow") { need("("); int a = expr(); need(","); int n = expr(); need(")"); Nd x; x.op = "ipow"; x.a = {a}; x.ip = (int)std::llround(D[n].v); return mk(x); }
    if (id == "pow") { need("("); int a = expr(); need(","); int b = expr(); need(")"); return op2("pow", a, b); }
    for (const char* f : {"sqrt", "log", "exp", "sin", "cos", "acos", "fabs", "floor"}) if (id == f) { need("("); int a = expr(); need(")"); return op1(f, a); }
    if (DEF.count(id)) return DEF[id];
    if (ENGV.count(id)) { Nd x; x.op = "id"; x.id = id; return mk(x); }
    throw std::runtime_error("unknown identifier " + id); } };
static int parse(const std::string& s) { Ps p; p.s = s; int r = p.expr(); p.ws(); if (p.i != s.size()) throw std::runtime_error("trailing text in " + s); return r; }

// ---------------- circuit builder ----------------
static const R VRf = 2.5L, VT = VT0, RSL = 10e3L; static R E3M = 0.049787068367863942979L;   // e^-3   // VT only sizes nominal currents; every gain is a ratio
static const R E15M = 0.22313016014842982893L, E15P = 4.4816890703380648226L;   // e^-1.5, e^+1.5 (20 digits, checked in main)
static const R VAFN = 74.03L;   // 2N3904 model Early voltage (netgen Q())
static R IIN = 1e-6L, IUo = 1e-6L * 0.049787068367863942979L;   // LOCK5 (1): IUo = IIN e^-3 (e^-3 to 20 digits, checked in main)   // antilog input / nominal output current (low: the 2N3904 high-injection term IKF = 66.8 mA adds (Ia - Ib)/IKF nepers per ratio)
struct Lg { std::map<std::string, R> t; R c = 0; };     // ln|v| = sum t[n] V(n)/V_T + c
struct Lin { std::string net; R g = 0; };               // V(net) = g v
struct Nep { std::string net; R a = 0; };               // V(net) = V_T a v
// Taylor series kept until the next term is below 1e-8 of the result. Smaller terms would run their antilog cascodes under
// ~1e-14 A (ngspice abstol): numerically indeterminate, and below any real junction's leakage. The remainder is reported.
static const R TRUNC = 1e-8L; static R maxrem = 0; static std::map<int, R> REM;   // trig node -> relative remainder
static bool PPSPLIT = false;
static bool AB_FLIP = false, AB_ABS = false, AB_BASE = false, AB_EARLY = false, AB_RHO = false;   // LOCK9 ablation switches
struct Ckt {
  std::string tag; std::vector<std::string> rec; std::vector<Part> parts; int uid = 0; std::set<std::string> chans;
  std::map<int, Lg> mLG; std::map<int, Lin> mLIN; std::map<int, Nep> mNEP; std::map<std::string, std::string> mINV;
  std::map<std::string, int> stats;
  std::string u(const std::string& p) { return "z" + p + std::to_string(++uid) + "x"; }
  void Rr(const std::string& nm, const std::string& a, const std::string& b, R val, const std::string& mean) {
    if (!(val >= 1) || !std::isfinite((double)val)) throw std::runtime_error("resistor out of range " + nm);
    if (val > 1e7L) {   // above the E24 decade table (max 10 M): a series string of equal pieces <= 9 M, each realised in E24
      if (val > 1.8e9L) {   // LOCK5: above 1.8 G a T-network into the (virtual-ground) node b: R_eq = Ra + Rc + Ra Rc / Rb, Ra = Rc = 9 M
        const R Ra = 9e6L, Rb = Ra * Ra / (val - 2 * Ra); stats["R_tee"]++; std::string t = nm + "_t";
        parts.push_back({"P_" + nm + "_ta", Ra, "ohm", mean + " (T-network arm a)"}); rec.push_back("R\t" + nm + "_ta\t" + a + "\t" + t + "\tP_" + nm + "_ta");
        parts.push_back({"P_" + nm + "_tb", Rb, "ohm", mean + " (T-network shunt to ground)"}); rec.push_back("R\t" + nm + "_tb\t" + t + "\t0\tP_" + nm + "_tb");
        parts.push_back({"P_" + nm + "_tc", Ra, "ohm", mean + " (T-network arm c)"}); rec.push_back("R\t" + nm + "_tc\t" + t + "\t" + b + "\tP_" + nm + "_tc"); return; }
      const int n = (int)std::ceil(val / 9e6L); stats["R>27M_strings"]++; if (n > 200) throw std::runtime_error("resistor > 1.8 G " + nm);
      std::string prev = a; for (int i = 0; i < n; ++i) { std::string nx = (i + 1 == n) ? b : nm + "_q" + std::to_string(i);
        parts.push_back({"P_" + nm + "_" + std::to_string(i), val / n, "ohm", mean + " (string piece " + std::to_string(i + 1) + "/" + std::to_string(n) + ")"});
        rec.push_back("R\t" + nm + "_" + std::to_string(i) + "\t" + prev + "\t" + nx + "\tP_" + nm + "_" + std::to_string(i)); prev = nx; }
      return; }
    parts.push_back({"P_" + nm, val, "ohm", mean}); rec.push_back("R\t" + nm + "\t" + a + "\t" + b + "\tP_" + nm); }
  void OAx(const std::string& nm, const std::string& p, const std::string& m, const std::string& o) { rec.push_back("OA\t" + nm + "\t" + p + "\t" + m + "\t" + o); stats["opamp"]++; }
  void Cx(const std::string& nm, const std::string& a, const std::string& b, const std::string& v) { rec.push_back("C\t" + nm + "\t" + a + "\t" + b + "\t" + v); }
  std::string inv(const std::string& net) { auto it = mINV.find(net); if (it != mINV.end()) return it->second;
    std::string p = u("I"), o = p + "o"; OAx(p, "0", p + "s", o); Rr(p + "a", net, p + "s", 100e3L, "unity inverter in"); Rr(p + "b", o, p + "s", 100e3L, "unity inverter fb");
    return mINV[net] = o; }
  // summer: V(out) = k (sum_n t_n V(n) + c V_T)
  std::string sum(const Lg& L, R k, const std::string& why) {
    std::string p = u("S"), o = p + "o"; OAx(p, "0", p + "s", o); Rr(p + "f", o, p + "s", RSL, why + ": summer feedback"); Cx(p, o, p + "s", "100p"); stats["summer"]++;
    int j = 0; for (auto& [n, t] : L.t) { R w = -k * t; std::string src = n; if (w == 0) continue; if (w < 0) { src = inv(n); w = -w; } Rr(p + "i" + std::to_string(++j), src, p + "s", RSL / w, why + ": weight"); }
    R kc = k * L.c; if (AB(kc) > 1e-18L) Rr(p + "c", kc > 0 ? "vnepn" : "vnep", p + "s", RSL * 10 / AB(kc), why + ": constant (nepers from V_NEP)");
    return o; }
  // CAL: sinks I = kap |v| (vr input) or kap V_T |v| (ptat input) from node out
  bool uncasc = false;   // set for sum terms below 1e-4 of the largest: no 2N7000 cascode (Q4 collector on the virtual ground;
                         // Early error V_CB/VAF ~ 5e-3 of a term that is < 1e-4 of the sum); a cascode at < 1e-11 A is numerically indeterminate
  std::vector<std::array<R, 4>> CALS;   // per antilog: c0, a, c, vx (nepers), for the LOCK7 report
  void cal(const Lg& L, R kap, bool ptat, const std::string& out, const std::string& why, R av) {
    std::string p = u("C"); const R Rin0 = ptat ? 10 * VT / IIN : VRf / IIN;
    const R q0 = ptat ? kap * Rin0 / 10 : kap * Rin0 / VRf; const R c0 = L.c + LN(q0), vx0 = LN(av * q0);   // vx0 = ln(Iout/IIN) nominal
    R a = std::min<R>(1.5L, std::max<R>(c0, std::max<R>(vx0 + 3, -3)));                                   // LOCK7 (1)
    if (AB_ABS) a = 0;   // LOCK9 ablation
    if (AB(a) < 1e-12L) a = 0;   // Amendment 7.2: vx0 + 3 is 0 in exact arithmetic for a nominal readout; its last-bit rounding differed between CI hosts
    const R ea = a == 0 ? R(1) : mth::exp(a); R Rin = Rin0 / ea; const R q = ptat ? kap * Rin / 10 : kap * Rin / VRf;     // I = Iin |v| q
    Lg M = L; M.c += LN(q); const R vxn = vx0 - a;
    R Re = Rin; if (uncasc && vxn < 0 && !AB_EARLY) { Re = Rin * (1 + (-vxn * VT) / VAFN); stats["early_corrected"]++; }   // LOCK7 (2)
    Rr(p + "p", ptat ? "vnep" : "vr", p + "d", Re, why + ": antilog input (I_in = IIN e^a)" + std::string(uncasc ? ", x(1 + |vx|/VAF)" : ""));
    CALS.push_back({c0, a, M.c, vxn}); if (std::getenv("FSOT_L7_DBG")) std::fprintf(stderr, "CALDBG\t%s\t%s\t%.3Lf\t%.3Lf\t%.3Lf\t%.3Lf\n", tag.c_str(), why.c_str(), c0, a, M.c, vxn); stats["const_absorbed_mN"] += (int)std::llround(1000 * (AB(c0) - AB(M.c)));
    std::string vx = sum(M, 1, why + " (antilog exponent)"); rec.push_back(std::string(uncasc ? "CALU" : "CAL") + "\t" + p + "\t" + vx + "\t" + out); stats[uncasc ? "antilog_uncascoded" : "antilog"]++; }
  // LOGR from a positive-going linear (ptat=false, ref V_REF) or V_T-scaled (ptat=true, ref V_NEP) node, |gain| gv, nominal |value| av
  Lg logr(std::string net, R gv, R av, bool ptat, const std::string& why) {
    if (gv < 0) { net = inv(net); gv = -gv; }
    if (AB_FLIP) {   // LOCK9 ablation: LOCK5 orientation (leafc5 logr, verbatim)
      std::string p = u("L"); R Vin = gv * av * (ptat ? VT : 1); R Rin = Vin / IIN; R Rref = (ptat ? 10 * VT : VRf) / (IIN * E3M);
      Rr(p + "i", net, p + "c1", Rin, why + ": log input"); Rr(p + "r", ptat ? "vnep" : "vr", p + "c2", Rref, why + ": log reference (Ir = Ix e^-3)");
      rec.push_back("LOGR\t" + p + "\t" + p + "n"); stats["log"]++;
      Lg L; L.t[p + "n"] = -1; L.c = -LN(gv * Rref / (Rin * (ptat ? 10 : VRf))); return L; }
    std::string p = u("L"); R Vin = gv * av * (ptat ? VT : 1); R Rin = Vin / (IIN * E3M); R Rref = (ptat ? 10 * VT : VRf) / IIN;   // LOCK7 (1), Amendment 7.1 (a)
    Rr(p + "i", net, p + "c2", Rin, why + ": log input (signal on the cascoded side, IIN e^-3)"); Rr(p + "r", ptat ? "vnep" : "vr", p + "c1", Rref, why + ": log reference (transdiode, IIN)");
    rec.push_back("LOGR\t" + p + "\t" + p + "n"); stats["log"]++;
    // n = V_T ln(Isig/Iref), Isig = gv V |v|/Rin, Iref = Vref/Rref  ->  ln|v| = +V(n)/V_T + ln(Vref_ratio Rin/(gv Rref))
    Lg L; L.t[p + "n"] = 1; L.c = LN((ptat ? 10 : VRf) * Rin / (gv * Rref)); return L; }
  // LOCK7 (3): design range factor of a node (1 unless it depends on a heavily cancelling linear sum)
  std::map<int, R> mRHO;
  R rho(int i) { if (AB_RHO) return 1; auto it = mRHO.find(i); if (it != mRHO.end()) return it->second; const Nd& n = D[i]; R r = 1;
    for (int x : n.a) r = std::max(r, rho(x)); for (auto& [c, x] : n.terms) r = std::max(r, rho(x));
    if (n.op == "add" || n.op == "sub" || n.op == "lsum") { std::vector<std::pair<R, int>> T; R k0 = 0; flatten(i, 1, T, k0); R mx = AB(k0);
      for (auto& [c, x] : T) mx = std::max(mx, AB(c * D[x].v)); const R kap = mx / AB(n.v);
      if (kap > 1e3L) r = std::max(r, std::pow(10.0L, std::ceil(std::log10(3 * std::sqrt(2.0L) * 1e-3L * kap)))); }
    return mRHO[i] = r; }
  bool logtype(int i) { const std::string& o = D[i].op; return o == "mul" || o == "div" || o == "ipow" || o == "sqrt" || o == "pow" || o == "exp" || o == "id" || o == "log" || o == "neg" || o == "fabs"; }
  Lg LG(int i) {
    auto it = mLG.find(i); if (it != mLG.end()) return it->second; const Nd& n = D[i]; Lg L;
    auto addm = [](Lg a, const Lg& b, R s) { for (auto& [k, v] : b.t) a.t[k] += s * v; a.c += s * b.c; return a; };
    if (n.op == "num") L.c = LN(AB(n.v));
    else if (n.op == "id" && n.id == "E") L.c = 1;
    else if (n.op == "id" && (n.id == "PI" || n.id == "PHI" || n.id == "GAMMA" || n.id == "G_CAT")) {   // constant generator base cell
      // smallest integer m with m c >= 1.4 (cascode margin: n = -V_T ln(m c) <= -8.7 mV); the literal ln m comes off the PTAT bus,
      // so a small m keeps the V_NEP error out of ln c (pi: 1, phi: 1, gamma: 3, G_cat: 2)
      // LOCK7 (1): LOCK5 orientation kept (t = -1, constant -ln m, no 3-neper term per unit weight) but the cascode headroom raised to >= 1 neper:
      // m = smallest integer with m c >= e (pi: 1, phi: 2, gamma: 5, G_cat: 3; LOCK5: m c >= 1.4, G_cat 0.60 and gamma 0.55 neper)
      R m = 1; while (m * n.v < (AB_BASE ? 1.4L : 2.718281828459045235L)) m += 1; std::string p = u("B"); stats["base_ref"]++;
      Rr(p + "i", "vr", p + "c1", VRf / IIN, "base " + n.id + ": Ix"); Rr(p + "r", "vr", p + "c2", VRf / IIN * m * n.v, "base " + n.id + ": Ir = Ix/(" + std::to_string((int)m) + " " + n.id + ")");
      rec.push_back("LOGR\t" + p + "\t" + p + "n"); stats["log"]++; L.t[p + "n"] = -1; L.c = -LN(m); }
    else if (n.op == "mul") L = addm(LG(n.a[0]), LG(n.a[1]), 1);
    else if (n.op == "div") L = addm(LG(n.a[0]), LG(n.a[1]), -1);
    else if (n.op == "fabs" && !logtype(n.a[0])) { Lin l = LIN(i); L = logr(l.net, l.g, AB(n.v) * rho(i), false, "log of |x|"); }   // LOCK5 (3), LOCK7 (3)
    else if (n.op == "neg" || n.op == "fabs") L = LG(n.a[0]);
    else if (n.op == "ipow") L = addm(Lg{}, LG(n.a[0]), n.ip);
    else if (n.op == "sqrt") L = addm(Lg{}, LG(n.a[0]), 0.5L);
    else if (n.op == "pow" && D[n.a[1]].op == "num") L = addm(Lg{}, LG(n.a[0]), D[n.a[1]].v);
    else if (n.op == "pow" && PPSPLIT && D[n.a[1]].v > 1 && D[n.a[1]].v - std::floor(D[n.a[1]].v) > 1e-9L) {   // LOCK7 (4), GENpp only: x^y = x^k exp((y - k) ln x), k = floor y (exact identity)
      const int k = (int)std::floor((double)D[n.a[1]].v); Nep e = NEP(op2("mul", op2("sub", n.a[1], num(k)), op1("log", n.a[0]))); L = addm(Lg{}, LG(n.a[0]), k); L.t[e.net] += 1 / e.a; stats["pow_split"]++; }
    else if (n.op == "pow") { Nep e = NEP(op2("mul", n.a[1], op1("log", n.a[0]))); L.t[e.net] = 1 / e.a; }   // x^y = exp(y ln x)
    else if (n.op == "exp" && D[n.a[0]].op == "num") L.c = D[n.a[0]].v;   // e^c: c nepers straight from the PTAT bus
    else if (n.op == "exp") { Nep e = NEP(n.a[0]); L.t[e.net] = 1 / e.a; }
    else if (n.op == "log") { Nep e = NEP(i); L = logr(e.net, e.a, AB(n.v), true, "ln-value"); }
    else { Lin l = LIN(i); L = logr(l.net, l.g * (n.v < 0 ? -1 : 1), AB(n.v) * rho(i), false, "log of " + n.op); }
    return mLG[i] = L; }
  // linear readout of a log-type node: CAL + TIA, I = 2.5 uA nominal
  Lin calTIA(int i, R g) {
    const R av = AB(D[i].v) * rho(i); std::string p = u("T"), o = p + "o"; R RT = AB(g) * av / IUo;   // Amendment 7.1 (b): av rho
    OAx(p, "0", p + "s", o); Rr(p + "f", o, p + "s", RT, "readout TIA"); Cx(p, o, p + "s", "100p"); cal(LG(i), IUo / av, false, p + "s", "readout", av);
    return {o, AB(g) * (D[i].v < 0 ? -1 : 1)}; }
  Lin LIN(int i) {
    auto it = mLIN.find(i); if (it != mLIN.end()) return it->second; const Nd& n = D[i]; Lin r;
    if (n.op == "id" && n.id.rfind("S_D_", 0) == 0) { const std::string d = n.id.substr(4); if (!DPREF.count(d)) { std::string p = "d"; p += (char)('a' + DPREF.size()); DPREF[d] = p; }
      chans.insert("D:" + d); r = {"vS" + DPREF[d], 2}; }   // LOCK8 (1)
    else if (n.op == "id" && n.id == "S_COSM") r = {"vSm", 2}; else if (n.op == "id" && n.id == "S_QUANT") r = {"vSq", 2}; else if (n.op == "id" && n.id == "S_CHEM") r = {"vSc", 2};
    else if (n.op == "fabs" && D[n.a[0]].op == "id" && D[n.a[0]].id == "S_COSM") r = {"vabs", 2};
    else if (n.op == "sub" && D[n.a[0]].op == "num" && D[n.a[0]].v == 1 && D[n.a[1]].op == "id" && D[n.a[1]].id == "S_CHEM") r = {"v1sc", -40};
    else if (n.op == "sub" && D[n.a[0]].op == "num" && D[n.a[0]].v == 1 && D[n.a[1]].op == "id" && D[n.a[1]].id == "S_QUANT") r = {"v1sq", -40};
    else if (n.op == "fabs" && logtype(n.a[0])) { Lin l = LIN(n.a[0]); r = {l.net, D[n.a[0]].v < 0 ? -l.g : l.g}; }   // log-type argument: its readout carries a fixed sign
    else if (n.op == "fabs") {   // LOCK5 (3): precision full-wave rectifier, out = |in| for either sign: V(out) = |g| |v|
      Lin l = LIN(n.a[0]); std::string p = u("F"), o = p + "o"; parts.push_back({"P_" + p + "a", 100e3L, "ohm", "full-wave R"}); parts.push_back({"P_" + p + "b", 200e3L, "ohm", "full-wave 2R"});
      rec.push_back("FWR\t" + p + "\t" + l.net + "\t" + o + "\tP_" + p + "a\tP_" + p + "b"); stats["fullwave"]++; r = {o, AB(l.g)}; }
    else if (n.op == "neg") { Lin l = LIN(n.a[0]); r = {l.net, -l.g}; }
    else if (n.op == "add" || n.op == "sub" || n.op == "lsum") r = linsum(i);
    else if (n.op == "sin" || n.op == "cos" || n.op == "acos") r = LIN(series(i));
    else if (n.op == "floor") r = quant(i);
    else if (n.op == "num") throw std::runtime_error("literal leaf");
    else r = calTIA(i, p10(8 / (AB(n.v) * rho(i))));
    return mLIN[i] = r; }
  void flatten(int i, R c, std::vector<std::pair<R, int>>& T, R& k0) {
    const Nd& n = D[i];
    if (n.op == "num") { k0 += c * n.v; return; }
    if (n.op == "add") { flatten(n.a[0], c, T, k0); flatten(n.a[1], c, T, k0); return; }
    if (n.op == "sub" && !(D[n.a[0]].op == "num" && D[n.a[0]].v == 1 && D[n.a[1]].op == "id")) { flatten(n.a[0], c, T, k0); flatten(n.a[1], -c, T, k0); return; }
    if (n.op == "neg") { flatten(n.a[0], -c, T, k0); return; }
    if (n.op == "lsum") { k0 += c * n.k0; for (auto& [cc, x] : n.terms) flatten(x, c * cc, T, k0); return; }
    if (n.op == "mul" && D[n.a[0]].op == "num") { flatten(n.a[1], c * D[n.a[0]].v, T, k0); return; }
    if (n.op == "mul" && D[n.a[1]].op == "num") { flatten(n.a[0], c * D[n.a[1]].v, T, k0); return; }
    if (n.op == "div" && D[n.a[1]].op == "num") { flatten(n.a[0], c / D[n.a[1]].v, T, k0); return; }
    T.push_back({c, i}); }
  Lin linsum(int i) {
    std::vector<std::pair<R, int>> T; R k0 = 0; flatten(i, 1, T, k0); const R v = D[i].v; stats["linear_sum"]++;
    R G = p10(8 / (AB(v) * rho(i))) * (v < 0 ? -1 : 1); if (rho(i) > 1) stats["range_sized"]++;
    R mx = AB(G * k0); for (auto& [c, x] : T) mx = std::max(mx, AB(G * c * D[x].v));
    if (mx / IUo > 1e8L) { G *= 1e8L * IUo / mx; mx = 1e8L * IUo; stats["cancelling_sum"]++; }   // heavy cancellation (terms > 25x the output): output scaled down, R_f capped at 100 M
    const R Rf = mx / IUo; std::string p = u("A"), o = p + "o"; OAx(p, "0", p + "s", o); Rr(p + "f", o, p + "s", Rf, "sum feedback"); Cx(p, o, p + "s", "100p");
    int j = 0; std::string an; R negsum = 0;
    auto sinkable = [&](int x) { return logtype(x) && D[x].op != "log" && !(D[x].op == "id" && D[x].id.rfind("S_", 0) == 0); };
    for (auto& [c, x] : T) if (sinkable(x) && G * c * D[x].v < 0) negsum += AB(G * c * D[x].v);
    if (negsum > 0) {   // negative log-type terms: sunk into an auxiliary TIA (V = Rf/m sum I), fed back with Rf/m: contributes -Rf sum I
      const R m = std::max<R>(1, std::ceil(negsum / 6)); an = p + "n"; OAx(an, "0", an + "s", an + "o"); Rr(an + "f", an + "o", an + "s", Rf / m, "negative-term TIA");
      Cx(an, an + "o", an + "s", "100p"); Rr(an + "b", an + "o", p + "s", Rf / m, "negative-term feed"); }
    for (auto& [c, x] : T) { const R want = G * c * D[x].v; ++j;
      if (sinkable(x)) { Lg lx = LG(x); uncasc = AB(want) < 1e-4L * mx; cal(lx, AB(G * c) / Rf, false, want > 0 ? p + "s" : an + "s", "sum term", AB(D[x].v)); uncasc = false; continue; }
      Lin l = LIN(x); R w = -G * c / l.g; std::string src = l.net; if (w < 0) { src = inv(src); w = -w; }
      Rr(p + "t" + std::to_string(j), src, p + "s", Rf / w, "sum term"); }
    if (AB(G * k0) > 0) Rr(p + "k", G * k0 < 0 ? "vr" : "vrn", p + "s", Rf * VRf / AB(G * k0), "sum constant (V_REF)");
    return {o, G}; }
  int series(int i) {   // sin/cos/acos as their Taylor series; terms kept until |term| < 1e-17 |result| (none dropped above that)
    const Nd& n = D[i]; Nd s; s.op = "lsum"; int x = n.a[0]; const R xv = D[x].v; R tot = n.v, f = 1;
    if (n.op == "sin" || n.op == "cos") { int k0 = n.op == "sin" ? 1 : 0; if (k0 == 0) s.k0 = 1;
      for (int k = (k0 == 0 ? 2 : 1); k < 80; k += 2) { f = 1; for (int j = 2; j <= k; ++j) f *= j; R c = ((k / 2) % 2 ? -1 : 1) / f; s.terms.push_back({c, k == 1 ? x : [&]{ Nd q; q.op = "ipow"; q.a = {x}; q.ip = k; return mk(q); }()});
        if (AB(c * ipw(xv, k)) < TRUNC * AB(tot) && k > 3) break; } }
    else {   // acos y = pi/2 - sum (2k)!/(4^k (k!)^2 (2k+1)) y^(2k+1)
      Nd pi; pi.op = "id"; pi.id = "PI"; s.terms.push_back({0.5L, mk(pi)});
      for (int k = 0; k < 200; ++k) { R c = 1; for (int j = 1; j <= k; ++j) c *= (R)(2 * j - 1) / (2 * j); c /= (2 * k + 1);
        int t = (k == 0) ? x : [&]{ Nd q; q.op = "ipow"; q.a = {x}; q.ip = 2 * k + 1; return mk(q); }(); s.terms.push_back({-c, t});
        if (AB(c * ipw(xv, 2 * k + 1)) < TRUNC * AB(tot) && k > 2) break; } }
    stats["series_terms"] += (int)s.terms.size(); int r = mk(s);
    const R rem = AB(D[r].v - n.v) / AB(n.v); maxrem = std::max(maxrem, rem); REM[i] = rem;   // the truncation remainder, reported per leaf
    if (rem > 10 * TRUNC) throw std::runtime_error("series remainder above bound");
    return r; }
  Lin quant(int i) {   // floor(x) = number of integers k >= 1 with x >= k (x >= 0 over the domain): one comparator per k
    const Nd& n = D[i]; Lin lx = LIN(n.a[0]); const int K = (int)n.v + 2; std::string p = u("Q"), o = p + "o"; const R G = -p10(8 / std::max<R>(1, AB(n.v)));
    const R Rf = 100e3L; OAx(p, "0", p + "s", o); Rr(p + "f", o, p + "s", Rf, "floor ladder sum"); Cx(p, o, p + "s", "100p"); stats["comparator"] += K;
    for (int k = 1; k <= K; ++k) { std::string t = p + "h" + std::to_string(k), ts = t + "s";   // threshold g k from V_REF
      OAx(t, "0", ts, t + "o"); Rr(t + "a", lx.g * k > 0 ? "vrn" : "vr", ts, 100e3L, "threshold in"); Rr(t + "b", t + "o", ts, 100e3L * AB(lx.g * k) / VRf, "threshold gain");
      std::string c = p + "k" + std::to_string(k); rec.push_back(std::string("CMP\t") + c + "\t" + (lx.g > 0 ? lx.net : t + "o") + "\t" + (lx.g > 0 ? t + "o" : lx.net) + "\t" + c + "o");
      // [x>=k] = (Vc + 6.8)/13.6 ; out = G floor = sum G Vc/13.6 + G K/2
      Rr(p + "c" + std::to_string(k), c + "o", p + "s", Rf * 13.6L / AB(G), "ladder weight"); }
    Rr(p + "k", G * K / 2 < 0 ? "vr" : "vrn", p + "s", Rf * VRf / AB(G * K / 2), "ladder offset (half-steps of the comparator swing)");
    return {o, G}; }
  Nep NEP(int i) {
    auto it = mNEP.find(i); if (it != mNEP.end()) return it->second; const Nd& n = D[i]; Nep r;
    // ln x as a V_T-scaled voltage straight from a summer, unity scale (noise gain 1 + sum|t| keeps the 1e6 op-amp gain error small)
    if (n.op == "log") { const R v = n.v; R k = AB(v) >= 0.1L ? 1 : p10(1 / std::max<R>(AB(v), 1e-30L)); r = {sum(LG(n.a[0]), k, "ln(x)"), k}; }
    else if (n.op == "neg") { Nep e = NEP(n.a[0]); r = {e.net, -e.a}; }
    else { const R av = AB(n.v); R a = p10(8 / (VT * av)); std::string p = u("P"), o = p + "o"; R RT = a * VT * av / IUo;   // V = V_T a v via a PTAT-fed antilog
      OAx(p, "0", p + "s", o); Rr(p + "f", o, p + "s", RT, "PTAT-scaled TIA"); Cx(p, o, p + "s", "100p"); cal(LG(i), IUo / (VT * av), true, p + "s", "x V_T", av); r = {o, a * (n.v < 0 ? -1 : 1)}; }
    return mNEP[i] = r; }
};

int main(int argc, char** argv) {
  if (argc < 4) { std::fprintf(stderr, "usage: leafc leaves.tsv golden.tsv outdir\n"); return 1; }
  Fsot s; auto& F = s.F; ENGF = &s.F; const std::string out = argv[3];
  if (const char* e = std::getenv("FSOT_L9_ABL")) { std::string a = std::string(",") + e + ","; auto has = [&](const char* k) { return a.find(std::string(",") + k + ",") != std::string::npos; };
    AB_FLIP = has("flip"); AB_ABS = has("abs"); AB_BASE = has("base"); AB_EARLY = has("early"); AB_RHO = has("rho"); }
  ENGV = {{"PI", F.PI}, {"E", F.E}, {"PHI", F.PHI}, {"GAMMA", F.GAMMA}, {"G_CAT", F.G_CAT}, {"S_COSM", F.S_COSM}, {"S_QUANT", F.S_QUANT}, {"S_CHEM", F.S_CHEM}};
  // the constant generator: core.hpp lines 115-135, verbatim
  const std::vector<std::pair<std::string, std::string>> defs = {
    {"ALPHA", "log(PI) / (E * ipow(PHI, 13))"}, {"PSI_CON", "R(1) - exp(R(-1))"}, {"ETA_EFF", "R(1) / (PI - R(1))"}, {"BETA", "R(1) / exp(pow(PI, PI) + (E - R(1)))"},
    {"GAMMA_C", "-log(R(2)) / PHI"}, {"OMEGA", "sin(PI / E) * sqrt(R(2))"}, {"THETA_S", "sin(PSI_CON * ETA_EFF)"}, {"POOF", "exp((-log(PI) / E) / (ETA_EFF * log(PHI)))"},
    {"C_EFF", "(R(1) - POOF * sin(THETA_S)) * (R(1) + (R(1) / ipow(PI, 4)) * G_CAT / (PI * PHI))"}, {"A_BLEED", "sin(PI / E) * PHI / sqrt(R(2))"}, {"P_VAR", "-cos(THETA_S + PI)"},
    {"B_IN", "C_EFF * (R(1) - sin(THETA_S) / PHI)"}, {"A_IN", "A_BLEED * (R(1) + cos(THETA_S) / PHI)"}, {"SUCTION", "POOF * (-cos(THETA_S - PI))"}, {"CHAOS", "GAMMA_C / OMEGA"},
    {"P_BASE", "GAMMA / E"}, {"P_NEW", "P_BASE * sqrt(R(2))"}, {"C_FACTOR", "C_EFF * P_NEW"}, {"K", "PHI * (GAMMA / E) * sqrt(R(2)) / log(PI) * (R(1) - R(1) / ipow(PI, 4))"}, {"C_COSM", "R(1) / (PHI * ipow(PI, 2))"}};
  const std::map<std::string, R> engd = {{"ALPHA", F.ALPHA}, {"PSI_CON", F.PSI_CON}, {"ETA_EFF", F.ETA_EFF}, {"BETA", F.BETA}, {"GAMMA_C", F.GAMMA_C}, {"OMEGA", F.OMEGA}, {"THETA_S", F.THETA_S}, {"POOF", F.POOF},
    {"C_EFF", F.C_EFF}, {"A_BLEED", F.A_BLEED}, {"P_VAR", F.P_VAR}, {"B_IN", F.B_IN}, {"A_IN", F.A_IN}, {"SUCTION", F.SUCTION}, {"CHAOS", F.CHAOS}, {"P_BASE", F.P_BASE}, {"P_NEW", F.P_NEW}, {"C_FACTOR", F.C_FACTOR}, {"K", F.K}, {"C_COSM", F.C_COSM}};
  // DEV dry run (env FSOT_LEAFC_DEV=1): the same compiler and topologies with NON-FSOT base values (pi->3, phi->1.5, gamma->0.5,
  // G->0.9; S_* stay the Phase-2 channel values), used only to debug the toolchain without simulating any locked FSOT value.
  const bool DEV = std::getenv("FSOT_LEAFC_DEV") != nullptr; if (const char* e = std::getenv("FSOT_LEAFC_IIN")) { IIN = std::stold(e); IUo = IIN * E3M; }
  if (DEV) ENGV = {{"PI", 3.0L}, {"E", F.E}, {"PHI", 1.5L}, {"GAMMA", 0.5L}, {"G_CAT", 0.9L}, {"S_COSM", F.S_COSM}, {"S_QUANT", F.S_QUANT}, {"S_CHEM", F.S_CHEM}};  // S: the Phase-2 channel outputs (already simulated)
  int bad = 0; if (AB(E3M - fsot::m::exp(R(-3))) > 1e-19L) { std::fprintf(stderr, "e^-3 literal mismatch\n"); ++bad; }
  if (AB(E15M - fsot::m::exp(R(-1.5L))) > 1e-19L || AB(E15P - fsot::m::exp(R(1.5L))) > 1e-18L) { std::fprintf(stderr, "e^+-1.5 literal mismatch\n"); ++bad; }
  for (auto& [nm, ex] : defs) { int r = parse(ex); DEF[nm] = r; if (DEV) continue; R rel = (D[r].v - engd.at(nm)) / engd.at(nm); if (AB(rel) > 1e-17L) { std::fprintf(stderr, "generator %s mismatch %Lg\n", nm.c_str(), rel); ++bad; } }
  // golden sec.* rows in order
  std::vector<std::array<std::string, 3>> gold; { std::ifstream g(argv[2]); std::string l; while (std::getline(g, l)) { if (l.rfind("sec.", 0)) continue; std::istringstream t(l); std::string a, b, c; std::getline(t, a, '\t'); std::getline(t, b, '\t'); std::getline(t, c, '\t'); gold.push_back({a, b, c}); } }
  // leaves: expand the two loop rows to their instances
  struct Lf { std::string sec, name, form, tier, cexpr; int node = -1; std::string rewrite; std::string gkey; R golden = 0; std::string group, net, status; R g = 0, rem = 0; };
  std::vector<Lf> L; { std::ifstream f(argv[1]); std::string l; std::getline(f, l);
    while (std::getline(f, l)) { std::vector<std::string> c; std::istringstream t(l); std::string x; while (std::getline(t, x, '\t')) c.push_back(x); while (c.size() < 9) c.push_back("");
      if (c[1] == "Layer_") { std::string tot = "(R(1)/ipow(PHI,1))"; for (int i = 2; i < 7; ++i) tot = "(" + tot + " + R(1)/ipow(PHI," + std::to_string(i) + "))";
        for (int i = 1; i < 7; ++i) L.push_back({c[0], "Layer_" + std::to_string(i) + "_thickness", c[2], c[7], "((R(1) / ipow(PHI, " + std::to_string(i) + ")) / " + tot + ")"}); continue; }
      if (c[1] == "Richardson_D=") { for (int d : {4, 13, 25}) L.push_back({c[0], "Richardson_D=" + std::to_string(d), c[2], "0 (literal)", "pow(R(25) / R(" + std::to_string(d) + "), T::lit(\"0.2\"))"}); continue; }
      L.push_back({c[0], c[1], c[2], c[7], c[8]}); } }
  { // LOCK8 (2)(3): part-C leaves (no golden row: target = the engine / hub formula in long double; gkey "partC.<name>")
    const std::vector<std::string> DOMS = {"Neuroscience", "Optics", "Quantum_Optics", "Materials_Science", "Condensed_Matter", "Astronomy", "Planetary_Science", "Cosmology", "Quantum_Mechanics", "Chemistry"};
    for (auto& d : DOMS) { Lf x{"partC", "fsot_correct_factor(" + d + ")", "engine.hpp:240 m(1+|S|ALPHA), m = 1", "C_S", "(R(1) + (fabs(domain_scalar(\"" + d + "\")) * ALPHA))"}; x.gkey = "partC.fc." + d; x.golden = ENGF->fsot_correct(R(1), d); L.push_back(x); }
    const int DF = ENGF->domain("Fluid_Dynamics").D_eff; const R Dm = AB(R(DF) - 25) / 25;
    auto H = [&](const std::string& nm, const std::string& meaning, const std::string& ex, R tgt) { Lf x{"partC", nm, meaning, "C_H", ex}; x.gkey = "partC.hub." + nm; x.golden = tgt; L.push_back(x); };
    const auto& E = *ENGF;
    H("gamma_rel", "fsot_dynamics.py:143 S relaxation rate |CHAOS| + PSI_CON POOF", "(fabs(CHAOS) + (PSI_CON * POOF))", AB(E.CHAOS) + E.PSI_CON * E.POOF);
    H("kappa", "fsot_dynamics.py:142 S diffusivity (POOF + SUCTION) A_BLEED", "((POOF + SUCTION) * A_BLEED)", (E.POOF + E.SUCTION) * E.A_BLEED);
    H("mu_Fluid", "fsot_dynamics.py:84 viscosity |CHAOS| |D-25|/25 + A_BLEED POOF at D_eff(Fluid_Dynamics) = " + std::to_string(DF), "((fabs(CHAOS) * " + std::to_string((double)Dm) + ") + (A_BLEED * POOF))", AB(E.CHAOS) * Dm + E.A_BLEED * E.POOF);
    H("mu_D25", "fsot_dynamics.py:84 viscosity at D = 25: A_BLEED POOF", "(A_BLEED * POOF)", E.A_BLEED * E.POOF);
    H("J_obs", "fsot_dynamics.py:97 observer source C_FACTOR cos(1 + THETA_S) (observed, delta_psi = 1)", "(C_FACTOR * cos((R(1) + THETA_S)))", E.C_FACTOR * mth::cos(R(1) + E.THETA_S));
    H("c_s2", "fsot_dynamics.py:102 sound speed^2 C_EFF/PHI at rho = 1", "(C_EFF / PHI)", E.C_EFF / E.PHI); }
  { size_t gi = 0; for (auto& lf : L) { if (lf.sec == "partC") continue; for (size_t j = gi; j < gold.size(); ++j) if (gold[j][1] == lf.name && gold[j][0].find("sec." + lf.sec + ".") == 0) { lf.gkey = gold[j][0]; lf.golden = std::stold(gold[j][2]); gi = j + 1; break; } } }
  // parse + verify every leaf against golden, classify
  std::function<void(int, std::set<std::string>&, bool)> ops = [&](int i, std::set<std::string>& o, bool deep) { const Nd& n = D[i]; o.insert(n.op); if (n.op == "id") o.insert("id:" + n.id);
    for (int x : n.a) ops(x, o, deep); for (auto& [c, x] : n.terms) ops(x, o, deep); };
  std::set<int> defnodes; for (auto& [k, v] : DEF) defnodes.insert(v);
  std::function<void(int, std::set<std::string>&)> topops = [&](int i, std::set<std::string>& o) { if (defnodes.count(i)) { o.insert("const"); return; } const Nd& n = D[i]; o.insert(n.op); if (n.op == "id") o.insert("id:" + n.id); for (int x : n.a) topops(x, o); };
  // LOCK5 (2): exact algebraic rewrites. |V_cb| = S_q/C_eff - S_q = S_q (1 - C_eff)/C_eff, and with C_eff = (1 - POOF sin THETA_S)(1 + g),
  // g = G_cat/(pi^4 pi phi): 1 - C_eff = POOF sin(THETA_S) (1 + g) - g. Identical in exact arithmetic; checked against golden below (1e-15).
  const std::map<std::string, std::string> REWRITE = {
    {"|V_cb|", "(S_QUANT * ((POOF * sin(THETA_S)) * (R(1) + (((R(1) / ipow(PI, 4)) * G_CAT) / (PI * PHI))) - (((R(1) / ipow(PI, 4)) * G_CAT) / (PI * PHI))) / C_EFF)"}};
  for (auto& lf : L) { auto rw = REWRITE.find(lf.name); if (rw != REWRITE.end()) { lf.rewrite = lf.cexpr; lf.cexpr = rw->second; } }
  for (auto& lf : L) {
    if (lf.gkey.empty()) { lf.status = "no golden row matched"; continue; }
    if (lf.sec == "partC") { try { lf.node = parse(lf.cexpr); } catch (std::exception& e) { lf.status = std::string("not compiled: ") + e.what(); continue; }
      R rel = (D[lf.node].v - lf.golden) / lf.golden; if (AB(rel) > 1e-15L) { std::fprintf(stderr, "partC %s mismatch %Lg\n", lf.name.c_str(), rel); ++bad; continue; }
      lf.group = lf.tier; lf.status = "compile"; continue; }
    try { lf.node = parse(lf.cexpr); } catch (std::exception& e) { lf.status = std::string("not compiled: ") + e.what(); continue; }
    if (DEV) lf.golden = D[lf.node].v;
    R rel = (D[lf.node].v - lf.golden) / (lf.golden == 0 ? 1 : lf.golden); if (AB(rel) > 1e-15L && !(AB(rel) < 1e-13L && lf.name.find("consistency") != std::string::npos)) { lf.status = "engine expression != golden"; ++bad; continue; }  // Chain_consistency_%: |chain - v1|/v1 cancels ~4 digits in long double
    std::set<std::string> o; topops(lf.node, o); const bool S = o.count("id:S_COSM") || o.count("id:S_QUANT") || o.count("id:S_CHEM");
    if (D[lf.node].op == "num") { lf.status = "literal (tier 0): resistor ratio, no computation"; lf.group = "0"; continue; }
    bool SD = false; for (auto& z : o) if (z.rfind("id:S_D_", 0) == 0) SD = true;
    if (SD || lf.sec == "partC") { lf.group = lf.sec == "partC" ? lf.tier : "C_S"; lf.status = "compile"; continue; }   // LOCK8: netlists C_S / C_H
    std::string tier = S ? "1" : (o.count("const") ? "2" : "2a");
    std::string grp = o.count("floor") ? "Q" : (o.count("sin") || o.count("cos") || o.count("acos")) ? "T" : (o.count("log") || o.count("exp") || (o.count("pow") && 1)) ? "X" : (o.count("add") || o.count("sub")) ? "P" : "M";
    lf.group = tier + "_" + grp; lf.status = "compile"; }
  // netlists: GEN (the 20 derived constants), T1 (tier 1 re-pointed onto the generator), and one per tier x block group
  std::map<std::string, Ckt> NL; std::map<std::string, std::vector<std::string>> prints;
  auto build = [&](const std::string& k, int node, const std::string& label) -> std::pair<std::string, R> {
    Ckt& c = NL[k]; c.tag = k; Lin l = c.LIN(node); std::string o = l.net;
    prints[k].push_back(o); return {o, l.g}; };
  std::vector<std::array<std::string, 6>> genrows;
  for (auto& [nm, ex] : defs) { auto [o, g] = build("GEN", DEF[nm], nm); char b[64]; std::snprintf(b, 64, "%.6Lg", g); char v[64]; std::snprintf(v, 64, "%.12Lg", D[DEF[nm]].v); genrows.push_back({nm, v, o, b, "", ""}); }
  for (auto& lf : L) { if (lf.status != "compile") continue; std::string k = lf.group.substr(0, 1) == "1" ? "T1" : lf.group;
    try { maxrem = 0; auto [o, g] = build(k, lf.node, lf.name); lf.net = o; lf.g = g; lf.status = "compiled";
      std::set<int> seen; std::function<void(int)> walk = [&](int i) { if (!seen.insert(i).second) return; if (REM.count(i)) lf.rem = std::max(lf.rem, REM[i]); for (int x : D[i].a) walk(x); for (auto& [c, x] : D[i].terms) walk(x); }; walk(lf.node); } catch (std::exception& e) { lf.status = std::string("not compiled: ") + e.what(); } }
  NL["T1"].chans.insert("cmq");
  { PPSPLIT = true; auto [o, g] = build("GENpp", DEF["BETA"], "BETA"); PPSPLIT = false; char b[64]; std::snprintf(b, 64, "%.6Lg", g); char v[64]; std::snprintf(v, 64, "%.12Lg", D[DEF["BETA"]].v); genrows.push_back({"BETA_pp", v, "GENpp:" + o, b, "", ""}); }
  // write
  std::FILE* t = std::fopen((out + "/leaves9_targets.tsv").c_str(), "w");
  std::fprintf(t, "gkey\tsection\tname\ttier_group\tnetlist\tengine\tgolden\tnet\tgain\tstatus\tcexpr\tseries_remainder_max\trewritten_from_engine_cexpr\n");
  for (auto& lf : L) { std::string k = lf.group.empty() ? "-" : (lf.group[0] == '1' ? "T1" : lf.group);
    char gb[64]; std::snprintf(gb, 64, lf.sec == "partC" ? "%.12Lg" : "%.21Lg", lf.golden);   // 8.1 (b): partC targets are computed, not read
    std::fprintf(t, "%s\t%s\t%s\t%s\t%s\t%.12Lg\t%s\t%s\t%.6Lg\t%s\t%s\t%.2e\t%s\n", lf.gkey.c_str(), lf.sec.c_str(), lf.name.c_str(), lf.group.c_str(), lf.status == "compiled" ? k.c_str() : "-",
      lf.node >= 0 ? D[lf.node].v : 0.0L, gb, lf.net.empty() ? "-" : lf.net.c_str(), lf.g, lf.status.c_str(), lf.cexpr.c_str(), (double)lf.rem, lf.rewrite.empty() ? "-" : lf.rewrite.c_str()); }
  std::fclose(t);
  std::FILE* gf = std::fopen((out + "/generator9.tsv").c_str(), "w"); std::fprintf(gf, "constant\tengine\tnet\tgain\n");
  for (auto& r : genrows) std::fprintf(gf, "%s\t%s\t%s\t%s\n", r[0].c_str(), r[1].c_str(), r[2].c_str(), r[3].c_str()); std::fclose(gf);
  std::FILE* sm = std::fopen((out + "/netlists9.tsv").c_str(), "w"); std::fprintf(sm, "netlist\tleaves\tparts\topamps\tlog\tantilog\tsummers\tlinear_sums\tseries_terms\tcomparators\tbase_refs\tR_strings\tcancelling_sums\tfullwave\tantilog_uncascoded\tearly_corrected\trange_sized\tpow_split\tconst_abs_mean_before\tconst_abs_mean_after\tconst_abs_max_after\n");
  for (auto& [k, c] : NL) {
    std::FILE* r = std::fopen((out + "/recipe9_" + k + ".txt").c_str(), "w"); for (auto& ch : c.chans) { if (ch == "cmq") std::fprintf(r, "CHAN\n"); else if (ch.rfind("D:", 0) == 0) std::fprintf(r, "DCHAN\t%s\t%s\n", DPREF[ch.substr(2)].c_str(), ch.substr(2).c_str()); }
    for (auto& l : c.rec) std::fprintf(r, "%s\n", l.c_str()); for (auto& p : prints[k]) std::fprintf(r, "PRINT\t%s\n", p.c_str()); std::fclose(r);
    std::FILE* v = std::fopen((out + "/design9_" + k + "_values.tsv").c_str(), "w"); std::fprintf(v, "name\tideal\tE24_a\tE24_b\tE24_c\tE24_rel\tunit\tmeaning\n");
    for (auto& p : c.parts) { char sb[64]; std::snprintf(sb, 64, "%.12Lg", p.ideal); const R x12 = std::strtold(sb, nullptr);   // 8.1 (a)
      auto e = ckt::realise(x12); std::fprintf(v, "%s\t%.12Lg\t%.6Lg\t%.6Lg\t%.6Lg\t%.3Le\t%s\t%s\n", p.name.c_str(), x12, e.a, e.b, e.c, e.rel, p.unit.c_str(), p.meaning.c_str()); }
    std::fclose(v);
    std::fprintf(sm, "%s\t%zu\t%zu\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%.3Lf\t%.3Lf\t%.3Lf\n", k.c_str(), prints[k].size(), c.parts.size(), c.stats["opamp"], c.stats["log"], c.stats["antilog"], c.stats["summer"], c.stats["linear_sum"], c.stats["series_terms"], c.stats["comparator"], c.stats["base_ref"], c.stats["R>27M_strings"], c.stats["cancelling_sum"], c.stats["fullwave"], c.stats["antilog_uncascoded"], c.stats["early_corrected"], c.stats["range_sized"], c.stats["pow_split"],
      [&]{ R z = 0; for (auto& q : c.CALS) z += AB(q[0]); return c.CALS.empty() ? 0.0L : z / c.CALS.size(); }(), [&]{ R z = 0; for (auto& q : c.CALS) z += AB(q[2]); return c.CALS.empty() ? 0.0L : z / c.CALS.size(); }(),
      [&]{ R z = 0; for (auto& q : c.CALS) z = std::max(z, AB(q[2])); return z; }()); }
  std::fclose(sm);
  std::map<std::string, int> cnt; for (auto& lf : L) cnt[lf.status.substr(0, std::min<size_t>(lf.status.size(), 40))]++;
  for (auto& [k, v] : cnt) std::printf("%4d  %s\n", v, k.c_str());
  std::printf("LEAFC_%s leaves=%zu netlists=%zu mismatches=%d\n", bad ? "FAIL" : "OK", L.size(), NL.size(), bad);
  return bad ? 1 : 0;
}
