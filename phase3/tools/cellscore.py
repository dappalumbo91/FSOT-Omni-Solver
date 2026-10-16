#!/usr/bin/env python3
"""cellscore.py (LOCK10A): per-cell error budget from the isolated-cell bench logs (phase2/spice/out/c10_<cell>_<run>.log) against the locked
closed-form predictions (cellpred.tsv). Error definitions: phase3/cpp/cells10.hpp. Usage: cellscore.py <cellpred.tsv> <out_budget.tsv> [log prefix c10]"""
import sys, os, math, re
PRED, OUT = sys.argv[1], sys.argv[2]; PFX = sys.argv[3] if len(sys.argv) > 3 else 'c10'
D = os.path.join(os.path.dirname(os.path.abspath(__file__)), '../../phase2/spice/out')
KQ = 1.38064852e-23 / 1.6021766208e-19; VT = KQ * 300.15; E3M = 0.049787068367863942979; KD = E3M / 0.7
C = {'LOGRa': ('LOGR', 1e-6, 1e-6 * E3M), 'LOGRb': ('LOGR', 1e-6, 1e-6 * 0.36787944117144233), 'LOGRp': ('LOGR', 1e-6, 1e-6 * E3M), 'LOGRk': ('LOGR', 5e-6, 5e-6 * 0.7 * KD),
     'CALa': ('CAL', 1e-6, -3), 'CALb': ('CAL', 1e-6 * math.exp(1.5), -4.5), 'CALc': ('CAL', 1e-6 * E3M, -3), 'CALt': ('CAL', 7e-6, -3), 'CALUa': ('CAL', 1e-6, -3),
     'SUMa': ('SUM', [(-3, 1), (-3, 1), (-10, 0.3)]), 'SUMb': ('SUM', [(-3, 2), (-3, 3), (-3, 4), (10, 3)]), 'TIAa': ('TIA', 1e-6 * E3M), 'VNEP': ('VNEP',)}
def val(log, node):
    for l in open(log):
        m = re.match(r'v\(%s\) = (\S+)' % node.lower(), l.strip().lower())
        if m: return float(m.group(1))
    return None
def err(cell, run):
    f = os.path.join(D, '%s_%s_%s.log' % (PFX, cell, run))
    if not os.path.exists(f): return None
    k = C[cell]
    try:
        if k[0] == 'LOGR': return val(f, 'nlc') / VT - math.log(k[2] / k[1])
        if k[0] == 'CAL': Iin, vx = k[1], k[2]; RT = 1 / (Iin * math.exp(vx)); return math.log(val(f, 'tout') / RT / Iin) - vx
        if k[0] == 'SUM': ideal = -sum(w * v * VT for v, w in k[1]); return (val(f, 'sout') - ideal) / VT
        if k[0] == 'TIA': I = k[1]; RT = 1 / I; return math.log(-val(f, 'tout') / (I * RT))
        if k[0] == 'VNEP': return val(f, 'vnep') / (10 * VT) - 1
    except TypeError: return None
P = {}
for l in open(PRED).read().splitlines()[1:]:
    c, m, e, u = l.split('\t'); P[(c, m)] = float(e)
MS = ['hi', 'rbib', 'ib', 'early', 'gain', 'round']
with open(OUT, 'w') as o:
    o.write('cell\tmechanism\tpredicted\tmeasured\tmeasured_minus_predicted\n')
    for c in C:
        n0 = err(c, 'none'); o.write('%s\tnone\t%.3e\t%s\t%s\n' % (c, P[(c, 'none')], '%.3e' % n0 if n0 is not None else 'NA', '%.1e' % (n0 - P[(c, 'none')]) if n0 is not None else 'NA'))
        sp = P[(c, 'none')]; sm = n0 if n0 is not None else float('nan')
        for m in MS:
            x = err(c, 'only_' + m); meas = (x - n0) if (x is not None and n0 is not None) else None
            sp += P[(c, m)]; sm += meas if meas is not None else float('nan')
            o.write('%s\t%s\t%.3e\t%s\t%s\n' % (c, m, P[(c, m)], '%.3e' % meas if meas is not None else 'NA', '%.1e' % (meas - P[(c, m)]) if meas is not None else 'NA'))
        f = err(c, 'full')
        o.write('%s\tfull(sum of parts)\t%.3e\t%s\t%s\n' % (c, sp, '%.3e' % f if f is not None else 'NA', '%.1e' % (f - sp) if f is not None else 'NA'))
print(open(OUT).read())
