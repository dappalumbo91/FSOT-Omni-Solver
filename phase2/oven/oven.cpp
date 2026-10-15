// oven.cpp (C++20): analog oven for the Phase 2 die (PTAT + matched-pair block). Not an FSOT quantity: standard physics.
// Writes oven_values.tsv (ideal + E24 realisation). Locked (sha256) before oven.cir is simulated.
// Sensor: 2N3904 (same Gummel-Poon card as the core) diode-connected, fed from VR = 2.5 V through R_s.
// Set point: T_set = 60 C (> 50 C max ambient + 10 K headroom). Controller: OPA187 error amp (gain -G) + OPA187 PI
// integrator -> TIP31 heater (emitter resistor R_e, collector at VCC = 9 V). Thermal block: theta_sa, C_th (aluminium
// block carrying the transistor arrays); sensor-to-core gradient theta_g per watt of heater power.
#include <cmath>
#include <cstdio>
#include <fstream>
#include <vector>
#include <string>
static double e24(double x) { static const double s[] = {1.0,1.1,1.2,1.3,1.5,1.6,1.8,2.0,2.2,2.4,2.7,3.0,3.3,3.6,3.9,4.3,4.7,5.1,5.6,6.2,6.8,7.5,8.2,9.1};
  double d = std::pow(10.0, std::floor(std::log10(x))), best = s[0] * d; for (double v : s) for (double m : {d, 10 * d}) if (std::fabs(v * m - x) < std::fabs(best - x)) best = v * m; return best; }
int main() {
  const double k = 1.380649e-23, q = 1.602176634e-19, VR = 2.5, IS = 6.734e-15, XTI = 3, EG = 1.11, Tn = 300.15;
  const double Tset = 60 + 273.15, Is_bias = 20e-6;
  auto VT = [&](double T) { return k * T / q; };
  auto ISt = [&](double T) { return IS * std::pow(T / Tn, XTI) * std::exp(EG / VT(T) * (T / Tn - 1)); };
  const double Rs_ideal = (VR - 0.5) / Is_bias, Rs = e24(Rs_ideal);
  auto vbe = [&](double T) { double v = 0.5; for (int i = 0; i < 200; ++i) v = VT(T) * std::log((VR - v) / Rs / ISt(T)); return v; };
  const double Vset = vbe(Tset), slope = (vbe(Tset + 0.5) - vbe(Tset - 0.5));
  // divider VR -> Vset: R_top + R_bot = 100k nominal, E24 pair chosen to land closest
  double bestErr = 1, Rt = 0, Rb = 0; for (double a = 10e3; a < 1e6; a *= 1.0001) { double t = e24(a), b = e24(t * Vset / (VR - Vset)); double e = std::fabs(VR * b / (t + b) - Vset); if (e < bestErr) { bestErr = e; Rt = t; Rb = b; } }
  const double Vset_r = VR * Rb / (Rt + Rb), Tset_r = Tset + (Vset_r - Vset) / slope;
  const double theta = 40, Cth = 3, theta_g = 1.0, G = 100, Rg_in = 10e3, Rg_f = G * Rg_in, Ri = 100e3, Ci = 10e-6, Rp = 1.8e6, Re = 22, VCC = 9;
  std::ofstream o("oven_values.tsv"); o.precision(10);
  o << "name\tideal\trealised\tunit\tmeaning\n";
  auto row = [&](const char* n, double a, double b, const char* u, const char* m) { o << n << '\t' << a << '\t' << b << '\t' << u << '\t' << m << '\n'; };
  row("R_s", Rs_ideal, Rs, "ohm", "sensor bias from VR (~20 uA)");
  row("V_set", Vset, Vset_r, "V", "sensor VBE at T_set (Gummel-Poon IS(T), XTI=3, EG=1.11)");
  row("R_top", Rt, Rt, "ohm", "set-point divider top (E24)"); row("R_bot", Rb, Rb, "ohm", "set-point divider bottom (E24)");
  row("T_set", Tset - 273.15, Tset_r - 273.15, "C", "set point; realised value from the E24 divider");
  row("dVBE_dT", slope, slope, "V/K", "sensor slope at T_set");
  row("R_gin", Rg_in, Rg_in, "ohm", "error amp input"); row("R_gf", Rg_f, Rg_f, "ohm", "error amp feedback (gain -100)");
  row("R_i", Ri, Ri, "ohm", "PI integrator input"); row("C_i", Ci, Ci, "F", "PI integrator capacitor (film)"); row("R_p", Rp, Rp, "ohm", "PI zero (R_p C_i = 18 s)");
  row("R_e", Re, Re, "ohm", "TIP31 heater emitter resistor"); row("VCC", VCC, VCC, "V", "heater supply");
  row("theta_sa", theta, theta, "K/W", "block-to-ambient thermal resistance"); row("C_th", Cth, Cth, "J/K", "block heat capacity");
  row("theta_g", theta_g, theta_g, "K/W", "sensor-to-core gradient per watt of heater power");
  std::printf("OVEN_OK Vset=%.6f slope=%.6e Tset_realised=%.3f C Rs=%g Rt=%g Rb=%g\n", Vset, slope, Tset_r - 273.15, Rs, Rt, Rb);
}
