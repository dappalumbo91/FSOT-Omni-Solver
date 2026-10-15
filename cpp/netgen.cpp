// netgen.cpp — builds ngspice netlists of the Phase-1 slice from the LOCKED values (../locked/design_values.tsv).
// Usage: netgen <slice|foldsweep|foldsteps> <variant> <seed> <out.cir> [tempC]
//   variant: ideal_ideal (exact values, ideal op-amp) | ideal_tl082 | e24_tl082 | mcA | mcB | mcC
#include <cmath>
#include <cstdio>
#include <fstream>
#include <map>
#include <random>
#include <sstream>
#include <string>
#include <vector>

struct PartV { double ideal, a, b, c; };
static std::map<std::string, PartV> P;
static std::map<std::string, double> T;  // targets
struct Var { bool e24 = true, ideal_op = false; double tolR = 0, vos = 0, vbe = 0, vref = 0, ron_lo = 125, ron_hi = 125; };
static Var V; static std::mt19937_64 rng; static std::ostringstream N, M;
static int nC = 0; static bool TRAN = false;
double U(double lo, double hi) { return std::uniform_real_distribution<double>(lo, hi)(rng); }
double G(double s, double clip) { if (s == 0) return 0; double x; do { x = std::normal_distribution<double>(0, s)(rng); } while (std::fabs(x) > clip); return x; }

