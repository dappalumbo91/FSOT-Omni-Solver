#!/usr/bin/env python3
"""summ4.py: aggregate tier-4 (compiled leaves) block-solve logs (phase2/spice/out/bs4_leafc_*) into per-leaf error tables.
Reporting only; every number comes from score4.sh (circuit V(net)/gain vs golden; GEN vs the engine constant).
Writes phase3/results/leaves4_summary.tsv, leaves4_netlists.tsv and leaves4_convergence.tsv."""
import subprocess, statistics as st, os, glob, re, collections
R = os.path.dirname(os.path.abspath(__file__)); OUT = os.path.join(R, '../../phase2/spice/out'); RES = os.path.join(R, '../results')
KEYS = ['GEN', 'T1', '2_M', '2_P', '2_T', '2_X', '2a_M', '2a_P', '2a_Q', '2a_T', '2a_X']
def score(log, k):
    p = subprocess.run([os.path.join(R, 'score4.sh'), log, k], capture_output=True, text=True).stdout.splitlines()
    return {l.split('\t')[0]: (l.split('\t')[1], l.split('\t')[4]) for l in p[1:] if not l.startswith('summary') and l.split('\t')[2] != 'MISSING'}
rows = collections.OrderedDict(); conv = []
for k in KEYS:
    for v, seeds, temps in (('ideal_ideal', [0], [0, 27, 50]), ('mcC', range(1, 9), [27]), ('mcB', range(1, 5), [27])):
        for s in seeds:
            for t in temps:
                f = f'{OUT}/bs4_leafc_{v}_s{s}_t{t}_{k}.log'
                ok = os.path.exists(f) and any(l.startswith('SOLVER: blocks') for l in open(f))
                why = '' if ok else (open(f).read().strip().splitlines()[-1] if os.path.exists(f) else 'missing')
                m = re.search(r'first (\S+)\)', why); conv.append((k, v, s, t, 'OK' if ok else 'FAIL', m.group(1) if m else ''))
                if not ok: continue
                for gid, (nm, rel) in score(f, k).items():
                    r = rows.setdefault((k, gid), {'name': nm, 'ideal': {}, 'mcC': [], 'mcB': []})
                    x = float(rel)
                    if v == 'ideal_ideal': r['ideal'][t] = x
                    else: r[v].append(x)
def fm(x): return '-' if x is None else f'{x:+.2e}'
def sd(a): return st.pstdev(a) if len(a) > 1 else None
def worst(a): return max(a, key=abs) if a else None
with open(f'{RES}/leaves4_summary.tsv', 'w') as o:
    o.write('netlist\tid\tname\tideal_0C\tideal_27C\tideal_50C\tmcC_n\tmcC_mean\tmcC_sd\tmcC_worst\tmcB_n\tmcB_sd\tmcB_worst\n')
    for (k, gid), r in rows.items():
        c, b = r['mcC'], r['mcB']
        o.write('\t'.join([k, gid, r['name'], fm(r['ideal'].get(0)), fm(r['ideal'].get(27)), fm(r['ideal'].get(50)), str(len(c)), fm(st.mean(c) if c else None), fm(sd(c)), fm(worst(c)), str(len(b)), fm(sd(b)), fm(worst(b))]) + '\n')
with open(f'{RES}/leaves4_netlists.tsv', 'w') as o:
    o.write('netlist\tleaves\tideal27_median_abs\tideal27_worst\tideal27_worst_leaf\tideal_0..50_worst\tmcC_runs_ok\tmcC_median_sd\tmcC_worst\tmcB_runs_ok\tmcB_median_sd\tmcB_worst\n')
    for k in KEYS:
        rs = [(g, r) for (kk, g), r in rows.items() if kk == k]
        i27 = [(abs(r['ideal'][27]), r['name']) for g, r in rs if 27 in r['ideal']]
        iall = [abs(x) for g, r in rs for x in r['ideal'].values()]
        sdc = [sd(r['mcC']) for g, r in rs if sd(r['mcC']) is not None]; sdb = [sd(r['mcB']) for g, r in rs if sd(r['mcB']) is not None]
        wc = [abs(x) for g, r in rs for x in r['mcC']]; wb = [abs(x) for g, r in rs for x in r['mcB']]
        okc = sum(1 for c in conv if c[0] == k and c[1] == 'mcC' and c[4] == 'OK'); okb = sum(1 for c in conv if c[0] == k and c[1] == 'mcB' and c[4] == 'OK')
        w = max(i27) if i27 else (None, '-')
        o.write('\t'.join([k, str(len(rs)), f'{st.median([a for a, _ in i27]):.2e}' if i27 else '-', f'{w[0]:.2e}' if i27 else '-', w[1], f'{max(iall):.2e}' if iall else '-',
                           f'{okc}/8', f'{st.median(sdc):.2e}' if sdc else '-', f'{max(wc):.2e}' if wc else '-', f'{okb}/4', f'{st.median(sdb):.2e}' if sdb else '-', f'{max(wb):.2e}' if wb else '-']) + '\n')
with open(f'{RES}/leaves4_convergence.tsv', 'w') as o:
    o.write('netlist\tvariant\tseed\ttemp_C\tresult\tfailed_block_first_element\n')
    for c in conv: o.write('\t'.join(map(str, c)) + '\n')
print(open(f'{RES}/leaves4_netlists.tsv').read())
