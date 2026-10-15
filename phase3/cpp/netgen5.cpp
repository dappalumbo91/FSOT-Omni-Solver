// netgen5.cpp — FSOT Omni-Solver Phase 3d (LOCK6): fork of phase3/cpp/netgen4.cpp (LOCK5, unchanged). Change, recorded in phase3/locked6/LOCK6.md:
//  every base-engine tiny tap (taps5.tsv "uncascoded", term < 1e-4) is CARRIED in the log domain instead of being one uncascoded transistor:
//  its log voltage vx (= V_T ln t) is lifted by the locked integer n nepers (phase3/locked6/taps6.tsv, from the PTAT V_NEP bus), the scaled
//  term I_U t e^n (49..92 nA) is made by a cascoded antilog and read on a TIA (v(vlc<tap>) = I_U t e^n R_lcT), then divided back by e^-n in a
//  matched antilog (input v(vlc)/R_lcT, base at -n V_T from the same bus) whose uncascoded output (sensed, VSE_/VSB_) is the term injected into
//  the engine's summing node. Mode "tapbench <fold>" (transient) compares, per tiny tap of one fold, the direct uncascoded stage, the log-carry and a
//  charge-integrator readout (C_int, T_int locked) driven from the same stimulus base voltage V_T ln t.
//  Also: the leafc5 FWR rectifier uses diode-connected 2N3904s (low reverse current) instead of 1N4148.
// netgen4.cpp — FSOT Omni-Solver Phase 3c (LOCK5): fork of phase2/cpp/netgen2.cpp (locked, unchanged). Changes, recorded in phase3/locked5/LOCK5.md:
//  (a) base-engine tiny taps: a power-series antilog tap (cy_k, sw_k, sv_k, st_k) or the T3 tap whose locked nominal term (from
//      folds35_targets: x^{2k}/n_k resp. |T3|, in units of I_U = 5 uA) is below 1e-4 is built without the 2N7000 output cascode
//      (CALU: Q4 collector straight into the virtual-ground node). The decision per fold is the "taps" mode table, locked before simulation.
//  (b) each tap's Q4 emitter and base pass 0 V sense sources VSE_/VSB_<tap> (I_c = i(vse) - i(vsb)) so the analog value is read against the engine;
//  (c) mode leafc5 (FSOT_P5_LOCKED, design5_/recipe5_) with the FWR (precision full-wave rectifier, own part names) primitive.
// netgen2.cpp — ngspice netlists of the Phase-2 full-core-law channel from the LOCKED values (../locked/).
// Usage: netgen2 <mode> <variant> <seed> <out.cir> [tempC] [analysis] [fold]
//   mode:     fold (one full-core-law channel, fold selected by its inputs) | omb (Chemistry + Cosmology channels,
//             current-mode 1 - S_chem, Omega_b h^2) | consts (PTAT, alpha_s, 1/alpha via three e^1 stages) | ptat
//   variant:  ideal_ideal | ideal_opa | e24_opa | mcB | mcC  (mc*: E24 + tolerance draws, ideal op-amp + Vos draw)
//   analysis: op | tran | dump | noise | ac_<block>
#include <cctype>
#include <cmath>
#include <cstdlib>
#include <cstdio>
#include <fstream>
#include <map>
#include <random>
#include <sstream>
#include <string>
#include <vector>

struct PartV { double ideal, a, b, c; };
static std::map<std::string, PartV> P;
static std::map<std::string, double> C;  // summer constants (nepers)
struct Var { bool e24 = true, ideal_op = false, vendor_cmp = true; double tolR = 0, tc = 0, vos = 0, vbe = 0, vref = 0, beta = 0, vaf = 0, ron_lo = 125, ron_hi = 125; };
static Var V; static std::mt19937_64 rng; static std::ostringstream N, M;
static bool TRAN = false, OPDYN = false, OPNOISE = false; static int STOP = 99;  // STOP: debug-only partial build (env FSOT_P2_STOP); never used for results
double U(double lo, double hi) { return std::uniform_real_distribution<double>(lo, hi)(rng); }
double G(double s, double clip) { if (s == 0) return 0; double x; do { x = std::normal_distribution<double>(0, s)(rng); } while (std::fabs(x) > clip); return x; }

