// blocksolve4.cpp — FSOT Omni-Solver Phase 3b fork of phase2/cpp/blocksolve.cpp (LOCK2 Amendment 14, unchanged). Changes: RE-ANCHOR in verify (Amendment 4.3); SEED 4th attempt (Amendment 4.4).
// blocksolve.cpp — exact block-sequential DC solve of a netgen2 netlist in ngspice (solver harness; no value or topology change).
// Every ideal op-amp / comparator output (E or B source to ground) and every grounded supply is a stiff node: its voltage depends
// only on that driver's inputs, never on the load it feeds. Tearing the netlist at those nodes therefore changes nothing in the
// solution: each block (the elements galvanically joined between stiff nodes, plus the drivers they control) is solved in
// ngspice on its own, with the stiff nodes it reads from other blocks held by DC sources at their already-solved values. Blocks
// that read each other (strongly connected) are merged and solved together; blocks are solved in dependency order.
// Vendor macro-model op-amps (X ... OPAx187) are handled the same way at their output pin (closed-loop output impedance ~mOhm:
// the tearing error is reported by the caller, not hidden).
// Usage: blocksolve <in.cir> <workdir> <out.log>   (out.log: the netlist's print vectors as "v(n) = x", then "SOLVER: blocks ...")
#include <algorithm>
#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <fstream>
#include <functional>
#include <map>
#include <regex>
#include <set>
#include <sstream>
#include <string>
#include <vector>
static std::string lc(std::string s) { for (auto& c : s) c = (char)std::tolower((unsigned char)c); return s; }
struct El { std::string line, name; std::vector<std::string> term, sense; std::string drv; bool global = false; };
static std::map<std::string, std::string> par;
static std::string F(const std::string& x) { auto it = par.find(x); if (it == par.end()) return par[x] = x; if (it->second == x) return x; return it->second = F(it->second); }
static void Un(const std::string& a, const std::string& b) { std::string x = F(a), y = F(b); if (x != y) par[x] = y; }
int main(int argc, char** argv) {
  if (argc < 4) { std::fprintf(stderr, "usage: blocksolve in.cir workdir out.log\n"); return 1; }
  std::ifstream in(argv[1]); std::string wd = argv[2], outlog = argv[3]; std::system(("mkdir -p " + wd).c_str());
  std::vector<std::string> head, models, prints; std::vector<El> els; std::map<std::string, std::string> nodeset; std::string options;
  std::string l; bool ctl = false; int ln = 0; double rail = 0; if (const char* r = std::getenv("FSOT_BS_RAIL")) rail = std::atof(r);
  while (std::getline(in, l)) { ++ln; if (l.empty()) continue; std::string L = lc(l);
    if (ctl) { if (L.rfind("print ", 0) == 0) prints.push_back(L.substr(6)); if (L == ".endc") ctl = false; continue; }
    if (L == ".control") { ctl = true; continue; }
    if (ln <= 2 && l[0] == '*') { head.push_back(l); continue; }
    if (l[0] == '*') continue;
    if (L.rfind(".model", 0) == 0 || L.rfind(".include", 0) == 0 || L.rfind(".lib", 0) == 0) { models.push_back(l); continue; }
    if (L.rfind(".nodeset", 0) == 0) { std::smatch m; static const std::regex r(R"(v\(([^)]+)\)=(\S+))"); if (std::regex_search(L, m, r)) nodeset[m[1]] = m[2]; continue; }
    if (L.rfind(".options", 0) == 0) { options = l; continue; }
    if (L[0] == '.') continue;
    std::istringstream s(L); std::vector<std::string> t; std::string w; while (s >> w) t.push_back(w);
    if (rail > 0 && t[0].rfind("e_", 0) == 0 && t.size() == 6) {  // rail-limited ideal op-amp: same DC inside the rails
      l = t[0] + " " + t[0] + "_i 0 " + t[3] + " " + t[4] + " " + t[5] + "\nB" + t[0].substr(1) + "_rl " + t[1] + " 0 V = " + std::to_string(rail) + "*tanh(V(" + t[0] + "_i)/" + std::to_string(rail) + ")";
      El e; e.line = l; e.name = t[0]; e.drv = t[1]; e.sense = {t[3], t[4]}; els.push_back(e); continue; }
    El e; e.line = l; e.name = t[0]; char c = t[0][0];
    if (c == 'r' || c == 'c' || c == 'd') e.term = {t[1], t[2]};
    else if (c == 'q' || c == 'm' || c == 'j') e.term = {t[1], t[2], t[3]};
    else if (c == 'v') { if (t[2] == "0") { e.global = true; e.drv = t[1]; } else e.term = {t[1], t[2]}; }
    else if (c == 'e') { e.drv = t[1]; e.sense = {t[3], t[4]}; }
    else if (c == 'b') { if (L.find(" i =") != std::string::npos || L.find(" i=") != std::string::npos) e.term = {t[1], t[2]}; else e.drv = t[1]; static const std::regex r(R"(v\(([^),]+)(?:,([^)]+))?\))"); for (std::sregex_iterator it(L.begin(), L.end(), r), en; it != en; ++it) { e.sense.push_back((*it)[1]); if ((*it)[2].matched) e.sense.push_back((*it)[2]); } }
    else if (c == 's') { e.term = {t[1], t[2]}; e.sense = {t[3], t[4]}; }
    else if (c == 'x') { e.drv = t[5]; e.sense = {t[1], t[2], t[3], t[4]}; }
    else { std::fprintf(stderr, "blocksolve: unsupported element %s\n", t[0].c_str()); return 1; }
    els.push_back(e); }
  std::set<std::string> bnd = {"0"}; std::map<std::string, int> drvOf, glob;
  for (int i = 0; i < (int)els.size(); ++i) if (!els[i].drv.empty()) { bnd.insert(els[i].drv); (els[i].global ? glob : drvOf)[els[i].drv] = i; }
  auto nb = [&](const std::string& n) { return !bnd.count(n); };
  // MOSFET gates driven by an op-amp output: they carry no DC current and, for a cascode passing ~1e-20 A (cut-off taps), are
  // loosely determined (below ngspice abstol 1e-14 A the gate is set by numerical noise). Verification checks them at 1e-2 rel + 1e-3 V; the cascode source node and every current it
  // passes on are still checked at the tight tolerance.
  std::set<std::string> gate; for (auto& e : els) if (e.name[0] == 'm' && e.term.size() == 3 && drvOf.count(e.term[1])) gate.insert(e.term[1]);
  for (auto& e : els) { std::string first; for (auto& n : e.term) if (nb(n)) { if (first.empty()) first = n; else Un(first, n); F(n); }
    if (!first.empty() && e.drv.empty()) for (auto& n : e.sense) if (nb(n)) Un(first, n); }  // controlled current sources: same block as what they read
  std::vector<std::string> key(els.size());
  for (int i = 0; i < (int)els.size(); ++i) { auto& e = els[i]; if (e.global) continue;
    if (!e.drv.empty()) { std::string first; for (auto& n : e.sense) if (nb(n)) { if (first.empty()) first = n; else Un(first, n); }
      if (first.empty()) F("drv:" + e.name); } }
  for (int i = 0; i < (int)els.size(); ++i) { auto& e = els[i]; if (e.global) continue; std::string k;
    if (!e.drv.empty()) { for (auto& n : e.sense) if (nb(n)) { k = n; break; } if (k.empty()) k = "drv:" + e.name; }
    else { for (auto& n : e.term) if (nb(n)) { k = n; break; } }
    key[i] = k; }
  // elements with only stiff terminals join the block that drives their first driven terminal
  for (int i = 0; i < (int)els.size(); ++i) if (!els[i].global && key[i].empty()) {
    for (auto& n : els[i].term) if (drvOf.count(n)) { key[i] = key[drvOf[n]]; break; }
    if (key[i].empty()) key[i] = "misc"; }
  std::map<std::string, int> bid; std::vector<std::vector<int>> mem;
  for (int i = 0; i < (int)els.size(); ++i) if (!els[i].global) { std::string r = F(key[i]); if (!bid.count(r)) { bid[r] = (int)mem.size(); mem.emplace_back(); } mem[bid[r]].push_back(i); }
  const int NB = (int)mem.size(); std::vector<int> blk(els.size(), -1);
  for (int b = 0; b < NB; ++b) for (int i : mem[b]) blk[i] = b;
  std::vector<std::set<int>> dep(NB);
  for (int i = 0; i < (int)els.size(); ++i) if (blk[i] >= 0) { std::vector<std::string> ns = els[i].term; ns.insert(ns.end(), els[i].sense.begin(), els[i].sense.end());
    for (auto& n : ns) if (drvOf.count(n)) { int a = blk[drvOf[n]]; if (a != blk[i]) dep[blk[i]].insert(a); } }
  // Tarjan SCC (emits dependencies first)
  std::vector<int> idx(NB, -1), low(NB), st, comp(NB, -1); std::vector<bool> on(NB); int cnt = 0, nc = 0; std::vector<std::vector<int>> sccs;
  std::function<void(int)> sc = [&](int v) { idx[v] = low[v] = cnt++; st.push_back(v); on[v] = true;
    for (int w : dep[v]) { if (idx[w] < 0) { sc(w); low[v] = std::min(low[v], low[w]); } else if (on[w]) low[v] = std::min(low[v], idx[w]); }
    if (low[v] == idx[v]) { sccs.emplace_back(); int w; do { w = st.back(); st.pop_back(); on[w] = false; comp[w] = nc; sccs.back().push_back(w); } while (w != v); ++nc; } };
  for (int v = 0; v < NB; ++v) if (idx[v] < 0) sc(v);
  // Amendment 4.4 SEED: FSOT_BS_SEED=<solution.ns of the SAME netlist key and temperature, ideal parts>. Used only as the initial
  // guess (.nodeset) of a 4th attempt after the three standard attempts fail; the result must still pass the unchanged verify
  // (plain Newton reproduces it, re-anchor allowed). A Monte Carlo operating point lies near the ideal one; the seed only places
  // Newton in its basin, it is never copied into the solution.
  std::map<std::string, std::string> seed; if (const char* e = std::getenv("FSOT_BS_SEED")) { std::ifstream sf(e); std::string a, eq, v; while (sf >> a >> eq >> v) if (a.find('#') == std::string::npos) seed[a] = v; }
  std::string tran; if (const char* e = std::getenv("FSOT_BS_TRAN")) tran = e;  // "tstep tstop": block-sequential transient (uic)
  std::map<std::string, std::vector<double>> wav; std::vector<double> tgrid;
  std::map<std::string, std::string> sol; int maxsz = 0, merged = 0, retried = 0; std::string fail;
  for (int k = 0; k < nc && fail.empty(); ++k) {
    std::vector<int> ei; for (int b : sccs[k]) ei.insert(ei.end(), mem[b].begin(), mem[b].end()); std::sort(ei.begin(), ei.end());
    if (sccs[k].size() > 1) ++merged; maxsz = std::max(maxsz, (int)ei.size());
    std::set<std::string> own, ext, gl; for (int i : ei) { for (auto& n : els[i].term) if (nb(n)) own.insert(n); if (!els[i].drv.empty()) own.insert(els[i].drv);
      for (auto& n : els[i].sense) if (nb(n)) own.insert(n); }
    for (int i : ei) { std::vector<std::string> ns = els[i].term; ns.insert(ns.end(), els[i].sense.begin(), els[i].sense.end());
      for (auto& n : ns) { if (glob.count(n)) gl.insert(n); else if (drvOf.count(n) && comp[blk[drvOf[n]]] != k) ext.insert(n); } }
    auto write = [&](const std::string& fn, int att) { std::ofstream o(fn); o << "* block " << k << "\n";
      for (auto& m : models) o << m << "\n";
      for (auto& g : gl) o << els[glob[g]].line << "\n";
      for (auto& x : ext) { if (tran.empty()) { o << "VFIX_" << x << " " << x << " 0 DC " << sol.at(x) << "\n"; continue; }
        o << "VFIX_" << x << " " << x << " 0 PWL("; const auto& w = wav.at(x); char b[64];
        for (size_t j = 0; j < w.size(); ++j) { std::snprintf(b, sizeof b, "%s%.9e %.15e", j ? (j % 4 ? " " : "\n+ ") : "", tgrid[j], w[j]); o << b; } o << ")\n"; }
      for (int i : ei) o << els[i].line << "\n";
      if (att == 3) { for (auto& n : own) if (seed.count(n)) o << ".nodeset v(" << n << ")=" << seed[n] << "\n"; }   // SEED attempt (Amendment 4.4)
      else for (auto& n : own) if (nodeset.count(n)) o << ".nodeset v(" << n << ")=" << nodeset[n] << "\n";
      std::string op = options; if (att == 1) op += " gmin=1e-12"; if (att == 2) { op = std::regex_replace(op, std::regex("reltol=[^ ]+"), "reltol=1e-5"); op += " gmin=1e-12 klu"; }
      if (att != 2) { auto p = op.find(' '); op.insert(p, " sparse"); }
      if (tran.empty()) o << op << "\n.control\nset numdgt=15\nop\nprint all\nquit\n.endc\n.end\n";
      else { std::istringstream ts(tran); std::string a, b; ts >> a >> b;
        o << op << " interp\n.control\nset wr_singlescale\nset wr_vecnames\noption numdgt=15\ntran " << a << " " << b << " 0 " << a << " uic\nwrdata " << fn << ".dat all\nquit\n.endc\n.end\n"; } };
    bool ok = false;
    for (int att = 0; att < (seed.empty() ? 3 : 4) && !ok; ++att) {
      std::string fn = wd + "/b" + std::to_string(k) + ".cir", lg = wd + "/b" + std::to_string(k) + ".log"; write(fn, att);
      std::system(("timeout 600 ngspice -b " + fn + " > " + lg + " 2>&1").c_str());
      std::ifstream r(lg); std::map<std::string, std::string> got; std::string q; bool nan = false;
      static const std::regex rv(R"(^(?:v\()?([a-z0-9_.#]+)\)? += (\S+)\s*$)");
      while (std::getline(r, q)) { std::smatch m; if (std::regex_match(q, m, rv)) { got[m[1]] = m[2]; if (m[2].str().find("nan") != std::string::npos) nan = true; } }
      if (!tran.empty()) { got.clear(); std::ifstream d(fn + ".dat"); std::string h; std::vector<std::string> nm; std::vector<std::vector<double>> col;
        if (std::getline(d, h)) { std::istringstream hs(h); std::string x; while (hs >> x) { x = lc(x); std::smatch m; if (std::regex_match(x, m, std::regex(R"(v\(([^)]+)\))"))) x = m[1]; nm.push_back(x); } col.resize(nm.size());
          while (std::getline(d, h)) { std::istringstream ls(h); for (size_t j = 0; j < nm.size(); ++j) { double v; if (!(ls >> v)) break; col[j].push_back(v); if (v != v) nan = true; } } }
        if (!nm.empty() && !col[0].empty()) { if (tgrid.empty()) tgrid = col[0]; if (col[0].size() != tgrid.size()) nan = true;
          for (size_t j = 1; j < nm.size(); ++j) { got[nm[j]] = std::to_string(col[j].back()); if (!ext.count(nm[j])) wav[nm[j]] = col[j]; } } }
      bool all = true; for (auto& n : own) if (!got.count(n)) all = false;
      if (all && !nan && tran.empty()) {  // verify: plain Newton from the reported point (no stepping, no pseudo-transient);
        // if plain Newton cannot start (a cascode passing ~1e-13 A at 0 C), one retry with gmin stepping only: a gmin-stepped
        // operating point is a true DC solution once gmin is removed. Any use of the pseudo-transient op still rejects the block.
        bool bad = true, reanchor = false; std::map<std::string, std::string> g2;
        // blocksolve4 (Phase 3b fork; Phase 2 blocksolve.cpp is locked and unchanged): RE-ANCHOR. If the reported point is not
        // reproduced, but plain Newton (or the gmin-stepped retry) from it lands cleanly (no pseudo-transient op, every node
        // present) on a DIFFERENT point, that point becomes the candidate. It is accepted only if a second plain Newton solve
        // started there reproduces it within the SAME tolerance, i.e. it is a self-consistent DC fixed point. Tolerances unchanged.
        for (int anc = 0; anc < 2 && bad; ++anc) { bool clean = false;
        if (anc == 1) { if (!reanchor) break; got = g2; if (std::getenv("FSOT_BS_DEBUG")) std::fprintf(stderr, "block %d att %d: re-anchor at the verify point\n", k, att); }
        for (int vt = 0; vt < 2 && bad; ++vt) {
          std::ifstream src(fn); std::ofstream v(fn + ".v.cir"); std::string q2;
          while (std::getline(src, q2)) { std::string L2 = lc(q2); if (L2.rfind(".nodeset", 0) == 0) continue;
            if (L2.rfind(".options", 0) == 0) { for (auto& n : own) if (got.count(n)) v << ".nodeset v(" << n << ")=" << got[n] << "\n";
              q2 = std::regex_replace(q2, std::regex("(gminsteps|srcsteps|itl1)=[^ ]+"), ""); q2 = std::regex_replace(q2, std::regex(" klu\\b"), ""); if (q2.find(" sparse") == std::string::npos) q2 += " sparse";
              q2 += vt == 0 ? " gminsteps=0 srcsteps=0 itl1=200 noopac" : " gminsteps=50 srcsteps=0 itl1=500 noopac"; v << q2 << "\n"; continue; }
            v << q2 << "\n"; }
          v.close(); std::system(("timeout 600 ngspice -b " + fn + ".v.cir > " + lg + ".v 2>&1").c_str()); if (std::getenv("FSOT_BS_DEBUG")) std::system(("cp " + fn + ".v.cir " + fn + ".v" + std::to_string(att) + std::to_string(vt) + ".cir; cp " + lg + ".v " + lg + ".v" + std::to_string(att) + std::to_string(vt)).c_str());
          std::ifstream r2(lg + ".v"); g2.clear(); bad = false;
          while (std::getline(r2, q)) { std::smatch m; if (std::regex_match(q, m, rv)) g2[m[1]] = m[2]; if (q.find("could not be simulated") != std::string::npos || q.find("ransient op") != std::string::npos) bad = true; }
          if (bad && std::getenv("FSOT_BS_DEBUG")) std::fprintf(stderr, "block %d att %d verify %d: no plain/gmin-stepped solution\n", k, att, vt);
          clean = !bad; if (!bad) for (auto& n : own) { if (!g2.count(n)) { bad = true; clean = false; if (std::getenv("FSOT_BS_DEBUG")) std::fprintf(stderr, "block %d: %s missing\n", k, n.c_str()); break; }
            const double a = std::atof(got[n].c_str()), b = std::atof(g2[n].c_str());
            const bool g = gate.count(n) > 0; if (std::fabs(a - b) > (g ? 1e-2 : 1e-5) * std::fabs(a) + (g ? 1e-3 : 1e-7)) { bad = true; if (std::getenv("FSOT_BS_DEBUG")) std::fprintf(stderr, "block %d: %s %g vs %g\n", k, n.c_str(), a, b); break; } } }
        reanchor = clean && bad; }
        if (bad) all = false; }
      if (all && !nan) { ok = true; for (auto& [a, v] : got) if (!ext.count(a)) sol[a] = v; if (att) ++retried; }
    }
    if (!ok) fail = "block " + std::to_string(k) + " (" + std::to_string(ei.size()) + " elements, first " + els[ei[0]].name + ")";
  }
  std::ofstream o(outlog);
  if (fail.empty()) for (auto& p : prints) { std::istringstream s(p); std::string v; while (s >> v) { std::string n = v; std::smatch m;
      if (std::regex_match(v, m, std::regex(R"(v\(([^)]+)\))"))) n = m[1]; else if (std::regex_match(v, m, std::regex(R"(i\(([^)]+)\))"))) n = m[1].str() + "#branch";
      if (!tran.empty() && (v.rfind("fin_", 0) == 0 || v.rfind("rip_", 0) == 0)) { std::string nn = v.substr(4); if (!wav.count(nn)) continue; const auto& w = wav[nn];
        const double fin = w.back(); if (v[0] == 'f') { char b[64]; std::snprintf(b, sizeof b, "%.15e", fin); o << v << " = " << b << "\n"; continue; }
        size_t k0 = w.size() > 251 ? w.size() - 251 : 0; double mx = -1e300, mn = 1e300; for (size_t j = k0; j < w.size(); ++j) { mx = std::max(mx, w[j]); mn = std::min(mn, w[j]); }
        size_t js = w.size(); while (js > 0 && std::fabs(w[js - 1] - fin) <= 1e-5 * std::fabs(fin) + 1e-7) --js;
        o << v << " = " << (mx - mn) << "\nsettle_" << nn << " = " << (js < w.size() ? tgrid[js] : tgrid.back()) << "\n"; continue; }
      if (sol.count(n)) o << v << " = " << sol[n] << "\n"; } }
  { std::ofstream a(wd + "/solution.ns"); for (auto& [n, v] : sol) a << n << " = " << v << "\n"; }
  o << "SOLVER: " << (fail.empty() ? "blocks" : "FAILED blocks at " + fail) << " (" << nc << " blocks, " << merged << " merged loops, largest " << maxsz << " elements, " << retried << " retried)\n";
  std::printf("%s", fail.empty() ? "OK\n" : ("FAIL " + fail + "\n").c_str());
  return fail.empty() ? 0 : 2;
}
