// score.cpp — parse ngspice slice logs, convert node voltages to FSOT quantities via the locked volts-per-unit
// scales, compare with the locked C++ engine targets. Usage: score <log>... (prints one TSV row per log)
#include <cmath>
#include <cstdio>
#include <fstream>
#include <map>
#include <regex>
#include <sstream>
#include <string>
int main(int argc, char** argv) {
  std::map<std::string, double> T; std::ifstream t("../locked/fsot_targets.tsv"); std::string line;
  while (std::getline(t, line)) { if (line.empty() || line[0] == '#' || line.rfind("name", 0) == 0) continue; std::istringstream s(line); std::string n, v; std::getline(s, n, '\t'); std::getline(s, v, '\t'); T[n] = std::stod(v); }
  struct Q { const char* name; const char* node; double scale; const char* target; };
  const Q qs[] = {{"S_quant", "v(vsq)", 2.0, "S_quant"}, {"S_chem", "v(vsc)", 2.0, "S_chem"}, {"S_cosm", "v(vsm)", 2.0, "S_cosm"},
                  {"f_D5", "v(vfq)", 1.0, "f_D5"}, {"f_D6", "v(vfc)", 1.0, "f_D6"}, {"f_D25", "v(vfm)", 1.0, "f_D25"},
                  {"m_H/m_W", "v(vmhw)", 2.0, "m_H/m_W"}, {"Omega_b_h2", "v(vomb)", 100.0, "Omega_b_h2"},
                  {"alpha_s(M_Z)", "v(vas)", 10.0, "alpha_s(M_Z)"}, {"1/alpha_em", "v(vainv)", 0.04, "1/alpha_em"}};
  std::printf("log");
  for (auto& q : qs) std::printf("\t%s", q.name);
  std::printf("\tD_q\tD_c\tD_m\tripple_max_V\n");
  const std::regex re(R"(([vi]\([^)]+\))\s*=\s*([-+0-9.eE]+))");
  for (int a = 1; a < argc; ++a) {
    std::ifstream f(argv[a]); std::stringstream ss; ss << f.rdbuf(); std::string s = ss.str(); std::map<std::string, double> v;
    for (std::sregex_iterator it(s.begin(), s.end(), re), e; it != e; ++it) v[(*it)[1]] = std::stod((*it)[2]);
    const std::regex rf(R"(fin_(i?)(\w+)\s*=\s*([-+0-9.eE]+))"), rr(R"(rip_(\w+)\s*=\s*([-+0-9.eE]+))");
    for (std::sregex_iterator it(s.begin(), s.end(), rf), e; it != e; ++it) v[std::string((*it)[1] == "i" ? "i(" : "v(") + std::string((*it)[2]) + ")"] = std::stod((*it)[3]);
    double ripmax = 0; for (std::sregex_iterator it(s.begin(), s.end(), rr), e; it != e; ++it) ripmax = std::max(ripmax, std::stod((*it)[2]));
    v["ripmax"] = ripmax;
    std::printf("%s", argv[a]);
    bool ok = v.count("v(vsq)");
    for (auto& q : qs) std::printf("\t%.9g", ok ? v[q.node] / q.scale : NAN);
    for (auto p : {"q", "c", "m"}) std::printf("\t%.6g", ok ? 25 * v[std::string("i(vid") + p + ")"] / v[std::string("i(vi25") + p + ")"] : NAN);
    std::printf("\t%.3g\n", v["ripmax"]);
  }
  std::printf("FSOT");
  for (auto& q : qs) std::printf("\t%.12g", T[q.target]);
  std::printf("\t5\t6\t25\n");
}
