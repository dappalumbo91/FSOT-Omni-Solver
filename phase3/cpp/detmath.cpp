// detmath.cpp — FSOT Omni-Solver LOCK8 Amendment 8.1: deterministic transcendental functions for the C++20 derivers.
// Why: glibc's x86-64 long double logl/expl/powl/log10l (and sinl/cosl/sincosl/acosl, and the FMA-dispatched double pow) are computed with
// x87 transcendental instructions (fyl2x, f2xm1, fsin/fcos, fpatan) or CPU-dispatched code paths. Their last bit differs between CPU models
// (GitHub's ubuntu-latest pool mixes Intel and AMD hosts), so a value sitting on a decision threshold (cpp/design.hpp realise(): "best.rel > 5e-4",
// LOCK7 leafc7: the antilog exponent a ~ 1e-19 vs 0) or printed to 18 digits re-derived differently on some runners.
// What: linked into a deriver, these definitions take precedence over libm's (the executable's own symbol binds first), for the deriver's
// code and for the inline engine headers alike. They use only IEEE + - * / on the x87 80-bit format (and exact operations: scaling by 2^k,
// floor), compiled with -ffp-contract=off, so their result is the same bits on every x86-64 CPU and every glibc. Accuracy: a few ulp of
// long double (checked against libm by detmath_check in verify_lock8b.sh: max relative difference < 1e-17 over the test grid).
// No FSOT rule or value is involved; this is arithmetic plumbing.
// Test hook (never used for a lock): -DDETMATH_PERTURB=+1 or -1 moves every expl/logl/sinl/cosl result by one ulp (x (1 +- 2^-63)), to emulate a
// different CPU's transcendental unit; tools/check_hostindep8b.sh checks that the LOCK8b outputs do not change.
#include <cstdint>
#include <cstring>
#include <limits>
using LD = long double;
namespace {
constexpr LD LN2_HI = 0.6931471803691238164901733L;     // ln 2 truncated to 32 significant bits: k LN2_HI is exact for |k| < 2^32
constexpr LD LN2_LO = 1.908214929270587816142319e-10L;     // ln2 - LN2_HI (rounded to 64 bits)
constexpr LD LN2 = 0.693147180559945309417232121458176568L;
constexpr LD INV_LN2 = 1.442695040888963407359924681001892137L;
constexpr LD PIO2_1 = 1.57079632673412561416626L;      // pi/2 split: 32 + 32 + 64 bits
constexpr LD PIO2_2 = 6.077100506303965976595549e-11L;
constexpr LD PIO2_3 = 2.022266248795950732376519e-21L;
constexpr LD PI = 3.141592653589793238462643383279502884L;
constexpr LD PIO2 = 1.570796326794896619231321691639751442L;
constexpr LD INV_PIO2 = 0.636619772367581343075535053490057448L;
LD qnan() { return std::numeric_limits<LD>::quiet_NaN(); }
LD inf() { return std::numeric_limits<LD>::infinity(); }
bool isnan_(LD x) { return x != x; }
LD fabs_(LD x) { return x < 0 ? -x : x; }
LD nearest(LD x) { LD f = __builtin_floorl(x + 0.5L); return f; }
LD scal(LD x, int k) { return __builtin_ldexpl(x, k); }   // exact (bit operation)
LD frexp_(LD x, int* e) { return __builtin_frexpl(x, e); }
// exp on |r| <= 0.35: Taylor to degree 22, Horner
LD exp_core(LD r) { LD s = 1; for (int n = 22; n >= 1; --n) s = 1 + s * r / n; return s; }
LD log_core(LD m) {   // m in [sqrt(1/2), sqrt 2): 2 atanh((m-1)/(m+1))
  LD f = (m - 1) / (m + 1), f2 = f * f, s = 0;
  for (int n = 45; n >= 3; n -= 2) s = (s + 1.0L / n) * f2;
  return 2 * f + 2 * f * s; }
// sin/cos on |r| <= pi/4 (+ tail)
LD sin_core(LD r) { LD r2 = r * r, s = 0; for (int n = 27; n >= 3; n -= 2) s = (s + 1) * (-r2) / ((LD)n * (n - 1)); return r + r * s; }
LD cos_core(LD r) { LD r2 = r * r, s = 0; for (int n = 28; n >= 2; n -= 2) s = (s + 1) * (-r2) / ((LD)n * (n - 1)); return 1 + s; }
void reduce(LD x, LD* r, int* q) {   // x = q pi/2 + r, Cody-Waite (|x| up to ~1e9 accurate)
  LD k = nearest(x * INV_PIO2); *r = ((x - k * PIO2_1) - k * PIO2_2) - k * PIO2_3;
  LD km = k - 4 * __builtin_floorl(k / 4); *q = (int)km; }
LD atan_core(LD x) {   // |x| <= tan(pi/24) ~ 0.1317
  LD x2 = x * x, s = 0; for (int n = 41; n >= 3; n -= 2) s = (s + ((n / 2) % 2 ? -1.0L : 1.0L) / n) * x2;
  return x + x * s; }
}
#ifdef DETMATH_PERTURB
#define PT(v) ((v) + (v) * (DETMATH_PERTURB * 1.0842021724855044340e-19L))
#else
#define PT(v) (v)
#endif
extern "C" {
LD expl(LD x) {
  if (isnan_(x)) return x; if (x > 11357.3L) return inf(); if (x < -11400.0L) return 0;
  LD k = nearest(x * INV_LN2); LD r = (x - k * LN2_HI) - k * LN2_LO;
  return PT(scal(exp_core(r), (int)k)); }
LD logl(LD x) {
  if (isnan_(x) || x < 0) return qnan(); if (x == 0) return -inf(); if (x == inf()) return x;
  int e; LD m = frexp_(x, &e); if (m < 0.70710678118654752440L) { m *= 2; e -= 1; }
  LD lm = log_core(m); return PT((e * LN2_HI) + (e * LN2_LO + lm)); }
LD log10l(LD x) { return logl(x) * 0.434294481903251827651128918916605082L; }
LD powl(LD x, LD y) {
  if (y == 0) return 1; if (isnan_(x) || isnan_(y)) return qnan(); if (x == 1) return 1;
  LD yi = __builtin_floorl(y); bool yint = (yi == y);
  if (yint && fabs_(y) <= 64) { LD b = x, r = 1; long long n = (long long)fabs_(y); while (n) { if (n & 1) r *= b; b *= b; n >>= 1; } return y < 0 ? 1 / r : r; }
  if (x == 0) return y > 0 ? 0 : inf();
  if (x < 0) { if (!yint) return qnan(); LD r = expl(y * logl(-x)); LD h = y / 2; return (__builtin_floorl(h) == h) ? r : -r; }
  return expl(y * logl(x)); }
double pow(double x, double y) { return (double)powl((LD)x, (LD)y); }
void sincosl(LD x, LD* s, LD* c) {
  if (isnan_(x) || fabs_(x) == inf()) { *s = *c = qnan(); return; }
  LD r; int q; reduce(x, &r, &q); LD sr = sin_core(r), cr = cos_core(r);
  switch (q) { case 0: *s = sr; *c = cr; break; case 1: *s = cr; *c = -sr; break; case 2: *s = -sr; *c = -cr; break; default: *s = -cr; *c = sr; }
  *s = PT(*s); *c = PT(*c); }
LD sinl(LD x) { LD s, c; sincosl(x, &s, &c); return s; }
LD cosl(LD x) { LD s, c; sincosl(x, &s, &c); return c; }
LD atanl(LD x) {
  if (isnan_(x)) return x; bool neg = x < 0; if (neg) x = -x; LD base = 0; bool inv = false;
  if (x > 1) { x = 1 / x; inv = true; }
  // x in [0,1]: reduce by tan(k pi/12): atan x = k pi/12 + atan((x - t)/(1 + x t))
  static const LD T[4] = {0.0L, 0.267949192431122706472553658494127633L, 0.577350269189625764509148780501957456L, 1.0L};
  int k = 0; if (x > 0.131652497587395853650L) k = 1; if (x > 0.414213562373095048801688724209698079L) k = 2; if (x > 0.767326987978960342396L) k = 3;
  LD z = k ? (x - T[k]) / (1 + x * T[k]) : x; base = k * (PI / 12);
  LD r = base + atan_core(z); if (inv) r = PIO2 - r; return neg ? -r : r; }
LD acosl(LD x) {
  if (isnan_(x) || x > 1 || x < -1) return qnan(); if (x == 1) return 0; if (x == -1) return PI;
  return 2 * atanl(__builtin_sqrtl((1 - x) / (1 + x))); }
LD asinl(LD x) { if (isnan_(x) || x > 1 || x < -1) return qnan(); return PIO2 - acosl(x); }
LD tanl(LD x) { LD s, c; sincosl(x, &s, &c); return s / c; }
LD atan2l(LD y, LD x) {
  if (isnan_(x) || isnan_(y)) return qnan();
  if (x > 0) return atanl(y / x); if (x < 0) return y >= 0 ? atanl(y / x) + PI : atanl(y / x) - PI;
  return y > 0 ? PIO2 : (y < 0 ? -PIO2 : 0); }
}