void load() {
  std::ifstream f("../locked/design_values.tsv"); std::string line; std::getline(f, line);
  while (std::getline(f, line)) { std::istringstream s(line); std::string n, id, a, b, c, r; std::getline(s, n, '\t'); std::getline(s, id, '\t'); std::getline(s, a, '\t'); std::getline(s, b, '\t'); std::getline(s, c, '\t');
    P[n] = {std::stod(id), std::stod(a), std::stod(b), std::stod(c)}; }
  std::ifstream t("../locked/fsot_targets.tsv"); while (std::getline(t, line)) { if (line.empty() || line[0] == '#' || line.rfind("name", 0) == 0) continue; std::istringstream s(line); std::string n, v; std::getline(s, n, '\t'); std::getline(s, v, '\t'); T[n] = std::stod(v); }
}
// resistor from locked part, realised (E24 series string) or ideal, each physical piece with its own tolerance draw
void Rp(const std::string& nm, const std::string& n1, const std::string& n2, const std::string& part, double mult = 1) {
  const PartV& p = P.at(part);
  if (!V.e24) { N << "R" << nm << " " << n1 << " " << n2 << " " << p.ideal * mult * (1 + U(-V.tolR, V.tolR)) << "\n"; return; }
  std::vector<double> pcs; for (double x : {p.a, p.b, p.c}) if (x > 0) pcs.push_back(x * mult);
  std::string prev = n1;
  for (size_t i = 0; i < pcs.size(); ++i) { std::string nx = (i + 1 == pcs.size()) ? n2 : (nm + "_m" + std::to_string(i));
    N << "R" << nm << "_" << i << " " << prev << " " << nx << " " << pcs[i] * (1 + (V.tolR > 0 ? U(-V.tolR, V.tolR) : 0)) << "\n"; prev = nx; }
}
void OA(const std::string& nm, const std::string& pl, const std::string& mi, const std::string& out) {
  const double vos = G(V.vos, 5 * V.vos);
  N << "VOS_" << nm << " " << nm << "_p " << pl << " DC " << vos << "\n";
  if (V.ideal_op) N << "E_" << nm << " " << out << " 0 " << nm << "_p " << mi << " 1e6\n";
  else N << "X_" << nm << " " << nm << "_p " << mi << " vcc vee " << out << " TL082\n";
}
// comparator (LM339/LM393 class): behavioral, open-loop gain ~1.4e5, output +-6.8 V, input offset drawn like the op-amps
void CMP(const std::string& nm, const std::string& pl, const std::string& mi, const std::string& out) {
  const double vos = G(V.vos, 5 * V.vos);
  N << "B_" << nm << " " << out << " 0 V = 6.8*tanh(2e4*(V(" << pl << ")-V(" << mi << ")+(" << vos << ")))\n";
}
void Q(const std::string& nm, const std::string& c, const std::string& b, const std::string& e) {
  const double dv = G(V.vbe, 5 * V.vbe); const double IS = 6.734e-15 * std::exp(-dv / 0.025865);
  M << ".model QM_" << nm << " NPN(IS=" << IS << " XTI=3 EG=1.11 VAF=74.03 BF=416.4 NE=1.259 ISE=6.734f IKF=66.78m XTB=1.5 BR=.7371 NC=2 ISC=0 IKR=0 RC=1 CJC=3.638p MJC=.3085 VJC=.75 FC=.5 CJE=4.493p MJE=.2593 VJE=.75 TR=239.5n TF=301.2p ITF=.4 VTF=4 XTF=2 RB=10)\n";
  N << "Q_" << nm << " " << c << " " << b << " " << e << " QM_" << nm << "\n";
}
void SW(const std::string& nm, const std::string& a, const std::string& b, const std::string& ctl) {
  const double ron = U(V.ron_lo, V.ron_hi + 1e-9);
  M << ".model SWM_" << nm << " SW(VT=0 VH=0.2 RON=" << ron << " ROFF=1e12)\n";
  N << "S_" << nm << " " << a << " " << b << " " << ctl << " 0 SWM_" << nm << "\n";
}
void SWn(const std::string& nm, const std::string& a, const std::string& b, const std::string& ctl) {  // complementary throw
  const double ron = U(V.ron_lo, V.ron_hi + 1e-9);
  M << ".model SWM_" << nm << " SW(VT=0 VH=0.2 RON=" << ron << " ROFF=1e12)\n";
  N << "S_" << nm << " " << a << " " << b << " 0 " << ctl << " SWM_" << nm << "\n";
}
void refs() {
  { const double vref = 2.5 * (1 + U(-V.vref, V.vref + 1e-15));
    if (TRAN) N << "VCC vcc 0 9\nVEE vee 0 -9\nVREF vr 0 PWL(0 0.05 3m " << vref << ")\n";
    else N << "VCC vcc 0 9\nVEE vee 0 -9\nVREF vr 0 DC " << vref << "\n"; }
  OA("inv", "0", "ninv", "vrn"); Rp("inv1", "vr", "ninv", "R_inv"); Rp("inv2", "ninv", "vrn", "R_inv");
  // threshold string (shared): taps th5..th24 at VR*g_k/20
  Rp("str5", "0", "th5", "R_str_5");
  for (int k = 6; k <= 24; ++k) Rp("str" + std::to_string(k), "th" + std::to_string(k - 1), "th" + std::to_string(k), "R_str_" + std::to_string(k));
  Rp("strtop", "th24", "vr", "R_str_top");
}
void gstring() {  // generation selector string (shared): tap tg_g = VR*g/20
  for (int i = 0; i < 20; ++i) Rp("g" + std::to_string(i), i == 0 ? "0" : "tg" + std::to_string(i), i == 19 ? "vr" : "tg" + std::to_string(i + 1), "R_gunit");
}
// fold ladder: comparators on g against g_k thresholds switch 1-unit currents into LOGR1 Q2 collector
void ladder(const std::string& p, const std::string& gnode) {
  for (int k = 5; k <= 24; ++k) { std::string ks = std::to_string(k);
    CMP("cmp" + p + ks, gnode, "th" + ks, "ct" + p + ks);
    Rp("st" + p + ks, "vr", "sk" + p + ks, "R_step");
    SW(p + ks, "sk" + p + ks, "c2in" + p, "ct" + p + ks); SWn(p + ks + "n", "sk" + p + ks, "0", "ct" + p + ks); }
  Rp("base" + p, "vr", "c2in" + p, "R_base");
  N << "VID" << p << " c2in" << p << " c2" << p << " DC 0\n";
}
// core law block: f(D) current I0*f, TIA -> vf
void core(const std::string& p) {
  // LOGR1: u = V_T ln(D/25) at node u<p>
  N << "VI25" << p << " vr i25" << p << " DC 0\n"; Rp("25" + p, "i25" + p, "c1" + p, "R_25");
  Q("1" + p, "c1" + p, "0", "e" + p); OA("A" + p, "0", "c1" + p, "e" + p);
  Q("2" + p, "c2" + p, "u" + p, "e" + p); OA("B" + p, "c2" + p, "0", "u" + p);
  N << "CA" << p << " e" << p << " c1" << p << " 1n\nCB" << p << " c2" << p << " 0 1n\n";
  // antilog: Q3 current I0(1 + P_new u), base -> u/2, Q4 = I0 (1+P_new u) e^{-u/2}
  Rp("I0" + p, "vr", "d1" + p, "R_I0"); Rp("x" + p, "d1" + p, "u" + p, "R_x");
  Rp("h1" + p, "u" + p, "b1" + p, "R_half"); Rp("h2" + p, "b1" + p, "0", "R_half");
  Q("3" + p, "d1" + p, "b1" + p, "ea" + p); Q("4" + p, "d2" + p, "0", "ea" + p); OA("C" + p, "0", "d1" + p, "ea" + p);
  N << "CC" << p << " ea" << p << " d1" << p << " 1n\n";
  OA("T" + p, "0", "d2" + p, "vf" + p); Rp("Tf" + p, "vf" + p, "d2" + p, "R_Tf"); N << "CT" << p << " vf" << p << " d2" << p << " 100p\n";
}
void sstage(const std::string& p, bool obs, const std::string& out) {
  std::string x = "vf" + p;
  if (obs) { OA("FI" + p, "0", "nfi" + p, "vfn" + p); Rp("fi1" + p, "vf" + p, "nfi" + p, "R_finv"); Rp("fi2" + p, "nfi" + p, "vfn" + p, "R_finv"); x = "vfn" + p; }
  OA("S" + out, "0", "ns" + out, out); Rp("Sx" + out, x, "ns" + out, obs ? "R_Sx_obs" : "R_Sx_med"); Rp("S0" + out, "vrn", "ns" + out, "R_S0"); Rp("Sf" + out, out, "ns" + out, "R_Sf");
}
std::string header(const std::string& what) {
  std::ostringstream h;
  h << "* FSOT analog solver Phase 1 — " << what << " (values from locked/design_values.tsv)\n";
  h << ".include ../models/vendor/ti_sloj070/TL082.301\n";
  h << ".model D1N4148 D(Is=2.52n Rs=0.568 N=1.752 Cjo=4p M=0.4 tt=20n BV=100)\n";
  return h.str();
}
int main(int argc, char** argv) {
  if (argc < 5) { std::fprintf(stderr, "usage\n"); return 1; }
  std::string mode = argv[1], var = argv[2], out = argv[4]; unsigned long seed = std::stoul(argv[3]); double temp = argc > 5 ? std::stod(argv[5]) : 27;
  rng.seed(seed * 7919 + 17); load(); std::string opt = argc > 6 ? argv[6] : "op"; if (opt == "tran") TRAN = true; const bool FORCE_IDEAL = (opt == "idealop" || opt == "dump"); std::string nsfile = argc > 7 ? argv[7] : "";
  if (var == "ideal_ideal") { V.e24 = false; V.ideal_op = true; }
  else if (var == "ideal_tl082") { V.e24 = false; }
  else if (var == "e24_tl082") { }
  else if (var == "mcA") { V.tolR = 0.01; V.vos = 3e-3; V.vbe = 1e-3; V.vref = 0.005; V.ron_lo = 75; V.ron_hi = 175; }
  else if (var == "mcB") { V.tolR = 0.001; V.vos = 25e-6; V.vbe = 1e-4; V.vref = 0.001; V.ron_lo = 75; V.ron_hi = 175; }
  else if (var == "mcC") { V.tolR = 0.0002; V.vos = 2e-6; V.vbe = 1e-5; V.vref = 0.0005; V.ron_lo = 75; V.ron_hi = 175; }
  else { std::fprintf(stderr, "bad variant\n"); return 1; }
  if (FORCE_IDEAL) V.ideal_op = true;
  std::string what;
  if (mode == "slice") {
    what = "vertical slice: core law x3 folds, m_H/m_W, Omega_b h^2, alpha_s, 1/alpha_em";
    refs(); gstring();
    const std::vector<std::pair<std::string, int>> ch = {{"q", 1}, {"c", 3}, {"m", 19}};  // QM g=1, Chemistry g=3, Cosmology g=19
    for (auto& [p, g] : ch) { ladder(p, "tg" + std::to_string(g)); core(p); }
    sstage("q", true, "vsq"); sstage("c", true, "vsc"); sstage("m", false, "vsm");
    // m_H/m_W = S_quant (1 + psi_con)
    OA("M", "vsq", "nm", "vmhw"); Rp("mg", "nm", "0", "R_mg"); Rp("mf", "vmhw", "nm", "R_mf");
    // |S_cosm|: precision full-wave
    OA("FW1", "0", "n1fw", "o1fw"); Rp("fwi", "vsm", "n1fw", "R_fw"); Rp("fwf", "n1fw", "hfw", "R_fw");
    N << "DFW1 hfw o1fw D1N4148\nDFW2 o1fw n1fw D1N4148\n";
    OA("FW2", "0", "n2fw", "vabs"); Rp("fwx", "vsm", "n2fw", "R_fw2"); Rp("fwh", "hfw", "n2fw", "R_fw"); Rp("fwo", "vabs", "n2fw", "R_fw2");
    // 40 (1 - S_chem)
    OA("C1S", "0", "nc", "v1s"); Rp("cin", "vsc", "nc", "R_c_in"); Rp("cref", "vrn", "nc", "R_c_ref"); Rp("cf", "v1s", "nc", "R_c_f");
    // Omega_b h^2: LOGR5 (-V_T ln|S|) -> antilog with Q current (1-S) I0
    Rp("oma", "vabs", "o1n", "R_om_a"); Q("5", "o1n", "0", "oe"); OA("OA", "0", "o1n", "oe");
    Rp("omr", "vr", "o2n", "R_om_ref"); Q("6", "o2n", "ol", "oe"); OA("OB", "o2n", "0", "ol");
    N << "COA oe o1n 1n\nCOB o2n 0 1n\n";
    Rp("omb", "v1s", "o3n", "R_om_b"); Q("7", "o3n", "ol", "oe2"); Q("8", "o4n", "0", "oe2"); OA("OC", "0", "o3n", "oe2");
    N << "COC oe2 o3n 1n\n";
    OA("OD", "0", "o4n", "vomb"); Rp("omT", "vomb", "o4n", "R_om_T"); N << "COD vomb o4n 100p\n";
    // alpha_s = e^{-1}/pi
    Rp("aspi", "vr", "a1n", "R_as_pi"); Rp("asvt", "vr", "ab", "R_vt_top"); Rp("asvb", "ab", "0", "R_vt_bot");
    Q("9", "a1n", "ab", "ae"); Q("10", "a2n", "0", "ae"); OA("AA", "ab", "a1n", "ae"); N << "CAA ae a1n 1n\n";
    OA("AT", "0", "a2n", "vas"); Rp("asT", "vas", "a2n", "R_as_T"); N << "CAT vas a2n 100p\n";
    // 1/alpha_em = e^3 (3 phi + 2) - 1 + e^{-1}
    Rp("a3p", "vr", "e1n", "R_a3phi"); Rp("a2", "vr", "e1n", "R_a2"); Rp("3vt", "vrn", "eb", "R_3vt_top"); Rp("3vb", "eb", "0", "R_3vt_bot");
    Q("11", "e1n", "eb", "ee"); Q("12", "t1n", "0", "ee"); OA("EA", "eb", "e1n", "ee"); N << "CEA ee e1n 1n\n";
    Rp("au1", "vr", "f1n", "R_au"); Rp("fvt", "vr", "fb", "R_vt_top"); Rp("fvb", "fb", "0", "R_vt_bot");
    Q("13", "f1n", "fb", "fe"); Q("14", "t1n", "0", "fe"); OA("FA", "fb", "f1n", "fe"); N << "CFA fe f1n 1n\n";
    Rp("au2", "vr", "t1n", "R_inj");
    OA("IT", "0", "t1n", "vainv"); Rp("aiT", "vainv", "t1n", "R_ainv_T"); N << "CIT vainv t1n 100p\n";
  } else if (mode.rfind("foldg", 0) == 0) {  // foldg<g>: one fold channel, generation g selected on the physical g-string
    const int g = std::stoi(mode.substr(5)); what = "fold block + core law at generation g = " + std::to_string(g) + " (g-string tap)";
    refs(); gstring(); ladder("z", g == 0 ? "0" : "tg" + std::to_string(g)); core("z"); sstage("z", true, "vso"); sstage("z", false, "vsd");
  } else if (mode == "foldsweep" || mode == "foldsteps") {
    what = "fold block + core law over the generation input g (DC sweep)";
    refs(); N << "VGS gs 0 DC 0\n"; ladder("z", "gs"); core("z"); sstage("z", true, "vso"); sstage("z", false, "vsd");
  }
  std::ofstream o(out);
  o << header(what) << "* variant " << var << " seed " << seed << " temp " << temp << " opt " << opt << "\n" << M.str() << N.str();
  if (!nsfile.empty()) {  // DC initial guess = same draw solved with ideal op-amps (solver aid only; ngspice then iterates freely)
    std::ifstream ns(nsfile); std::string l; const std::string body = N.str();
    while (std::getline(ns, l)) { auto eq = l.find(" = "); if (eq == std::string::npos) continue; std::string nd = l.substr(0, eq);
      if (body.find(" " + nd + " ") == std::string::npos && body.find(" " + nd + "\n") == std::string::npos) continue;
      o << ".nodeset v(" << nd << ")=" << l.substr(eq + 3) << "\n"; }
  }
  o << ".options temp=" << temp << " tnom=27 reltol=1e-6 abstol=1e-13 vntol=1e-8 gmin=1e-15 itl1=500\n.control\n";
  if (mode == "slice") {
    const char* nodes[] = {"vsq", "vsc", "vsm", "vfq", "vfc", "vfm", "vmhw", "vabs", "v1s", "vomb", "vas", "vainv", "uq", "uc", "um", "vr"};
    const char* curs[] = {"vidq", "vi25q", "vidc", "vi25c", "vidm", "vi25m"};
    if (opt == "dump") { o << "op\nprint all\n"; }
    else if (!TRAN) { o << "op\nprint"; for (auto x : nodes) o << " v(" << x << ")"; o << "\nprint"; for (auto x : curs) o << " i(" << x << ")"; o << "\n"; }
    else {
      o << "tran 20u 40m 0 20u\nlet n = length(time) - 1\nlet k = n - 500\n";  // last 10 ms window
      for (auto x : nodes) o << "let fin_" << x << " = v(" << x << ")[n]\nlet rip_" << x << " = vecmax(v(" << x << ")[k,n]) - vecmin(v(" << x << ")[k,n])\nprint fin_" << x << " rip_" << x << "\n";
      for (auto x : curs) o << "let fin_i" << x << " = i(" << x << ")[n]\nprint fin_i" << x << "\n";
    }
  } else if (mode.rfind("foldg", 0) == 0) {
    o << "op\nprint i(vidz) i(vi25z) v(vfz) v(vso) v(vsd)\n";
  } else if (mode == "foldsweep") {
    o << "dc VGS 0 2.4 0.0025\nwrdata out/" << out.substr(out.rfind('/') + 1) << ".dat v(gs) i(vidz) i(vi25z) v(vfz) v(vso) v(vsd)\n";
  } else {
    o << "dc VGS 0 2.375 0.125\nwrdata out/" << out.substr(out.rfind('/') + 1) << ".dat v(gs) i(vidz) i(vi25z) v(vfz) v(vso) v(vsd)\n";
  }
  o << "quit\n.endc\n.end\n";
}
