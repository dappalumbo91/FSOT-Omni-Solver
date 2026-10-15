// ovennet.cpp (C++20): oven netlist from the locked oven_values.tsv. usage: ovennet <ambient C> <op|tran> <out.cir>
// Thermal network as its electrical analog (1 V = 1 C, 1 A = 1 W). Sensor VBE is the Gummel-Poon law of the core's
// 2N3904 card evaluated at the sensor temperature (implicit B source). Op-amps: TI OPAx187 vendor model.
// TIP31 heater: behavioural Ic = max(0, V_b - 0.7 V)/R_e (class-A emitter follower), P = VCC * Ic.
#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
int main(int argc, char** argv) {
  if (argc < 4) return 1; std::map<std::string, double> V; std::ifstream in("../locked/oven_values.tsv"); std::string l; std::getline(in, l);
  while (std::getline(in, l)) { std::istringstream s(l); std::string n; double a, b; s >> n >> a >> b; V[n] = b; }
  const double Ta = std::stod(argv[1]); const std::string an = argv[2]; const bool ideal = argc > 4 && std::string(argv[4]) == "ideal"; std::ofstream o(argv[3]); o.precision(10);
  o << "* FSOT Omni-Solver Phase 2 oven (values from oven/oven_values.tsv, locked) ambient " << Ta << " C\n";
  if (!ideal) o << ".include ../spice/models/vendor/ti_sbomab6/OPAx187.LIB\n";
  // ideal (harness) op-amp: gain 1e5, output limited to the OPA187 swing (+-8.9 V at +-9 V), one pole at 16 Hz (GBW 1.6 MHz)
  auto OA = [&](const std::string& n, const std::string& p, const std::string& m, const std::string& out) {
    if (!ideal) { o << "X" << n << " " << p << " " << m << " vcc vee " << out << " OPAx187\n"; return; }
    o << "Boa" << n << " oa" << n << "i 0 V = 1e5*(V(" << p << ")-V(" << m << "))\nRoa" << n << " oa" << n << "i oa" << n << "f 1Meg\nCoa" << n << " oa" << n << "f 0 10n\n"
      << "Boa" << n << "o " << out << " 0 V = max(-8.9, min(8.9, V(oa" << n << "f)))\n"; };
  o << "VCC vcc 0 9\nVEE vee 0 -9\nVREF vr 0 2.5\n";
  o << "VAMB tamb 0 " << Ta << "\nRth tdie tamb " << V["theta_sa"] << "\nCth tdie tamb " << V["C_th"] << (an == "tran" ? " IC=0" : "") << "\n";
  o << "Bheat 0 tdie I = " << V["VCC"] << "*max(0,(V(vb)-0.7)/" << V["R_e"] << ")\n";
  o << "Bcore tcore 0 V = V(tdie) - " << V["theta_g"] << "*" << V["VCC"] << "*max(0,(V(vb)-0.7)/" << V["R_e"] << ")\n";
  o << ".param IS=6.734e-15 XTI=3 EG=1.11 TN=300.15 KQ=8.617333262e-5\n";
  o << "Bvbe vbe 0 V = KQ*(V(tdie)+273.15)*ln(max(1e-12,(2.5-V(vbe))/" << V["R_s"] << ")/(IS*pwr((V(tdie)+273.15)/TN,XTI)*exp(EG/(KQ*(V(tdie)+273.15))*((V(tdie)+273.15)/TN-1))))\n";
  o << "Rtop vr vset " << V["R_top"] << "\nRbot vset 0 " << V["R_bot"] << "\n";
  o << "Rgi vbe ne " << V["R_gin"] << "\nRgr vset pe " << V["R_gin"] << "\nRgp pe 0 " << V["R_gf"] << "\nRgf ne verr " << V["R_gf"] << "\n";   // diff amp: verr = -G (vbe - vset)
  OA("E", "pe", "ne", "verr");
  o << "Ri verr ni " << V["R_i"] << "\nRp ni pz " << V["R_p"] << "\nCi pz vb " << V["C_i"] << "\n"; OA("I", "0", "ni", "vb");
  o << ".options reltol=1e-6 abstol=1e-12 vntol=1e-9 itl1=5000 gminsteps=50 srcsteps=50 method=gear\n.control\n";
  if (an == "tran") o << "tran 1 1500 uic\nlet n = length(time)-1\nmeas tran t_settle when v(tcore)=" << V["T_set"] - 0.1 << " rise=1\nmeas tran tmax max v(tcore)\nprint v(tcore)[n] v(tdie)[n] v(vb)[n]\nwrdata oven_tran_" << Ta << ".txt v(tcore) v(vb)\n";
  else o << "op\nprint v(tcore) v(tdie) v(vbe) v(vset) v(vb) v(verr)\n";
  o << "quit\n.endc\n.end\n";
}
