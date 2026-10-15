// leafmap.cpp — map of the FSOT closed forms ("leaves") from the pinned engine's generated table
// (FSOT-2.1-Cpp 3466e8d include/fsot/closed_forms_core.gen.inc): section, name, formula, S inputs, constants used, analog
// operations needed, the analog block, and a dependency tier. Output: TSV on stdout. Usage: leafmap <closed_forms_core.gen.inc>
#include <cstdio>
#include <fstream>
#include <map>
#include <regex>
#include <set>
#include <sstream>
#include <string>
#include <vector>
static std::string unesc(const std::string& s) {  // \uXXXX -> UTF-8
  std::string o; for (size_t i = 0; i < s.size(); ++i) {
    if (s[i] == '\\' && i + 5 < s.size() && s[i + 1] == 'u') { unsigned c = std::stoul(s.substr(i + 2, 4), nullptr, 16); i += 5;
      if (c < 0x80) o += char(c); else if (c < 0x800) { o += char(0xC0 | (c >> 6)); o += char(0x80 | (c & 0x3F)); }
      else { o += char(0xE0 | (c >> 12)); o += char(0x80 | ((c >> 6) & 0x3F)); o += char(0x80 | (c & 0x3F)); } }
    else o += s[i]; }
  return o; }
static std::vector<std::string> args(const std::string& s, size_t open) {  // top-level comma split of the call starting at '('
  std::vector<std::string> a; int d = 0; bool q = false; std::string cur;
  for (size_t i = open; i < s.size(); ++i) { char c = s[i];
    if (c == '"' && s[i - 1] != '\\') q = !q;
    if (!q) { if (c == '(') { if (d++ == 0) continue; } else if (c == ')') { if (--d == 0) { a.push_back(cur); return a; } } else if (c == ',' && d == 1) { a.push_back(cur); cur.clear(); continue; } }
    cur += c; }
  return a; }
