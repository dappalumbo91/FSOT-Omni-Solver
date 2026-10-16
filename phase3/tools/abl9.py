#!/usr/bin/env python3
"""abl9.py: LOCK9A ablation table. Scores phase2/spice/out/bs9a_<variant>_<key>.log (ideal 27 C) with score9.sh against the variant's
leaves9_targets/generator9 and compares each leaf with the locked CI baseline (phase3/locked_ci/baseline_errors.tsv; T1 and 2a_X: bs6 rows).
Reporting only. Usage: abl9.py [variant ...]  -> phase3/results/ablation9.tsv (per variant x key) and ablation9_leaves.tsv."""
import subprocess, os, sys, statistics as st
R = os.path.dirname(os.path.abspath(__file__)); OUT = os.path.join(R, '../../phase2/spice/out'); RES = os.path.join(R, '../results')
KEYS = ['GEN', 'T1', '2_M', '2_P', '2_T', '2_X', '2a_M', '2a_P', '2a_T', '2a_X']
VARS = sys.argv[1:] or ['v71', 'flip', 'abs', 'flipabs', 'base', 'early', 'rho', 'all', 'v71_b6', 'all_b6']
base = {}
for l in open(os.path.join(R, '../locked_ci/baseline_errors.tsv')).read().splitlines()[1:]:
    k, nm, e = l.split('\t'); p = k.split('|')
    if p[0] == 'leaf': base[(p[2], p[3], p[1])] = float(e)
def bl(key, gid): return base.get((key, gid, 'bs6')) if key in ('T1', '2a_X') and (key, gid, 'bs6') in base else base.get((key, gid, 'bs5'))
def score(log, key, d):
    if not os.path.exists(log): return None
    p = subprocess.run([os.path.join(R, 'score9.sh'), log, key, d], capture_output=True, text=True).stdout.splitlines()
    r = {}
    for l in p[1:]:
        c = l.split('\t')
        if c[0] == 'summary' or c[2] == 'MISSING': continue
        r[c[0]] = (c[1], float(c[4]))
    return r
rows = []; leaves = []
for v in VARS:
    d = os.path.join(R, '../locked9a', v.replace('_b6', ''))
    allr = []
    for k in KEYS:
        s = score(f'{OUT}/bs9a_{v}_{k}.log', k, d)
        if s is None: rows.append((v, k, 'missing')); continue
        rat = []
        for gid, (nm, e) in s.items():
            b = bl(k, gid)
            if b is None: continue
            lim = max(2 * abs(b), 1e-6); rat.append((abs(e) / max(abs(b), 1e-12), abs(e), abs(b), abs(e) > lim)); leaves.append((v, k, gid, nm, e, b))
        allr += rat
        if rat: rows.append((v, k, len(rat), st.median([x[1] for x in rat]), st.median([x[2] for x in rat]), st.median([x[0] for x in rat]), sum(x[3] for x in rat), sum(1 for x in rat if x[1] <= x[2])))
    if allr: rows.append((v, 'ALL', len(allr), st.median([x[1] for x in allr]), st.median([x[2] for x in allr]), st.median([x[0] for x in allr]), sum(x[3] for x in allr), sum(1 for x in allr if x[1] <= x[2])))
with open(f'{RES}/ablation9.tsv', 'w') as o:
    o.write('variant\tkey\tleaves\tmedian_abs_err\tmedian_abs_baseline\tmedian_ratio\tover_2x_rule\tat_or_below_baseline\n')
    for r in rows: o.write('\t'.join(x if isinstance(x, str) else (f'{x:.3g}' if isinstance(x, float) else str(x)) for x in r) + '\n')
with open(f'{RES}/ablation9_leaves.tsv', 'w') as o:
    o.write('variant\tkey\tid\tname\tideal27_rel\tbaseline\n')
    for r in leaves: o.write('\t'.join([r[0], r[1], r[2], r[3], f'{r[4]:+.3e}', f'{r[5]:.3e}']) + '\n')
print(open(f'{RES}/ablation9.tsv').read())
