#!/usr/bin/env python3
"""hubcmp.py (LOCK9H): compare the hub bench transients with the C++ (exact + RK4) and Python (RK4) references.
Usage: hubcmp.py <locked9h dir> <hubref_py.tsv> <compiled.dat> <exact.dat> <results dir>
SPICE: wrdata columns t,V(outS),t,V(pF),t,V(p25),t,V(oS); state = V/2 (2 V per unit); t_unit = t/T0. Sampled every 0.25 unit by linear
interpolation of the 5 us transient grid. Also reports the bench against the exact solution evaluated with the stage-1 compiled coefficients
(isolates the bench's integration error from the coefficient compile error)."""
import sys, math, bisect
L, PY, DC, DE, RES = sys.argv[1:6]
V = {l.split('\t')[0]: float(l.split('\t')[1]) for l in open(L + '/hubbench_values.tsv')}
S1 = {l.split('\t')[0]: float(l.split('\t')[1]) for l in open(L + '/hub_stage1.tsv') if not l.startswith('#')}
cpp = [l.split('\t') for l in open(L + '/hubref_cpp.tsv').read().splitlines()[1:]]
py = [l.split('\t') for l in open(PY).read().splitlines()[1:]]
def load(f):
    t, a, b, c = [], [], [], []
    for l in open(f):
        x = l.split()
        if len(x) < 8: continue
        t.append(float(x[0]) / V['T0']); a.append(float(x[1]) / 2); b.append(float(x[3]) / 2); c.append(float(x[5]) / 2)
    return t, a, b, c
def at(T, Y, t):
    i = bisect.bisect_left(T, t)
    if i < len(T) and abs(T[i] - t) < 1e-12: return Y[i]
    i = max(1, min(i, len(T) - 1)); w = (t - T[i - 1]) / (T[i] - T[i - 1]); return Y[i - 1] + w * (Y[i] - Y[i - 1])
sc, se = load(DC), load(DE)
g, mF, m25, J, Sq = (S1['v(za1xo)'] / 10, S1['v(za96xo)'] / 10, S1['v(zt101xo)'] / 10, S1['v(zt104xo)'] / 100, S1['v(vsdh)'] / 2)
cc = lambda t: (Sq * (1 - math.exp(-g * t)), math.exp(-mF * t), J / m25 + (1 - J / m25) * math.exp(-m25 * t))
rows = []; H = ['t', 'mode', 'exact_cpp', 'rk4_cpp', 'rk4_py', 'spice_exactcoef', 'spice_compiled', 'exact_compiledcoef']
for r, p in zip(cpp, py):
    t = float(r[0]); cv = cc(t)
    for k, (m, ie, ir, ip) in enumerate([('S', 1, 2, 1), ('v_Fluid', 3, 4, 2), ('v_D25', 5, 6, 3)]):
        rows.append([t, m, float(r[ie]), float(r[ir]), float(p[ip]), at(se[0], se[k + 1], t), at(sc[0], sc[k + 1], t), cv[k]])
with open(RES + '/hub9_compare.tsv', 'w') as o:
    o.write('\t'.join(H) + '\n')
    for x in rows: o.write('%.2f\t%s\t' % (x[0], x[1]) + '\t'.join('%.12e' % y for y in x[2:]) + '\n')
# summary: max |diff| over t in (0, 8], per mode, scaled by the mode's range (|S_eq| for S, 1 for v)
def mx(m, a, b, rel=False):
    d = [abs(x[a] - x[b]) / (abs(x[b]) if rel else 1) for x in rows if x[1] == m and x[0] > 0]; return max(d)
with open(RES + '/hub9_summary.tsv', 'w') as o:
    o.write('mode\tpy_vs_cpp_rk4\tcpp_rk4_vs_exact\tspice_exactcoef_vs_exact\tspice_compiled_vs_exact\tspice_compiled_vs_exact_compiledcoef\tspice_compiled_vs_py\tfinal_t8_spice_compiled\tfinal_t8_exact\n')
    for m in ['S', 'v_Fluid', 'v_D25']:
        f = [x for x in rows if x[1] == m and abs(x[0] - 8) < 1e-9][0]
        o.write('%s\t%.2e\t%.2e\t%.2e\t%.2e\t%.2e\t%.2e\t%.9f\t%.9f\n' % (m, mx(m, 4, 3), mx(m, 3, 2), mx(m, 5, 2), mx(m, 6, 2), mx(m, 6, 7), mx(m, 6, 4), f[6], f[2]))
print(open(RES + '/hub9_summary.tsv').read())
