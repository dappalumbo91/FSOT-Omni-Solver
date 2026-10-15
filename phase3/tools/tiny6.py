#!/usr/bin/env python3
"""tiny6.py: reporting only. Base-engine tiny terms (LOCK6 log-carry vs LOCK5 direct uncascoded) against the engine terms of taps6.tsv.
Per fold and tiny tap (ideal, 27 C, full fold channel solved by blocksolve4):
  carried  = v(vlc<tap>x) / (R_lcT I_U e^n)      LOCK6 scaled readout (log-carried term)
  inj6     = (i(vse) - i(vsb)) / I_U              LOCK6 divided-back current injected into the engine node
  dir5     = (i(vse) - i(vsb)) / I_U              LOCK5 direct uncascoded tap
Bench (tb6/ig6, stimulus V_T ln t with the engine term, 0/27/50 C): carried, divided-back, direct, integrator (I = C V(T)/T) for IB = 0/100 pA/25 fA.
Writes phase3/results/tiny_terms6_fold.tsv, tiny_terms6_bench.tsv and prints a per-tap summary."""
import os, re, math, statistics as st, collections
R = os.path.dirname(os.path.abspath(__file__)); OUT = os.path.join(R, '../../phase2/spice/out'); RES = os.path.join(R, '../results'); L6 = os.path.join(R, '../locked6')
IU = 5e-6
RlcT = float([l.split('\t')[1] for l in open(f'{L6}/design6_taps_values.tsv') if l.startswith('R_lcT\t')][0])
taps = [l.rstrip('\n').split('\t') for l in open(f'{L6}/taps6.tsv')][1:]
def rd(f):
    d = {}
    if not os.path.exists(f): return None
    for l in open(f):
        m = re.match(r'(\S+) = (\S+)', l.strip())
        if m:
            try: d[m.group(1).lower()] = float(m.group(2))
            except ValueError: pass
    return d
def rel(a, b): return None if a is None else (a - b) / b
fm = lambda x: '-' if x is None else f'{x:+.3e}'
rows = []
for fold, tap, term, lnt, n, scaled, marg, Rn in taps:
    t = float(term); n = int(n); f6 = rd(f'{OUT}/bs6_fold_ideal_ideal_s0_t27_{fold}.log'); f5 = rd(f'{OUT}/bs5_fold_ideal_ideal_s0_t27_{fold}.log')
    k = tap + 'x'
    car = f6.get(f'v(vlc{k})') / (RlcT * IU * math.exp(n)) if f6 and f'v(vlc{k})' in f6 else None
    inj = (f6[f'i(vse_{k})'] - f6[f'i(vsb_{k})']) / IU if f6 and f'i(vse_{k})' in f6 else None
    dr5 = (f5[f'i(vse_{k})'] - f5[f'i(vsb_{k})']) / IU if f5 and f'i(vse_{k})' in f5 else None
    rows.append((fold, tap, t, n, car, inj, dr5))
os.makedirs(RES, exist_ok=True)
with open(f'{RES}/tiny_terms6_fold.tsv', 'w') as o:
    o.write('fold\ttap\tengine_term\tn\tcarried_LOCK6\trel\tinjected_LOCK6\trel\tdirect_LOCK5\trel\n')
    for fold, tap, t, n, car, inj, dr5 in rows:
        o.write('\t'.join([fold, tap, f'{t:.6e}', str(n), fm(car), fm(rel(car, t)), fm(inj), fm(rel(inj, t)), fm(dr5), fm(rel(dr5, t))]) + '\n')
by = collections.defaultdict(list)
for r in rows: by[r[1]].append(r)
print('tap\tn_folds\tterm_range\tcarried_rel(median,worst)\tinjected_rel(median,worst)\tdirect_LOCK5_rel(median,worst)')
def mw(a):
    a = [x for x in a if x is not None]
    return '-' if not a else f'{st.median(a):+.2e},{max(a, key=abs):+.2e}'
for tap, rs in sorted(by.items()):
    print(f"{tap}\t{len(rs)}\t{min(r[2] for r in rs):.2e}..{max(r[2] for r in rs):.2e}\t{mw([rel(r[4], r[2]) for r in rs])}\t{mw([rel(r[5], r[2]) for r in rs])}\t{mw([rel(r[6], r[2]) for r in rs])}")
# bench
with open(f'{RES}/tiny_terms6_bench.tsv', 'w') as o:
    o.write('fold\ttemp_C\ttap\tengine_term\tcarried_rel\tdivided_back_rel\tdirect_rel\tintegrator_rel_IB0\tintegrator_rel_IB100pA\tintegrator_rel_IB25fA\thold_droop_1s_IB0\n')
    bsum = collections.defaultdict(lambda: collections.defaultdict(list))
    for fold in sorted(set(r[0] for r in rows)):
        for T in (0, 27, 50):
            b = rd(f'{OUT}/tb6_ideal_ideal_t{T}_{fold}.log'); ig = f'{OUT}/ig6_ideal_ideal_t{T}_{fold}.tsv'
            if not b: continue
            igd = collections.defaultdict(dict)
            if os.path.exists(ig):
                for l in list(open(ig))[1:]:
                    c = l.split('\t')
                    try: igd[c[0]][c[2]] = (float(c[4]), float(c[5]), float(c[6]))
                    except ValueError: pass
            for fo, tap, term, lnt, n, *_ in taps:
                if fo != fold: continue
                t = float(term); n = int(n); I = IU * t
                car = b.get(f'c_{tap}'); car = car / (RlcT * IU * math.exp(n)) / t - 1 if car is not None else None
                x = b.get(f'x_{tap}'); x = x / I - 1 if x is not None else None
                u = b.get(f'u_{tap}'); u = u / I - 1 if u is not None else None
                ir = [igd[tap][k][0] * 1e-9 / 1 / I - 1 if k in igd[tap] else None for k in ('0', '100e-12', '25e-15')]
                dr = (igd[tap]['0'][1] - igd[tap]['0'][0]) / igd[tap]['0'][0] if '0' in igd[tap] else None
                o.write('\t'.join([fold, str(T), tap, f'{t:.6e}', fm(car), fm(x), fm(u)] + [fm(z) for z in ir] + [fm(dr)]) + '\n')
                for nm, z in zip(('carried', 'divided_back', 'direct', 'integ_IB0', 'integ_IB100pA', 'integ_IB25fA'), (car, x, u, *ir)):
                    if z is not None: bsum[tap][nm].append(z)
print('\nbench (all folds, 0/27/50 C): tap\t' + '\t'.join(('carried', 'divided_back', 'direct', 'integ_IB0', 'integ_IB100pA', 'integ_IB25fA')) + '  (median,worst rel)')
for tap in sorted(bsum): print(tap + '\t' + '\t'.join(mw(bsum[tap][k]) for k in ('carried', 'divided_back', 'direct', 'integ_IB0', 'integ_IB100pA', 'integ_IB25fA')))