static std::string LK = "../locked";
void load() {
  std::ifstream f(LK + "/design2_values.tsv"); std::string line; std::getline(f, line);
  while (std::getline(f, line)) { std::istringstream s(line); std::string n, id, a, b, c; std::getline(s, n, '\t'); std::getline(s, id, '\t'); std::getline(s, a, '\t'); std::getline(s, b, '\t'); std::getline(s, c, '\t');
    P[n] = {std::stod(id), std::stod(a), std::stod(b), std::stod(c)}; }
  std::ifstream c(LK + "/design2_consts.tsv"); std::getline(c, line);
  while (std::getline(c, line)) { std::istringstream s(line); std::string n, v; std::getline(s, n, '\t'); std::getline(s, v, '\t'); C[n] = std::stod(v); }
}
void load3(const std::string& d) {  // Phase 3 (leaf tier 1) locked values, same format
  std::ifstream f(d + "/design3_values.tsv"); std::string line; std::getline(f, line);
  while (std::getline(f, line)) { std::istringstream s(line); std::string n, id, a, b, c; std::getline(s, n, '\t'); std::getline(s, id, '\t'); std::getline(s, a, '\t'); std::getline(s, b, '\t'); std::getline(s, c, '\t');
    P[n] = {std::stod(id), std::stod(a), std::stod(b), std::stod(c)}; }
  std::ifstream c(d + "/design3_consts.tsv"); std::getline(c, line);
  while (std::getline(c, line)) { std::istringstream s(line); std::string n, v; std::getline(s, n, '\t'); std::getline(s, v, '\t'); C[n] = std::stod(v); }
}
void Rp(const std::string& nm, const std::string& n1, const std::string& n2, const std::string& part) {
  const PartV& p = P.at(part);
  auto tcs = [&]() { std::ostringstream t; if (V.tc > 0) t << " tc1=" << U(-V.tc, V.tc); return t.str(); };
  if (!V.e24) { N << "R" << nm << " " << n1 << " " << n2 << " " << p.ideal * (1 + (V.tolR > 0 ? U(-V.tolR, V.tolR) : 0)) << tcs() << "\n"; return; }
  std::vector<double> pcs; for (double x : {p.a, p.b, p.c}) if (x > 0) pcs.push_back(x);
  std::string prev = n1;
  for (size_t i = 0; i < pcs.size(); ++i) { std::string nx = (i + 1 == pcs.size()) ? n2 : (nm + "_m" + std::to_string(i));
    N << "R" << nm << "_" << i << " " << prev << " " << nx << " " << pcs[i] * (1 + (V.tolR > 0 ? U(-V.tolR, V.tolR) : 0)) << tcs() << "\n"; prev = nx; }
}
void OA(const std::string& nm, const std::string& pl, const std::string& mi, const std::string& out) {
  const double vos = G(V.vos, 5 * V.vos);
  // Amendment 12 (noise runs, env FSOT_P2_OPNOISE=1): OPA187 input voltage noise 20 nV/rtHz as the thermal noise of a 24.15 k
  // resistor in series with the non-inverting input (no DC current flows in it: ideal input). Current noise is not modelled.
  if (OPNOISE) N << "VOS_" << nm << " " << nm << "_p " << nm << "_q DC " << vos << "\nRN_" << nm << " " << nm << "_q " << pl << " 24.15k\n";
  else N << "VOS_" << nm << " " << nm << "_p " << pl << " DC " << vos << "\n";
  // ideal op-amp: gain 1e6. Amendment 7 (start-up runs, env FSOT_P2_OPDYN=1): OPA187 large-signal behaviour as a Gm-C stage:
  // gm = 1 S into R = 1 Meg (DC gain 1e6) || C = 0.2894 uF (GBW 0.55 MHz), current limited to 58 mA (slew 0.2 V/us), and the
  // integrator clamped at the +-8.9 V swing (no wind-up); unity buffer to the output. DC inside the rails: identical to gain 1e6.
  if (V.ideal_op && OPDYN) N << "B_" << nm << "_g 0 " << nm << "_f I = max(-0.058, min(0.058, V(" << nm << "_p)-V(" << mi << ")))\nR_" << nm << "_o " << nm << "_f 0 1Meg\nC_" << nm << "_o " << nm << "_f 0 0.2894u\n"
                             << "B_" << nm << "_c " << nm << "_f 0 I = 100*(max(V(" << nm << "_f)-8.9,0)+min(V(" << nm << "_f)+8.9,0))\n"
                             << "E_" << nm << " " << out << " 0 " << nm << "_f 0 1\n";
  else if (V.ideal_op) N << "E_" << nm << " " << out << " 0 " << nm << "_p " << mi << " 1e6\n";
  else N << "X_" << nm << " " << nm << "_p " << mi << " vcc vee " << out << " OPAx187\n";
}
void CMP(const std::string& nm, const std::string& pl, const std::string& mi, const std::string& out) {
  const double vos = G(V.vos, 5 * V.vos);
  if (V.vendor_cmp) { N << "VOS_" << nm << " " << nm << "_p " << pl << " DC " << vos << "\nX_" << nm << " " << nm << "_p " << mi << " vcc vee " << out << " LM139\n"; Rp("pu" + nm, "vcc", out, "R_pullup"); }
  else N << "B_" << nm << " " << out << " 0 V = 6.8*tanh(2e4*(V(" << pl << ")-V(" << mi << ")+(" << vos << ")))\n";
}
void Q(const std::string& nm, const std::string& c, const std::string& b, const std::string& e) {
  const double dv = G(V.vbe, 5 * V.vbe); const double IS = 6.734e-15 * std::exp(-dv / 0.025865);
  const double bf = 416.4 * (1 + (V.beta > 0 ? U(-V.beta, V.beta) : 0)), vaf = 74.03 * (1 + (V.vaf > 0 ? U(-V.vaf, V.vaf) : 0));
  M << ".model QM_" << nm << " NPN(IS=" << IS << " XTI=3 EG=1.11 VAF=" << vaf << " BF=" << bf << " NE=1.259 ISE=6.734f IKF=66.78m XTB=1.5 BR=.7371 NC=2 ISC=0 IKR=0 RC=1 CJC=3.638p MJC=.3085 VJC=.75 FC=.5 CJE=4.493p MJE=.2593 VJE=.75 TR=239.5n TF=301.2p ITF=.4 VTF=4 XTF=2 RB=10)\n";
  N << "Q_" << nm << " " << c << " " << b << " " << e << " QM_" << nm << "\n";
}
void MOS(const std::string& nm, const std::string& d, const std::string& g, const std::string& s) { N << "M_" << nm << " " << d << " " << g << " " << s << " M2N7000\n"; }
// SPDT (CD4053 pole): com <-> a when ctl > 0, com <-> b otherwise
void SPDT(const std::string& nm, const std::string& com, const std::string& a, const std::string& b, const std::string& ctl) {
  const double r1 = U(V.ron_lo, V.ron_hi + 1e-9), r2 = U(V.ron_lo, V.ron_hi + 1e-9);
  M << ".model SWa_" << nm << " SW(VT=0 VH=0.2 RON=" << r1 << " ROFF=1e12)\n.model SWb_" << nm << " SW(VT=0 VH=0.2 RON=" << r2 << " ROFF=1e12)\n";
  N << "Sa_" << nm << " " << com << " " << a << " " << ctl << " 0 SWa_" << nm << "\n";
  N << "Sb_" << nm << " " << com << " " << b << " 0 " << ctl << " SWb_" << nm << "\n";
}
// cascoded log-ratio: Ix summed into node <p>c1 (transdiode Q1), Ir into <p>c2 (cascoded Q2); out n = V_T ln(Ir/Ix)
void LOGR(const std::string& p, const std::string& n) {
  Q(p + "1", p + "c1", "0", p + "e"); OA(p + "A", "0", p + "c1", p + "e");
  Q(p + "2", p + "k", n, p + "e"); MOS(p, p + "c2", p + "g", p + "k");
  // Amendment 9 (stability): the cascode servo D is an integrator (100 k from k, 1 nF from g to its inverting input; ~1.6 kHz),
  // well below the k-node pole of the low-current 2N7000 (g_m/C_k ~ 16-160 kHz). No DC current flows in the 100 k.
  OA(p + "D", n, p + "kd", p + "g"); N << "RD" << p << " " << p << "k " << p << "kd 100k\nCD" << p << " " << p << "g " << p << "kd 100p\n";  // Amendment 11: 100 pF
  OA(p + "B", p + "c2", "0", n);
  // Amendment 8 (stability): the c2 integrating capacitor gets a series zero resistor, 100 nF + 1 k (was 1 nF alone, phase margin
  // -75...-158 deg with an OPA187-class 0.55 MHz op-amp). DC is unchanged (capacitor open).
  N << "CA" << p << " " << p << "e " << p << "c1 1n\nCB" << p << " " << p << "c2 " << p << "cz 10u\nRZ" << p << " " << p << "cz 0 330\n";  // Amendment 10: 10 uF + 330 R
}
// cascoded antilog: Iin into <p>d (transdiode Q3); Q4 base vx; Iout = Iin exp(vx/V_T) sunk from each node in outs
void CAL(const std::string& p, const std::string& vx, const std::vector<std::string>& outs) {
  Q(p + "3", p + "d", "0", p + "ea"); OA(p + "C", "0", p + "d", p + "ea"); N << "CC" << p << " " << p << "ea " << p << "d 1n\n";
  for (size_t i = 0; i < outs.size(); ++i) { std::string s = p + (i ? "b" : "");
    Q(s + "4", s + "k4", vx, p + "ea"); MOS(s + "4", outs[i], s + "g4", s + "k4"); OA(s + "E", vx, s + "k4", s + "g4"); }
}
void TIA(const std::string& p, const std::string& node, const std::string& out, const std::string& part) {
  OA("T" + p, "0", node, out); Rp("T" + p, out, node, part); N << "CT" << p << " " << out << " " << node << " 100p\n";
}
void BUF(const std::string& p, const std::string& in, const std::string& out) { OA("U" + p, in, out, out); }
// summer: inverting, feedback R_sum; inputs (node, part); a constant (nepers) from the PTAT bus with the locked sign
void SUM(const std::string& p, const std::string& out, const std::vector<std::pair<std::string, std::string>>& in, const std::string& ckey = "", const std::string& cpart = "") {
  OA("S" + p, "0", "s" + p, out); Rp("Sf" + p, out, "s" + p, "R_sum"); N << "CS" << p << " " << out << " s" << p << " 100p\n";
  int i = 0; for (auto& [nd, part] : in) Rp("S" + p + "_" + std::to_string(i++), nd, "s" + p, part);
  if (!ckey.empty()) Rp("S" + p + "_c", C.at(ckey) < 0 ? "vnep" : "vnepn", "s" + p, cpart);
}
void FW(const std::string& p, const std::string& in, const std::string& out) {  // precision full-wave |in|
  OA("F1" + p, "0", "f1" + p, "fo" + p); Rp("fwi" + p, in, "f1" + p, "R_fw"); Rp("fwf" + p, "f1" + p, "fh" + p, "R_fw");
  N << "DF1" << p << " fh" << p << " fo" << p << " D1N4148\nDF2" << p << " fo" << p << " f1" << p << " D1N4148\n";
  OA("F2" + p, "0", "f2" + p, out); Rp("fwx" + p, in, "f2" + p, "R_fw2"); Rp("fwh" + p, "fh" + p, "f2" + p, "R_fw"); Rp("fwo" + p, out, "f2" + p, "R_fw2");
}
void refs() {
  const double vref = 2.5 * (1 + U(-V.vref, V.vref + 1e-15));
  if (TRAN) N << "VCC vcc 0 PWL(0 0 1m 9)\nVEE vee 0 PWL(0 0 1m -9)\nVREF vr 0 PWL(0 0 1m 0 3m " << vref << ")\n";
  else N << "VCC vcc 0 9\nVEE vee 0 -9\nVREF vr 0 DC " << vref << (OPNOISE ? " AC 1" : "") << "\n";
  OA("inv", "0", "ninv", "vrn"); Rp("inv1", "vr", "ninv", "R_inv"); Rp("inv2", "ninv", "vrn", "R_inv");
  // PTAT neper reference
  Rp("ptAh", "vr", "ptAc1", "R_ptA_hi"); Rp("ptAl", "vr", "ptAc2", "R_ptA_lo"); LOGR("ptA", "nptA");
  Rp("ptBh", "vr", "ptBc1", "R_ptB_hi"); Rp("ptBl", "vr", "ptBc2", "R_ptB_lo"); LOGR("ptB", "nptB");
  OA("ptAi", "0", "nptAi", "nptAn"); Rp("ptAi1", "nptA", "nptAi", "R_inv"); Rp("ptAi2", "nptAi", "nptAn", "R_inv");
  OA("ptg", "0", "nptg", "vnep"); Rp("ptgf", "nptg", "vnep", "R_sum"); Rp("ptgB", "nptB", "nptg", "R_ptg_B"); Rp("ptgA", "nptAn", "nptg", "R_ptg_A");
  OA("ninv", "0", "nninv", "vnepn"); Rp("ni1", "vnep", "nninv", "R_inv"); Rp("ni2", "nninv", "vnepn", "R_inv");
}
// fold ladder (Phase-1 thresholds), comparators on g, two steering copies: c2Db (LOGR_Db) and cTD (chaos TIA)
void ladder(const std::string& p, const std::string& gnode, const std::vector<std::string>& dst) {
  Rp("str5" + p, "0", "th5" + p, "R_str_5");
  for (int k = 6; k <= 24; ++k) Rp("str" + std::to_string(k) + p, "th" + std::to_string(k - 1) + p, "th" + std::to_string(k) + p, "R_str_" + std::to_string(k));
  Rp("strtop" + p, "th24" + p, "vr", "R_str_top");
  for (int k = 5; k <= 24; ++k) { std::string ks = std::to_string(k);
    CMP("cmp" + p + ks, gnode, "th" + ks + p, "ct" + p + ks);
    for (size_t j = 0; j < dst.size(); ++j) { std::string sj = p + ks + "_" + std::to_string(j);
      Rp("st" + sj, "vr", "sk" + sj, "R_step"); SPDT("l" + sj, "sk" + sj, dst[j], "0", "ct" + p + ks); } }
  for (size_t j = 0; j < dst.size(); ++j) Rp("base" + p + std::to_string(j), "vr", dst[j], "R_base");
}
void gstring(const std::string& p) { for (int i = 0; i < 20; ++i) Rp("g" + std::to_string(i) + p, i == 0 ? "0" : "tg" + std::to_string(i) + p, i == 19 ? "vr" : "tg" + std::to_string(i + 1) + p, "R_gunit"); }
struct Fold; static std::string CURF; static std::map<std::pair<std::string, std::string>, int> NLC; static double TAPX = 0; static std::vector<std::string> TAPS;   // netgen4: term argument of the current powers() call; tap log
static bool SENSE = true;
double fact(int n) { double r = 1; for (int i = 2; i <= n; ++i) r *= i; return r; }
// netgen4 (a): antilog with or without the output cascode. (b) Q4 emitter and base each pass a 0 V sense source (VSE_<tap>, VSB_<tap>) inside the
// tap's own block (a sense source on the collector would tie the downstream virtual-ground node into this block); I_c = i(vse) - i(vsb).
void CALX(const std::string& p, const std::string& vx, const std::vector<std::string>& outs, double term, const std::string& base) {
  const bool unc = term < 1e-4; char b[160]; std::snprintf(b, sizeof b, "%s\t%.6e\t%s", p.c_str(), term, unc ? "uncascoded" : "cascoded"); TAPS.push_back(b);
  Q(p + "3", p + "d", "0", p + "ea"); OA(p + "C", "0", p + "d", p + "ea"); N << "CC" << p << " " << p << "ea " << p << "d 1n\n";
  if (unc) {   // LOCK6 log-carry
    auto it = NLC.find({CURF, base}); if (it == NLC.end()) { std::fprintf(stderr, "netgen5: no taps6 row for %s %s\n", CURF.c_str(), base.c_str()); std::exit(1); }
    const std::string Rn = "R_lcn_" + std::to_string(it->second);
    OA("lu" + p, "0", "slu" + p, "vlu" + p); Rp("luf" + p, "vlu" + p, "slu" + p, "R_sum"); Rp("lux" + p, vx, "slu" + p, "R_sum"); Rp("lun" + p, "vnep", "slu" + p, Rn);
    N << "CLU" << p << " vlu" << p << " slu" << p << " 100p\n";                                                          // vlu = -vx - n V_T
    OA("li" + p, "0", "sli" + p, "vls" + p); Rp("li1" + p, "vlu" + p, "sli" + p, "R_inv"); Rp("li2" + p, "sli" + p, "vls" + p, "R_inv");   // vls = vx + n V_T
    Q(p + "s4", p + "sk4", "vls" + p, p + "ea"); MOS(p + "s4", "lt" + p, p + "sg4", p + "sk4"); OA(p + "sE", "vls" + p, p + "sk4", p + "sg4");
    TIA("lc" + p, "lt" + p, "vlc" + p, "R_lcT");                                                                          // v(vlc) = I_U t e^n R_lcT
    OA("ld" + p, "0", "sld" + p, "vld" + p); Rp("ldf" + p, "vld" + p, "sld" + p, "R_sum"); Rp("ldn" + p, "vnep", "sld" + p, Rn);
    N << "CLD" << p << " vld" << p << " sld" << p << " 100p\n";                                                          // vld = -n V_T
    Rp("ldi" + p, "vlc" + p, "dd" + p, "R_lcT"); Q(p + "d3", "dd" + p, "0", p + "dea"); OA(p + "dC", "0", "dd" + p, p + "dea"); N << "CCD" << p << " " << p << "dea dd" << p << " 1n\n";
    for (size_t i = 0; i < outs.size(); ++i) { std::string s = p + (i ? "b" : "");
      N << "VSE_" << s << " se" << s << " " << p << "dea DC 0\nVSB_" << s << " vld" << p << " sb" << s << " DC 0\n"; Q(s + "4", outs[i], "sb" + s, "se" + s); }
    return; }
  for (size_t i = 0; i < outs.size(); ++i) { std::string s = p + (i ? "b" : "");
    N << "VSE_" << s << " se" << s << " " << p << "ea DC 0\nVSB_" << s << " " << vx << " sb" << s << " DC 0\n";
    if (unc) Q(s + "4", outs[i], "sb" + s, "se" + s);
    else { Q(s + "4", s + "k4", "sb" + s, "se" + s); MOS(s + "4", outs[i], s + "g4", s + "k4"); OA(s + "E", vx, s + "k4", s + "g4"); } }
}
// powers of a log input: k-th antilog Iout = I_U x^{2k} / n_k sunk at odd -> sink node, even -> TIA copy node
void powers(const std::string& p, const std::string& x, const std::string& nlog, int kmax, const std::string& ckey, const std::string& sink, const std::string& tia) {
  for (int k = 1; k <= kmax; ++k) { std::string ks = std::to_string(k), q = p + ks + x;
    SUM(q, "vx" + q, {{nlog, "R_" + p + ks + "_n"}}, ckey + ks, "R_" + p + ks + "_c");
    const double term = std::pow(TAPX, 2 * k) / fact(ckey == "cy" ? 2 * k : 2 * k + 1);
    Rp("pin" + q, "vr", q + "d", "R_pin"); CALX(q, "vx" + q, {k % 2 ? sink : tia}, term, p + ks); }
}
struct Fold { std::string name; int g, D, look /*0:1 1:atomic 2:hep*/, hits; bool obs; double y = 0, w = 0, v = 0, T3 = 0; };
// one full-core-law channel (prefix x): writes vS<x>; optional 1-S node (n1s<x>) copies
void channel(const std::string& x, const Fold& f, bool oneminus) { CURF = f.name;
  auto ctl = [&](const std::string& nm, bool on) { N << "VCTL" << nm << x << " " << nm << x << " 0 DC " << (on ? 5 : -5) << "\n"; };
  ctl("cl1", f.look == 0); ctl("cat", f.look == 1); ctl("ch", f.hits > 0); ctl("cob", f.obs);
  gstring(x);
  // look selector: angle bus (VR) and neper bus (-V_NEP)
  Rp("lkla" + x, "0", "la" + x, "R_lkl_a"); Rp("lklh" + x, "la" + x, "lh" + x, "R_lkl_h"); Rp("lkl1" + x, "lh" + x, "l1" + x, "R_lkl_1"); Rp("lklt" + x, "l1" + x, "vr", "R_lkl_t");
  SPDT("ls1" + x, "ds1" + x, "l1" + x, "ds2" + x, "cl1" + x); SPDT("ls2" + x, "ds2" + x, "la" + x, "lh" + x, "cat" + x); BUF("dpl" + x, "ds1" + x, "dpl" + x);
  Rp("lkna" + x, "0", "na" + x, "R_lkn_a"); Rp("lknh" + x, "na" + x, "nh" + x, "R_lkn_h"); Rp("lkn1" + x, "nh" + x, "vnepn", "R_lkn_1");
  SPDT("ns1" + x, "es1" + x, "vnepn", "es2" + x, "cl1" + x); SPDT("ns2" + x, "es2" + x, "na" + x, "nh" + x, "cat" + x); BUF("dpn" + x, "es1" + x, "dpn" + x);
  Rp("dtb" + x, "0", "dtt" + x, "R_dt_b"); Rp("dtt" + x, "dtt" + x, "vr", "R_dt_t"); BUF("dtl" + x, "dtt" + x, "dtl" + x);
  if (STOP == 0) return;
  // N, P
  Rp("N" + x, "vr", "lN" + x + "c1", "R_N"); Rp("Nr" + x, "vr", "lN" + x + "c2", "R_Nr"); LOGR("lN" + x, "nN" + x);
  Rp("P" + x, "vr", "lP" + x + "c1", "R_P"); Rp("Pr" + x, "vr", "lP" + x + "c2", "R_Pr"); LOGR("lP" + x, "nP" + x);
  if (STOP == 1) return;
  // D: ladder copies -> LOGR_Db cascoded side (via 0 V sense source) and the chaos TIA
  ladder(x, f.g == 0 ? "0" : "tg" + std::to_string(f.g) + x, {"dsen" + x, "cTD" + x});
  N << "VID" << x << " dsen" << x << " lDb" << x << "c2 DC 0\n";
  Rp("Dbr" + x, "vr", "lDb" + x + "c1", "R_Dbr"); LOGR("lDb" + x, "nDb" + x);
  TIA("D" + x, "cTD" + x, "vD" + x, "R_TD");
  if (STOP == 2) return;
  // a2 = cos x = -cos y
  SUM("y" + x, "vmy" + x, {{"dpl" + x, "R_y_dp"}, {"vrn", "R_y_c"}}); FW("y" + x, "vmy" + x, "vay" + x);
  Rp("ang_y" + x, "vay" + x, "ly" + x + "c1", "R_ang"); Rp("yr" + x, "vr", "ly" + x + "c2", "R_yr"); LOGR("ly" + x, "ny" + x);
  TAPX = f.y; powers("cy", x, "ny" + x, 3, "cy", "lcos" + x + "c1", "cp" + x); TIA("cp" + x, "cp" + x, "vcp" + x, "R_pT");
  Rp("cpc" + x, "vcp" + x, "lcos" + x + "c1", "R_pT"); Rp("cosU" + x, "vr", "lcos" + x + "c1", "R_U"); Rp("cosr" + x, "vr", "lcos" + x + "c2", "R_cosr"); LOGR("lcos" + x, "ncos" + x);
  if (STOP == 3) return;
  // observer (quirkMod): exp(C_F P_var) cos(dpsi + P_var), cos(dpsi+P_var) = -sin w
  SUM("w" + x, "vmw" + x, {{"dpl" + x, "R_w_dp"}, {"vrn", "R_w_c"}}); CMP("wcmp" + x, "0", "vmw" + x, "cwp" + x); FW("w" + x, "vmw" + x, "vaw" + x);
  Rp("ang_w" + x, "vaw" + x, "lw" + x + "c1", "R_ang"); Rp("wr" + x, "vr", "lw" + x + "c2", "R_wr"); LOGR("lw" + x, "nw" + x);
  TAPX = f.w; powers("sw", x, "nw" + x, 3, "sw", "lsp" + x + "c1", "spt" + x); TIA("spt" + x, "spt" + x, "vspt" + x, "R_pT");
  Rp("spc" + x, "vspt" + x, "lsp" + x + "c1", "R_pT"); Rp("spU" + x, "vr", "lsp" + x + "c1", "R_U"); Rp("spr" + x, "vr", "lsp" + x + "c2", "R_spr"); LOGR("lsp" + x, "nsp" + x);
  SPDT("go1" + x, "go1" + x, "nw" + x, "0", "cob" + x); BUF("nwg" + x, "go1" + x, "nwg" + x);
  SPDT("go2" + x, "go2" + x, "nsp" + x, "0", "cob" + x); BUF("nspg" + x, "go2" + x, "nspg" + x);
  SPDT("go3" + x, "go3" + x, C.at("T1obs") < 0 ? "vnep" : "vnepn", "0", "cob" + x); BUF("vqg" + x, "go3" + x, "vqg" + x);
  if (STOP == 4) return;
  // h/N and alpha buses
  Rp("hNin" + x, "vnep", "hN" + x + "d", "R_hN_in"); CAL("hN" + x, "nN" + x, {"thN" + x}); TIA("hN" + x, "thN" + x, "v1N" + x, "R_hN_T");
  SPDT("hs" + x, "hs" + x, "v1N" + x, "0", "ch" + x); BUF("vhs" + x, "hs" + x, "vhs" + x);
  Rp("alt" + x, "vhs" + x, "vahN" + x, "R_al_t"); Rp("alb" + x, "vahN" + x, "0", "R_al_b");
  Rp("alnt" + x, "vnepn", "valn" + x, "R_al_t"); Rp("alnb" + x, "valn" + x, "0", "R_al_b");
  if (STOP == 5) return;
  // a4 = 1 + C_eff growth
  SUM("G" + x, "vxg" + x, {{"valn" + x, "R_G_al"}, {"vahN" + x, "R_G_ahN"}}, "G", "R_G_c");
  Rp("gin" + x, "vr", "g" + x + "d", "R_g_in"); CAL("g" + x, "vxg" + x, {"ngr" + x}); TIA("g4" + x, "ngr" + x, "vg" + x, "R_pT");
  Rp("gc" + x, "vg" + x, "la4" + x + "c1", "R_pT"); Rp("a4U" + x, "vr", "la4" + x + "c1", "R_U"); Rp("a4r" + x, "vr", "la4" + x + "c2", "R_a4r"); LOGR("la4" + x, "na4" + x);
  if (STOP == 6) return;
  // a5 = 1 + P_new ln(D/25), PTAT-referenced
  Rp("a5p" + x, "vnep", "la5" + x + "c1", "R_pt"); Rp("a5x" + x, "nDb" + x, "la5" + x + "c1", "R_x"); Rp("a5c" + x, "vnep", "la5" + x + "c1", "R_x12");
  Rp("a5r" + x, "vnep", "la5" + x + "c2", "R_a5r"); LOGR("la5" + x, "na5" + x);
  if (STOP == 7) return;
  // T1 summer -> antilog -> |T1| TIA -> sign chain -> S node
  SUM("T1" + x, "vx1" + x, {{"nN" + x, "R_T1_N"}, {"nP" + x, "R_T1_P"}, {"nDb" + x, "R_T1_Db"}, {"ncos" + x, "R_T1_cos"}, {"na4" + x, "R_T1_a4"}, {"na5" + x, "R_T1_a5"},
    {"nwg" + x, "R_T1_w"}, {"nspg" + x, "R_T1_sp"}, {"vnepn", "R_T1_rho"}, {"dpn" + x, "R_T1_dp"}, {"vahN" + x, "R_T1_ahN"}, {"vqg" + x, "R_T1_co"}}, "T1", "R_T1_c");
  Rp("T1in" + x, "vr", "a1" + x + "d", "R_T1in"); CAL("a1" + x, "vx1" + x, {"tT1" + x}); TIA("T1" + x, "tT1" + x, "vt1" + x, "R_T1T");
  OA("t1i" + x, "0", "nt1i" + x, "vt1n" + x); Rp("t1i1" + x, "vt1" + x, "nt1i" + x, "R_inv"); Rp("t1i2" + x, "nt1i" + x, "vt1n" + x, "R_inv");
  SPDT("pa" + x, "pa" + x, "vt1n" + x, "vt1" + x, "cwp" + x); SPDT("pb" + x, "pb" + x, "pa" + x, "vt1" + x, "cob" + x); BUF("t1s" + x, "pb" + x, "vt1s" + x);
  OA("SS" + x, "0", "nS" + x, "vS" + x); Rp("SF" + x, "vS" + x, "nS" + x, "R_SF"); N << "CSS" << x << " vS" << x << " nS" << x << " 100p\n";
  Rp("s1" + x, "vt1s" + x, "nS" + x, "R_s1");
  N << "VTB" << x << " vtb" << x << " 0 DC 0\n"; Rp("tb" + x, "vtb" + x, "nS" + x, "R_tb");
  if (STOP == 8) return;
  // T2 = scale*amplitude + trend_bias
  Rp("sc" + x, "vr", "lsc" + x + "c1", "R_sc"); Rp("scr" + x, "vr", "lsc" + x + "c2", "R_scr"); LOGR("lsc" + x, "nsc" + x);
  Rp("am" + x, "vr", "lam" + x + "c1", "R_am"); Rp("amr" + x, "vr", "lam" + x + "c2", "R_amr"); LOGR("lam" + x, "nam" + x);
  SUM("T2" + x, "vx2" + x, {{"nsc" + x, "R_T2_sc"}, {"nam" + x, "R_T2_am"}}, "T2", "R_T2_c");
  std::vector<std::string> o2 = {"nS" + x}; if (oneminus) o2.push_back("n1s" + x);
  Rp("T2in" + x, "vr", "a2" + x + "d", "R_T2in"); CAL("a2" + x, "vx2" + x, o2);
  if (STOP == 9) return;
  // T3 = beta cos(dpsi) NP/sqrt(D) (1 + chaos(D-25)/25) valve * acoustic * phase  (log channel)
  SUM("v" + x, "vv" + x, {{"dpl" + x, "R_v_dp"}, {"vrn", "R_v_c"}});
  Rp("ang_v" + x, "vv" + x, "lv" + x + "c1", "R_ang"); Rp("vr" + x, "vr", "lv" + x + "c2", "R_vr"); LOGR("lv" + x, "nv" + x);
  TAPX = f.v; powers("sv", x, "nv" + x, 4, "sv", "lvp" + x + "c1", "vpt" + x); TIA("vpt" + x, "vpt" + x, "vvpt" + x, "R_pT");
  Rp("vpc" + x, "vvpt" + x, "lvp" + x + "c1", "R_pT"); Rp("vpU" + x, "vr", "lvp" + x + "c1", "R_U"); Rp("vpr" + x, "vr", "lvp" + x + "c2", "R_vpr"); LOGR("lvp" + x, "nvp" + x);
  Rp("ang_t" + x, "dtl" + x, "lt" + x + "c1", "R_ang"); Rp("tr" + x, "vr", "lt" + x + "c2", "R_tr"); LOGR("lt" + x, "nt" + x);
  TAPX = 1.0; powers("st", x, "nt" + x, 5, "st", "ltp" + x + "c1", "tpt" + x); TIA("tpt" + x, "tpt" + x, "vtpt" + x, "R_pT");
  Rp("tpc" + x, "vtpt" + x, "ltp" + x + "c1", "R_pT"); Rp("tpU" + x, "vr", "ltp" + x + "c1", "R_U"); Rp("tpr" + x, "vr", "ltp" + x + "c2", "R_tpr"); LOGR("ltp" + x, "ntp" + x);
  SUM("s2" + x, "vxs2" + x, {{"nt" + x, "R_s2_n"}, {"ntp" + x, "R_s2_n"}}, "sin2", "R_s2_c");
  Rp("s2in" + x, "vr", "s2" + x + "d", "R_pin"); CAL("s2" + x, "vxs2" + x, {"lac" + x + "c1"});
  Rp("ac0" + x, "vr", "lac" + x + "c1", "R_ac0"); Rp("acr" + x, "vr", "lac" + x + "c2", "R_acr"); LOGR("lac" + x, "nac" + x);
  Rp("ch0" + x, "vr", "lch" + x + "c1", "R_ch0"); Rp("chD" + x, "vD" + x, "lch" + x + "c1", "R_chD"); Rp("chr" + x, "vr", "lch" + x + "c2", "R_chr"); LOGR("lch" + x, "nch" + x);
  Rp("v2" + x, "vr", "lv2" + x + "c1", "R_v2"); Rp("v2r" + x, "vr", "lv2" + x + "c2", "R_v2r"); LOGR("lv2" + x, "nv2" + x);
  Rp("ph" + x, "vr", "lph" + x + "c1", "R_ph"); Rp("phr" + x, "vr", "lph" + x + "c2", "R_phr"); LOGR("lph" + x, "nph" + x);
  SUM("T3" + x, "vx3" + x, {{"nv" + x, "R_T3_v"}, {"nvp" + x, "R_T3_vp"}, {"nN" + x, "R_T3_N"}, {"nP" + x, "R_T3_P"}, {"nDb" + x, "R_T3_Db"}, {"nch" + x, "R_T3_ch"},
    {"nv2" + x, "R_T3_v2"}, {"nac" + x, "R_T3_ac"}, {"nph" + x, "R_T3_ph"}}, "T3", "R_T3_c");
  std::vector<std::string> o3 = {"nS" + x}; if (oneminus) o3.push_back("n1s" + x);
  Rp("T3in" + x, "vr", "a3" + x + "d", "R_T3in"); CALX("a3" + x, "vx3" + x, o3, std::fabs(f.T3), "a3");
  if (oneminus) {  // 1 - S = K (1/K - T1 - T2 - T3) as one current node
    OA("1S" + x, "0", "n1s" + x, "v1s" + x); Rp("1sF" + x, "v1s" + x, "n1s" + x, "R_1sF"); N << "C1S" << x << " v1s" << x << " n1s" << x << " 100p\n";
    Rp("1s0" + x, "vr", "n1s" + x, "R_1s0"); Rp("1s1" + x, "vt1s" + x, "n1s" + x, "R_s1"); Rp("1stb" + x, "vtb" + x, "n1s" + x, "R_tb");
  }
}
std::vector<Fold> folds() {
  std::vector<Fold> v; std::ifstream f(LK + "/folds35_targets.tsv"); std::string line; std::getline(f, line);
  while (std::getline(f, line)) { std::istringstream s(line); std::vector<std::string> c; std::string x; while (std::getline(s, x, '\t')) c.push_back(x);
    const double lk = std::stod(c[3]); Fold fd{c[0], std::stoi(c[1]), std::stoi(c[2]), lk == 1.0 ? 0 : (lk < 0.9 ? 1 : 2), std::stoi(c[4]), c[5] == "1"};
    fd.y = std::stod(c[6]); fd.w = std::stod(c[7]); fd.v = std::stod(c[8]); fd.T3 = std::stod(c[13]); v.push_back(fd); }
  return v;
}
int main(int argc, char** argv) {
  if (argc < 5) { std::fprintf(stderr, "usage: netgen5 mode variant seed out.cir [temp] [analysis] [fold]\n"); return 1; }
  std::string mode = argv[1], var = argv[2], out = argv[4]; unsigned long seed = std::stoul(argv[3]); double temp = argc > 5 ? std::stod(argv[5]) : 27;
  std::string an = argc > 6 ? argv[6] : "op", fname = argc > 7 ? argv[7] : "Chemistry";
  if (const char* e = std::getenv("FSOT_P2_LOCKED")) LK = e;
  if (const char* e = std::getenv("FSOT_P2_STOP")) STOP = std::atoi(e);
  if (const char* e = std::getenv("FSOT_P2_OPDYN")) OPDYN = std::string(e) == "1";
  if (const char* e = std::getenv("FSOT_P2_OPNOISE")) OPNOISE = std::string(e) == "1";
  rng.seed(seed * 7919 + 101); load(); TRAN = (an == "tran");
  if (var == "ideal_ideal") { V.e24 = false; V.ideal_op = true; V.vendor_cmp = false; }
  else if (var == "ideal_opa") { V.e24 = false; }
  else if (var == "e24_opa") { }
  else if (var == "e24_ideal") { V.ideal_op = true; V.vendor_cmp = false; }
  else if (var == "mcB") { V.ideal_op = true; V.vendor_cmp = false; V.tolR = 0.001; V.tc = 25e-6; V.vos = 10e-6; V.vbe = 1e-4; V.vref = 0.001; V.beta = 0.3; V.vaf = 0.2; V.ron_lo = 75; V.ron_hi = 175; }
  else if (var == "mcC") { V.ideal_op = true; V.vendor_cmp = false; V.tolR = 0.0002; V.tc = 5e-6; V.vos = 2e-6; V.vbe = 1e-5; V.vref = 0.0005; V.beta = 0.3; V.vaf = 0.2; V.ron_lo = 75; V.ron_hi = 175; }
  else if (var == "mcX") {  // sensitivity runs: E24 + ideal op-amps + only the draws named in FSOT_P2_TOL (e.g. "tolR=0.001,vos=1e-5")
    V.ideal_op = true; V.vendor_cmp = false;
    if (const char* e = std::getenv("FSOT_P2_TOL")) { std::string t = e; std::istringstream ss(t); std::string kv;
      while (std::getline(ss, kv, ',')) { auto q = kv.find('='); std::string k = kv.substr(0, q); double x = std::stod(kv.substr(q + 1));
        if (k == "tolR") V.tolR = x; else if (k == "tc") V.tc = x; else if (k == "vos") V.vos = x; else if (k == "vbe") V.vbe = x; else if (k == "vref") V.vref = x;
        else if (k == "beta") V.beta = x; else if (k == "vaf") V.vaf = x; else if (k == "ron") { V.ron_lo = 125 - x; V.ron_hi = 125 + x; }
        else { std::fprintf(stderr, "bad FSOT_P2_TOL key %s\n", k.c_str()); return 1; } } } }
  else { std::fprintf(stderr, "bad variant\n"); return 1; }
  if (an == "dump") { V.ideal_op = true; V.vendor_cmp = false; }
  std::vector<std::string> pr;
  const auto FL = folds();
  auto findf = [&](const std::string& n) { for (auto& f : FL) if (f.name == n) return f; std::fprintf(stderr, "no fold %s\n", n.c_str()); std::exit(1); };
  std::string what;
  { std::string d6 = "../../phase3/locked6"; if (const char* e = std::getenv("FSOT_P6_LOCKED")) d6 = e;
    std::ifstream f(d6 + "/design6_taps_values.tsv"); std::string line; std::getline(f, line);
    while (std::getline(f, line)) { std::istringstream s(line); std::string n, id, a, b, c; std::getline(s, n, '\t'); std::getline(s, id, '\t'); std::getline(s, a, '\t'); std::getline(s, b, '\t'); std::getline(s, c, '\t');
      auto num = [](const std::string& z) { return z == "-" ? 0.0 : std::stod(z); }; P[n] = {std::stod(id), num(a), num(b), num(c)}; }
    std::ifstream t(d6 + "/taps6.tsv"); std::getline(t, line);
    while (std::getline(t, line)) { std::istringstream s(line); std::vector<std::string> c; std::string x; while (std::getline(s, x, '\t')) c.push_back(x); NLC[{c[0], c[1]}] = std::stoi(c[4]); } }
  refs();
  if (mode == "fold") {
    Fold f = findf(fname); what = "full-core-law channel, fold " + f.name;
    const char* cp = std::getenv("FSOT_P2_CHPREFIX");  // seed generation for multi-channel netlists only
    channel(cp ? cp : "x", f, cp && std::string(cp) == "c");
    for (auto& t : TAPS) { std::string tp = t.substr(0, t.find('\t')); pr.push_back("@" + tp); if (t.find("uncascoded") != std::string::npos) pr.push_back("vlc" + tp); }
    pr.insert(pr.begin(), {"vSx", "vt1x", "vx1x", "vx2x", "vx3x", "vnep", "vr", "dplx", "dpnx", "vmyx", "vayx", "vmwx", "cwpx", "nNx", "nPx", "nDbx", "ncosx", "nyx", "nwx", "nspx", "na4x", "na5x", "nvx", "nvpx", "ntx", "ntpx", "nacx", "nchx", "nv2x", "nphx", "nscx", "namx", "vahNx", "vxgx", "vt1sx"});
  } else if (mode == "omb") {
    what = "Chemistry + Cosmology channels, current-mode 1 - S_chem, Omega_b h^2";
    channel("c", findf("Chemistry"), true); channel("m", findf("Cosmology"), false);
    FW("om", "vSm", "vabs"); OA("o1i", "0", "no1i", "v1sp"); Rp("o1i1", "v1sc", "no1i", "R_inv"); Rp("o1i2", "no1i", "v1sp", "R_inv");
    Rp("oS", "vabs", "loSc1", "R_oS"); Rp("oSr", "vr", "loSc2", "R_oSr"); LOGR("loS", "noS");
    Rp("o1", "v1sp", "lo1c1", "R_o1"); Rp("o1r", "vr", "lo1c2", "R_o1r"); LOGR("lo1", "no1");
    SUM("Om", "vxom", {{"noS", "R_Om_n"}, {"no1", "R_Om_n"}}, "Om", "R_Om_c");
    Rp("omin", "vr", "omd", "R_omin"); CAL("om", "vxom", {"tom"}); TIA("om", "tom", "vomb", "R_omT");
    pr = {"vSc", "vSm", "v1sc", "vomb", "vabs", "vt1c", "vt1m", "vx3c", "vx3m", "vnep"};
  } else if (mode == "tapbench") {   // LOCK6 bench (transient, plain ngspice): per tiny tap of fold fname, one stimulus base V_T ln t (constant
    // from the V_NEP bus, R = R_SUM 10/|ln t|), one input transdiode (I_U), and three readouts: (a) direct uncascoded Q4 into a TIA (R_pT) with
    // emitter/base sense; (b) the LOCK6 log-carry (scaled TIA v(vlc) and divided-back current into a TIA, sensed); (c) a charge integrator:
    // uncascoded Q4 into an op-amp integrator, C_int || 100 G (C0G insulation, model assumption), from v = 0 at t = 0 for T_int.
    Fold f = findf(fname); CURF = f.name; what = "tiny-tap bench, fold " + f.name; an = "tapbench";
    std::ifstream t((std::getenv("FSOT_P6_LOCKED") ? std::string(std::getenv("FSOT_P6_LOCKED")) : std::string("../../phase3/locked6")) + "/taps6.tsv"); std::string line; std::getline(t, line);
    while (std::getline(t, line)) { std::istringstream s(line); std::vector<std::string> c; std::string x; while (std::getline(s, x, '\t')) c.push_back(x);
      if (c[0] != f.name) continue; const std::string b = c[1], p = "B" + b; const double lnt = std::stod(c[3]);
      P["R_bln_" + b] = {10e3 * 10 / std::fabs(lnt), 10e3 * 10 / std::fabs(lnt), 0, 0};
      OA("bs" + b, "0", "sbs" + b, "vb" + b); Rp("bsf" + b, "vb" + b, "sbs" + b, "R_sum"); Rp("bsn" + b, "vnep", "sbs" + b, "R_bln_" + b); N << "CBS" << b << " vb" << b << " sbs" << b << " 100p\n";
      Rp("pin" + p, "vr", p + "d", "R_pin");
      CALX(p, "vb" + b, {"ta" + b}, 0, b);
      TIA("ta" + b, "ta" + b, "vta" + b, "R_pT");
      N << "VSE_" << p << "u se" << p << "u " << p << "ea DC 0\nVSB_" << p << "u vb" << b << " sb" << p << "u DC 0\n"; Q(p + "u4", "tu" + b, "sb" + p + "u", "se" + p + "u"); TIA("tu" + b, "tu" + b, "vtu" + b, "R_pT");
      Q(p + "i4", "ii" + b, "vb" + b, p + "ea"); OA("ii" + b, "0", "ii" + b, "vii" + b); N << "CI" << b << " vii" << b << " ii" << b << " " << P.at("C_int").ideal << "\nRLK" << b << " vii" << b << " ii" << b << " 100G\n";
      pr.push_back(b); }
  } else if (mode == "taps") {   // netgen4: the tiny-tap decision table of every fold (locked in phase3/locked5/taps5.tsv)
    std::ofstream o(out); o << "fold\ttap\tterm_in_I_U\tstage\n";
    for (auto& f : FL) { TAPS.clear(); std::ostringstream keep; keep << N.str(); channel("x", f, false); N.str(keep.str()); N.seekp(0, std::ios::end);
      for (auto& t : TAPS) o << f.name << "\t" << t << "\n"; }
    return 0;
  } else if (mode == "leafc5") {  // Phase 3c: LOCK5 compiled leaf netlist (phase3/cpp/leafc5)
    std::string d5 = "../../phase3/locked5"; if (const char* e = std::getenv("FSOT_P5_LOCKED")) d5 = e;
    { std::ifstream f(d5 + "/design5_" + fname + "_values.tsv"); std::string line; std::getline(f, line);
      while (std::getline(f, line)) { std::istringstream s(line); std::string n, id, a, b, c; std::getline(s, n, '\t'); std::getline(s, id, '\t'); std::getline(s, a, '\t'); std::getline(s, b, '\t'); std::getline(s, c, '\t');
        P[n] = {std::stod(id), std::stod(a), std::stod(b), std::stod(c)}; } }
    what = "compiled leaves (LOCK5): " + fname; pr = {"vnep"};
    std::ifstream rf(d5 + "/recipe5_" + fname + ".txt"); std::string l;
    while (std::getline(rf, l)) { std::vector<std::string> c; std::istringstream t(l); std::string x; while (std::getline(t, x, '\t')) c.push_back(x);
      if (c.empty()) continue; const std::string& k = c[0];
      if (k == "CHAN") { channel("c", findf("Chemistry"), true); channel("m", findf("Cosmology"), false); channel("q", findf("Quantum_Mechanics"), true); FW("om", "vSm", "vabs"); }
      else if (k == "R") Rp(c[1], c[2], c[3], c[4]);
      else if (k == "OA") OA(c[1], c[2], c[3], c[4]);
      else if (k == "C") N << "C" << c[1] << " " << c[2] << " " << c[3] << " " << c[4] << "\n";
      else if (k == "LOGR") LOGR(c[1], c[2]);
      else if (k == "CAL") CAL(c[1], c[2], {c[3]});
      else if (k == "CALU") { const std::string& q = c[1];
        Q(q + "3", q + "d", "0", q + "ea"); OA(q + "C", "0", q + "d", q + "ea"); N << "CC" << q << " " << q << "ea " << q << "d 1n\n"; Q(q + "4", c[3], c[2], q + "ea"); }
      else if (k == "FWR") { const std::string& p = c[1], &in = c[2], &o = c[3], &ra = c[4], &rb = c[5];   // FW() with the compiler's own part names
        OA("F1" + p, "0", "f1" + p, "fo" + p); Rp("fwi" + p, in, "f1" + p, ra); Rp("fwf" + p, "f1" + p, "fh" + p, ra);
        // LOCK6: the two rectifier diodes are diode-connected 2N3904s (I_S 6.7 fA) instead of 1N4148 (I_S 2.52 nA): the 1N4148 reverse current is
        // comparable to the 2.6 nA signal current of Chain_consistency_% (LOCK5 ideal: |x| read as 2.65 uV for a 261 uV input).
        Q("DF1" + p, "fh" + p, "fh" + p, "fo" + p); Q("DF2" + p, "fo" + p, "fo" + p, "f1" + p);
        OA("F2" + p, "0", "f2" + p, o); Rp("fwx" + p, in, "f2" + p, rb); Rp("fwh" + p, "fh" + p, "f2" + p, ra); Rp("fwo" + p, o, "f2" + p, rb); }
      else if (k == "CMP") CMP(c[1], c[2], c[3], c[4]);
      else if (k == "PRINT") pr.push_back(c[1]);
      else { std::fprintf(stderr, "netgen4 leafc5: bad recipe line %s\n", l.c_str()); return 1; } }
  } else if (mode == "leafc") {  // Phase 3b: a compiled leaf netlist (phase3/cpp/leafc): recipe of primitives + locked values, netlist key = [fold] arg
    std::string d4 = "../../phase3/locked4"; if (const char* e = std::getenv("FSOT_P4_LOCKED")) d4 = e;
    { std::ifstream f(d4 + "/design4_" + fname + "_values.tsv"); std::string line; std::getline(f, line);
      while (std::getline(f, line)) { std::istringstream s(line); std::string n, id, a, b, c; std::getline(s, n, '\t'); std::getline(s, id, '\t'); std::getline(s, a, '\t'); std::getline(s, b, '\t'); std::getline(s, c, '\t');
        P[n] = {std::stod(id), std::stod(a), std::stod(b), std::stod(c)}; } }
    what = "compiled leaves: " + fname; pr = {"vnep"};
    std::ifstream rf(d4 + "/recipe4_" + fname + ".txt"); std::string l;
    while (std::getline(rf, l)) { std::vector<std::string> c; std::istringstream t(l); std::string x; while (std::getline(t, x, '\t')) c.push_back(x);
      if (c.empty()) continue; const std::string& k = c[0];
      if (k == "CHAN") { channel("c", findf("Chemistry"), true); channel("m", findf("Cosmology"), false); channel("q", findf("Quantum_Mechanics"), true); FW("om", "vSm", "vabs"); }
      else if (k == "R") Rp(c[1], c[2], c[3], c[4]);
      else if (k == "OA") OA(c[1], c[2], c[3], c[4]);
      else if (k == "C") N << "C" << c[1] << " " << c[2] << " " << c[3] << " " << c[4] << "\n";
      else if (k == "LOGR") LOGR(c[1], c[2]);
      else if (k == "CAL") CAL(c[1], c[2], {c[3]});
      else if (k == "CALU") { const std::string& q = c[1];   // antilog without the output cascode (tiny series terms)
        Q(q + "3", q + "d", "0", q + "ea"); OA(q + "C", "0", q + "d", q + "ea"); N << "CC" << q << " " << q << "ea " << q << "d 1n\n"; Q(q + "4", c[3], c[2], q + "ea"); }
      else if (k == "CMP") CMP(c[1], c[2], c[3], c[4]);
      else if (k == "PRINT") pr.push_back(c[1]);
      else { std::fprintf(stderr, "netgen2 leafc: bad recipe line %s\n", l.c_str()); return 1; } }
  } else if (mode == "leaf1") {  // Phase 3 leaf tier 1: the closed forms fed by S_cosm, S_quant, S_chem, solved by the circuit
    std::string d3 = "../../phase3/locked"; if (const char* e = std::getenv("FSOT_P3_LOCKED")) d3 = e; load3(d3);
    what = "leaf tier 1 (S_cosm, S_quant, S_chem channels + 14 closed forms)";
    channel("c", findf("Chemistry"), true); channel("m", findf("Cosmology"), false); channel("q", findf("Quantum_Mechanics"), true);
    FW("om", "vSm", "vabs"); OA("o1i", "0", "no1i", "v1sp"); Rp("o1i1", "v1sc", "no1i", "R_inv"); Rp("o1i2", "no1i", "v1sp", "R_inv");
    Rp("oS", "vabs", "loSc1", "R_oS"); Rp("oSr", "vr", "loSc2", "R_oSr"); LOGR("loS", "noS");
    Rp("o1", "v1sp", "lo1c1", "R_o1"); Rp("o1r", "vr", "lo1c2", "R_o1r"); LOGR("lo1", "no1");
    SUM("Om", "vxom", {{"noS", "R_Om_n"}, {"no1", "R_Om_n"}}, "Om", "R_Om_c");
    Rp("omin", "vr", "omd", "R_omin"); CAL("om", "vxom", {"tom"}); TIA("om", "tom", "vomb", "R_omT");
    // |S_cosm| * S_quant (sigma_8): same topology as the Omega_b h^2 product
    // (prefixes "zg..." chosen so no name collides case-insensitively with the channel's lP<x>, P<x>, ... elements)
    Rp("zgS", "vabs", "lzgSc1", "R_pS"); Rp("zgSr", "vr", "lzgSc2", "R_pSr"); LOGR("lzgS", "nzgS");
    Rp("zgQ", "vSq", "lzgQc1", "R_pq"); Rp("zgQr", "vr", "lzgQc2", "R_pqr"); LOGR("lzgQ", "nzgQ");
    SUM("zg", "vxzg", {{"nzgS", "R_p_n"}, {"nzgQ", "R_p_n"}}, "P", "R_p_c");
    Rp("zgin", "vr", "zgd", "R_pin"); CAL("zg", "vxzg", {"tzg"}); TIA("zg", "tzg", "vprod", "R_pT");
    // linear leaves: one inverting summer each, V = G (b + k x)
    std::ifstream lt(d3 + "/leaves3_targets.tsv"); std::string l; std::getline(lt, l);
    pr = {"vSc", "vSm", "vSq", "v1sc", "v1sq", "vabs", "vprod", "vomb", "vnep"};
    while (std::getline(lt, l)) { std::istringstream t(l); std::vector<std::string> c; std::string x; while (std::getline(t, x, '\t')) c.push_back(x);
      if (c[0] == "Omb") continue; const std::string id = c[0], node = c[6], bus = c[9];
      OA("YL" + id, "0", "sYL" + id, "vL" + id); Rp("YLf" + id, "vL" + id, "sYL" + id, "R_Lf"); N << "CYL" << id << " vL" << id << " sYL" << id << " 100p\n";
      Rp("YLx" + id, node, "sYL" + id, "R_L" + id + "_x"); if (bus != "none") Rp("YLc" + id, bus, "sYL" + id, "R_L" + id + "_c");
      pr.push_back("vL" + id); }
  } else if (mode == "consts") {
    what = "PTAT reference, alpha_s = e^-1/pi, 1/alpha_em = e^3(3phi+2) - 1 + e^-1 with e^3 from three e^1 stages";
    SUM("em1", "vxm1", {}, "em1", "R_em1_c");
    Rp("asin", "vr", "asd", "R_as_in"); CAL("as", "vxm1", {"tas"}); TIA("as", "tas", "vas", "R_asT");
    SUM("e1", "vxe1", {}, "e1", "R_e1_c");  // one shared (1 - ln4) exponent line per stage below (own summer each)
    SUM("e2", "vxe2", {}, "e1", "R_e1_c"); SUM("e3", "vxe3", {}, "e1", "R_e1_c");
    Rp("e1in", "vr", "ec1d", "R_e1in"); CAL("ec1", "vxe1", {"te1"}); TIA("e1", "te1", "ve1", "R_eT");
    Rp("e2in", "ve1", "ec2d", "R_eT"); CAL("ec2", "vxe2", {"te2"}); TIA("e2", "te2", "ve2", "R_eT");
    Rp("e3in", "ve2", "ec3d", "R_e3c"); CAL("ec3", "vxe3", {"tai"});
    Rp("em1", "vr", "tai", "R_em1"); Rp("eiin", "vr", "eid", "R_eiin"); CAL("ei", "vxm1", {"tai"}); TIA("ai", "tai", "vainv", "R_aiT");
    pr = {"vnep", "vnepn", "nptA", "nptB", "vas", "ve1", "ve2", "vainv", "vxe1", "vxm1", "vr"};
  } else if (mode == "ptat") {
    what = "PTAT neper reference only"; pr = {"vnep", "vnepn", "nptA", "nptB", "vr"};
  } else { std::fprintf(stderr, "bad mode\n"); return 1; }
  std::ofstream o(out);
  o << "* FSOT Omni-Solver Phase 3d (netgen5) — " << what << " (values from phase2/locked/design2_values.tsv)\n";
  o << "* variant " << var << " seed " << seed << " temp " << temp << " analysis " << an << "\n";
  if (!V.ideal_op) o << ".include ../models/vendor/ti_sbomab6/OPAx187.LIB\n";
  if (V.vendor_cmp) o << ".include ../models/vendor/ti_slcj002/LM139.5_1\n";
  o << ".model D1N4148 D(Is=2.52n Rs=0.568 N=1.752 Cjo=4p M=0.4 tt=20n BV=100)\n";
  o << ".model M2N7000 VDMOS(Rg=3 Vto=1.6 Rd=0 Rs=.75 Rb=3.5 Kp=.15 Cgdmax=67p Cgdmin=12p Cgs=50p Cjo=50p Is=5.6p Vds=60 Ron=1.8 Qg=1.8n)\n";
  o << M.str() << N.str();
  if (const char* ns = std::getenv("FSOT_P2_NODESET")) {  // solver aid only: initial guess from a converged solve
    std::ifstream nf(ns); std::string l; std::string body = N.str(); for (auto& ch : body) ch = (char)std::tolower((unsigned char)ch);
    while (std::getline(nf, l)) { auto eq = l.find(" = "); if (eq == std::string::npos) continue; std::string nd = l.substr(0, eq);
      while (!nd.empty() && nd.back() == ' ') nd.pop_back();
      if (nd.find('#') != std::string::npos || nd.find('(') != std::string::npos) continue;
      if (body.find(" " + nd + " ") == std::string::npos && body.find(" " + nd + "\n") == std::string::npos) continue;
      o << ".nodeset v(" << nd << ")=" << l.substr(eq + 3) << "\n"; }
  }
  if (an == "tapbench") { o << ".ic"; for (auto& b : pr) o << " v(vii" << b << ")=0"; o << "\n"; }
  o << ".options temp=" << temp << " tnom=27 reltol=1e-6 abstol=1e-14 vntol=1e-8 gmin=1e-15 itl1=5000 gminsteps=50 srcsteps=50\n.control\n";
  if (an == "tapbench") {
    const double Ti = P.at("T_int").ideal;
    o << "tran " << Ti / 2000 << " " << Ti << "\nlet n = length(time) - 1\n";
    for (auto& b : pr) { std::string p = "B" + b;
      o << "let x_" << b << " = (i(vse_" << p << ") - i(vsb_" << p << "))[n]\nlet u_" << b << " = (i(vse_" << p << "u) - i(vsb_" << p << "u))[n]\nlet c_" << b << " = v(vlc" << p << ")[n]\nlet q_" << b << " = v(vii" << b << ")[n]\n"
        << "let h_" << b << " = v(vii" << b << ")[n/2]\nprint x_" << b << " u_" << b << " c_" << b << " q_" << b << " h_" << b << "\n"; } }
  else if (an == "dump" || an == "opdump") o << "op\nprint all\n";
  else if (an == "tran") {
    o << "tran 20u 30m 0 20u\nlet n = length(time) - 1\nlet k = n - 250\n";
    for (auto& x : pr) o << "let fin_" << x << " = v(" << x << ")[n]\nlet rip_" << x << " = vecmax(v(" << x << ")[k,n]) - vecmin(v(" << x << ")[k,n])\nprint fin_" << x << " rip_" << x << "\n";
  } else if (an == "noise") {
    o << "noise v(" << pr[0] << ") VREF dec 20 0.1 10 1\nsetplot noise2\nprint onoise_total\nsetplot noise1\nprint onoise_spectrum[0]\n"
      << "noise v(" << pr[0] << ") VREF dec 20 0.1 10k 1\nsetplot noise4\nprint onoise_total\nsetplot noise3\nprint onoise_spectrum[60]\n";
  } else {
    o << "op\nprint"; for (auto& x : pr) if (x[0] != '@') o << " v(" << x << ")"; o << "\n";
    { std::string ip; for (auto& x : pr) if (x[0] == '@') ip += " i(vse_" + x.substr(1) + ") i(vsb_" + x.substr(1) + ")"; if (!ip.empty()) o << "print" << ip << "\n"; }
    if (mode == "fold") o << "print i(vidx)\n";
  }
  o << "quit\n.endc\n.end\n";
  return 0;
}