static std::string trim(std::string s) { while (!s.empty() && isspace((unsigned char)s.front())) s.erase(0, 1); while (!s.empty() && isspace((unsigned char)s.back())) s.pop_back(); return s; }
int main(int argc, char** argv) {
  std::ifstream f(argv[1]); std::stringstream ss; ss << f.rdbuf(); const std::string src = ss.str();
  const std::set<std::string> S3 = {"S_COSM", "S_QUANT", "S_CHEM"};
  const std::set<std::string> BASE = {"PI", "E", "PHI", "GAMMA", "G_CAT"};
  const std::set<std::string> DER = {"ALPHA", "PSI_CON", "ETA_EFF", "BETA", "GAMMA_C", "OMEGA", "THETA_S", "POOF", "C_EFF", "A_BLEED", "P_VAR", "B_IN", "A_IN", "SUCTION", "CHAOS", "P_BASE", "P_NEW", "C_FACTOR", "K", "C_COSM"};
  std::printf("section\tname\tformula\tS_inputs\tconstants\toperations\tanalog_block\ttier\tcexpr\n");
  std::regex fn(R"(template <class S> void ([a-z_0-9]+)\(S& out\) const \{)"); std::sregex_iterator it(src.begin(), src.end(), fn), en;
  std::vector<std::pair<std::string, size_t>> secs; for (; it != en; ++it) secs.push_back({(*it)[1], (size_t)it->position()});
  for (size_t k = 0; k < secs.size(); ++k) { const std::string sec = secs[k].first; if (sec == "all_sections") continue;
    const size_t a = secs[k].second, b = k + 1 < secs.size() ? secs[k + 1].second : src.size(); const std::string body = src.substr(a, b - a);
    std::map<std::string, std::string> loc; std::string vexpr;
    // walk statements in order: assignments and cf( calls
    size_t p = 0; std::regex asg(R"((?:const\s+)?(?:R\s+)?([A-Za-z_][A-Za-z_0-9]*)\s*=\s*)");
    while (p < body.size()) {
      size_t nc = body.find("cf(_sec,", p), ns = body.find(';', p); if (ns == std::string::npos) break;
      if (nc != std::string::npos && nc < ns) {
        auto ar = args(body, nc + 2); if (ar.size() < 4) { p = ns + 1; continue; }
        std::string name = unesc(trim(ar[1])), form = unesc(trim(ar[2])), ex = trim(ar[3]);
        if (name.size() > 1 && name.front() == '"') name = name.substr(1, name.size() - 2); if (form.size() > 1 && form.front() == '"') form = form.substr(1, form.size() - 2);
        // expand locals (two passes)
        for (int pass = 0; pass < 3; ++pass) for (auto& [l, e] : loc) ex = std::regex_replace(ex, std::regex("\\b" + l + "\\b"), "(" + e + ")");
        std::set<std::string> si, cs, ops; std::smatch m; std::string t = ex;
        std::regex id(R"([A-Za-z_][A-Za-z_0-9]*)"); for (std::sregex_iterator i2(t.begin(), t.end(), id), e2; i2 != e2; ++i2) { std::string w = (*i2).str();
          if (S3.count(w)) si.insert(w); else if (BASE.count(w) || DER.count(w)) cs.insert(w);
          else if (w == "exp" || w == "log" || w == "sqrt" || w == "sin" || w == "cos" || w == "tan" || w == "acos" || w == "asin" || w == "atan" || w == "atan2" || w == "fabs" || w == "pow" || w == "ipow" || w == "tanh" || w == "floor" || w == "round") ops.insert(w); }
        if (ex.find('*') != std::string::npos) ops.insert("mul"); if (ex.find('/') != std::string::npos) ops.insert("div");
        if (std::regex_search(ex, std::regex(R"([^eE(,\s]\s*[-+]\s*[^\s])"))) ops.insert("add");
        if (ex.find("T::lit") != std::string::npos && si.empty() && cs.empty()) ops.insert("literal");
        auto has = [&](const char* x) { return ops.count(x) > 0; };
        std::string blk;
        if (form == "exact" || (si.empty() && cs.empty() && !has("sin") && !has("cos") && !has("acos"))) blk = "resistor ratio / literal (no computation)";
        else { std::vector<std::string> bl;
          if (has("mul") || has("div") || has("pow") || has("ipow") || has("sqrt")) bl.push_back("LOGR+SUM+CAL (log-domain product/power)");
          if (has("add")) bl.push_back("current-mode summing TIA");
          if (has("exp")) bl.push_back("CAL (antilog of a computed exponent)");
          if (has("log")) bl.push_back("LOGR (log of a computed quantity)");
          if (has("sin") || has("cos") || has("tan")) bl.push_back("trig series (powers block)");
          if (has("acos") || has("asin") || has("atan") || has("atan2")) bl.push_back("inverse trig: servo around trig series (new block)");
          if (has("fabs")) bl.push_back("precision full-wave");
          if (has("floor") || has("round")) bl.push_back("comparator ladder (quantiser)");
          if (bl.empty()) bl.push_back("buffer of a generated constant");
          for (size_t i = 0; i < bl.size(); ++i) blk += (i ? " + " : "") + bl[i]; }
        std::string tier = !si.empty() ? "1 (fed by S_cosm/S_quant/S_chem)" : (!cs.empty() ? (std::any_of(cs.begin(), cs.end(), [&](const std::string& c) { return DER.count(c) > 0; }) ? "2 (derived constants only)" : "2a (base constants only)") : "0 (literal)");
        auto join = [](const std::set<std::string>& s) { std::string o; for (auto& x : s) o += (o.empty() ? "" : ",") + x; return o.empty() ? std::string("-") : o; };
        std::string cx = ex; for (auto& ch : cx) if (ch == '\t' || ch == '\n' || ch == '\r') ch = ' ';  // expanded C++ expression (column 9)
        std::printf("%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\n", sec.c_str(), name.c_str(), form.c_str(), join(si).c_str(), join(cs).c_str(), join(ops).c_str(), blk.c_str(), tier.c_str(), cx.c_str());
        p = ns + 1; continue; }
      std::string st = body.substr(p, ns - p); std::smatch m;
      std::string sst = trim(st); while (!sst.empty() && (sst[0] == '}' || sst[0] == '{')) sst = trim(sst.substr(1));  // statement after a closing loop block
      if (std::regex_search(sst, m, asg) && m.position() == 0 && sst.find("cf(") == std::string::npos && sst.find("for (") == std::string::npos) {
        std::string nm = m[1], ex = sst.substr(m.length()); if (nm != "_sec") { for (int pass = 0; pass < 2; ++pass) for (auto& [l, e] : loc) if (l != nm) ex = std::regex_replace(ex, std::regex("\\b" + l + "\\b"), "(" + e + ")"); loc[nm] = ex; } }
      p = ns + 1; } }
  return 0; }
