// score2.cpp — convert Phase-2 ngspice logs to FSOT quantities with the LOCKED scales and compare with the
// locked C++ targets. Every node is compared with its expected value at the simulated temperature (V_T(T) = kT/q).
// Usage: score2 fold <fold> <tempC> <log>   |   score2 omb <tempC> <log>   |   score2 consts <tempC> <log>
#include <cmath>
#include <cstdio>
#include <fstream>
#include <map>
#include <regex>
#include <sstream>
#include <string>
static std::map<std::string, double> T, Cn;
static std::map<std::string, std::map<std::string, double>> FT;
std::map<std::string, double> parse(const char* f) {
  std::ifstream i(f); std::stringstream ss; ss << i.rdbuf(); std::string s = ss.str(); std::map<std::string, double> v;
  const std::regex re(R"((?:v|i)\(([a-z0-9_]+)\)\s*=\s*([-+0-9.eE]+))"), rf(R"(fin_([a-z0-9_]+)\s*=\s*([-+0-9.eE]+))"), rr(R"(rip_([a-z0-9_]+)\s*=\s*([-+0-9.eE]+))");
  for (std::sregex_iterator it(s.begin(), s.end(), re), e; it != e; ++it) v[(*it)[1]] = std::stod((*it)[2]);
  for (std::sregex_iterator it(s.begin(), s.end(), rf), e; it != e; ++it) v[(*it)[1]] = std::stod((*it)[2]);
  double rip = 0; for (std::sregex_iterator it(s.begin(), s.end(), rr), e; it != e; ++it) rip = std::max(rip, std::fabs(std::stod((*it)[2])));
  v["__ripple"] = rip; return v;
}
void load(const std::string& LK) {
  std::string l; std::ifstream t(LK + "/targets2.tsv"); std::getline(t, l);
  while (std::getline(t, l)) { std::istringstream s(l); std::string n, v; std::getline(s, n, '\t'); std::getline(s, v, '\t'); T[n] = std::stod(v); }
  std::ifstream f(LK + "/folds35_targets.tsv"); std::getline(f, l); std::istringstream hs(l); std::vector<std::string> h; std::string x; while (std::getline(hs, x, '\t')) h.push_back(x);
  while (std::getline(f, l)) { std::istringstream s(l); std::vector<std::string> c; while (std::getline(s, x, '\t')) c.push_back(x); for (size_t k = 1; k < c.size(); ++k) FT[c[0]][h[k]] = std::stod(c[k]); }
}
int main(int argc, char** argv) {
  if (argc < 4) return 1;
  std::string LK = std::getenv("FSOT_P2_LOCKED") ? std::getenv("FSOT_P2_LOCKED") : "../locked"; load(LK);
  const std::string mode = argv[1];
  const double kB = 1.380649e-23, q = 1.602176634e-19;
  if (mode == "fold") {
    const std::string fn = argv[2]; const double tc = std::stod(argv[3]); auto v = parse(argv[4]); auto& t = FT.at(fn);
    const double VT = kB * (tc + 273.15) / q; bool ok = v.count("vsx");
    const double S = ok ? v["vsx"] / 2.0 : NAN, T1m = ok ? v["vt1x"] : NAN;
    const double T1 = ok ? (v["vt1sx"] < 0 ? T1m : -T1m) : NAN;  // signed: V_S gets -R_SF*vt1s/R_s1
    const double lnT3 = ok ? v["vx3x"] / VT : NAN;
    const double D = ok ? v["vidx"] / 0.2e-6 : NAN;
    std::printf("%s\t%.1f\t%.9f\t%.9f\t%.3e\t%.9f\t%.9f\t%.3e\t%.6f\t%.6f\t%.3e\t%.4f\t%.4f\t%.6f\t%.6f\t%.3g\n", fn.c_str(), tc, S, t["S"], (S - t["S"]) / std::fabs(t["S"]), T1, t["T1"], (T1 - t["T1"]) / std::fabs(t["T1"]),
      lnT3, t["lnT3"], lnT3 - t["lnT3"], D, t["D"], ok ? v["vnep"] / (10 * VT) - 1 : NAN, ok ? v["dplx"] : NAN, v["__ripple"]);
  } else if (mode == "omb") {
    const double tc = std::stod(argv[2]); auto v = parse(argv[3]); bool ok = v.count("vomb");
    const double Sc = v["vsc"] / 2, Sm = v["vsm"] / 2, om1 = -v["v1sc"] / 40, Om = v["vomb"] / 100;
    std::printf("%.1f\t%.9f\t%.3e\t%.9f\t%.3e\t%.9f\t%.3e\t%.9f\t%.3e\t%.3g\n", tc, ok ? Sc : NAN, (Sc - T["S_chem"]) / T["S_chem"], ok ? Sm : NAN, (Sm - T["S_cosm"]) / std::fabs(T["S_cosm"]),
      ok ? om1 : NAN, (om1 - T["one_minus_S_chem"]) / T["one_minus_S_chem"], ok ? Om : NAN, (Om - T["Omega_b_h2"]) / T["Omega_b_h2"], v["__ripple"]);
  } else if (mode == "consts") {
    const double tc = std::stod(argv[2]); auto v = parse(argv[3]); const double VT = kB * (tc + 273.15) / q;
    const double as = v["vas"] / 10, ai = v["vainv"] / 0.04;
    std::printf("%.1f\t%.9f\t%.3e\t%.9f\t%.3e\t%.9f\t%.3e\t%.3g\n", tc, v["vnep"], v["vnep"] / (10 * VT) - 1, as, (as - T["alpha_s(M_Z)"]) / T["alpha_s(M_Z)"], ai, (ai - T["1/alpha_em"]) / T["1/alpha_em"], v["__ripple"]);
  }
}
