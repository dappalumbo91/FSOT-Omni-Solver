#!/usr/bin/env python3
"""regress_ci.py: the approved CI regression check (rule in phase3/locked_ci/LOCK.md).
For every quantity CI scores at 27 C with ideal parts (each compiled leaf vs golden, each base-engine fold S channel vs the engine S, each tiny
term vs its engine term), CI fails if |error| > 2 x max(|baseline error|, 1e-6), the baseline being phase3/locked_ci/baseline_errors.tsv.
  regress_ci.py check [baseline.tsv]   -> prints a table, exit 1 on any violation or on a baseline row with no current value
  regress_ci.py emit  > baseline.tsv   -> writes the current errors in baseline format (used only when a lock is (re)baselined)
Runs scored (env FSOT_CI_RUNS, default the reproduce5.sh set): '<tag>:<key>' pairs, tag bs5/bs6/bs7c..., log phase2/spice/out/<tag>_leafc_ideal_ideal_s0_t27_<key>.log,
scored by score5.sh (bs5/bs6) or score7.sh with the run's locked dir (FSOT_CI_LOCKDIR_<tag>). Folds: FSOT_CI_FOLDS (tag:fold), S = v(vsx)/2.
Tiny terms (folds of FSOT_CI_FOLDS, bench fold FSOT_CI_BENCH = Cosmology): FSOT_CI_TINY = python script that writes the tiny-term tables (default tiny6.py -> results/tiny_terms6_*.tsv)."""
import os, sys, re, csv, subprocess
R = os.path.dirname(os.path.abspath(__file__)); OUT = os.path.join(R, '../../phase2/spice/out'); RES = os.path.join(R, '../results')
FLOOR, FACTOR = 1e-6, 2.0
RUNS = os.environ.get('FSOT_CI_RUNS', ' '.join([f'bs5:{k}' for k in 'GEN T1 2_M 2_P 2_T 2_X 2a_M 2a_P 2a_Q 2a_T'.split()] + ['bs6:T1', 'bs6:2a_X'])).split()
FOLDS = os.environ.get('FSOT_CI_FOLDS', 'bs6:Cosmology bs6:Chemistry bs6:Quantum_Mechanics').split()
TINY = os.environ.get('FSOT_CI_TINY', 'tiny6'); BENCH = os.environ.get('FSOT_CI_BENCH', 'Cosmology').split()
SDIR = os.environ.get('FSOT_CI_STARGETS', os.path.join(R, '../../phase2/locked/folds35_targets.tsv'))
def current():
    cur = {}
    for rk in RUNS:
        tag, k = rk.split(':'); log = f'{OUT}/{tag}_leafc_ideal_ideal_s0_t27_{k}.log'
        if not os.path.exists(log) or not any(l.startswith('SOLVER: blocks') for l in open(log)): continue
        ld = os.environ.get(f'FSOT_CI_LOCKDIR_{tag}')
        cmd = [os.path.join(R, 'score7.sh'), log, k, ld] if ld else [os.path.join(R, 'score5.sh'), log, k]
        for l in subprocess.run(cmd, capture_output=True, text=True).stdout.splitlines()[1:]:
            c = l.split('\t')
            if c[0] == 'summary' or c[2] == 'MISSING': continue
            cur[f'leaf|{tag}|{k}|{c[0]}'] = (c[1], float(c[4]))
    S = {r['domain']: float(r['S']) for r in csv.DictReader(open(SDIR), delimiter='\t')}
    for tf in FOLDS:
        tag, f = tf.split(':'); log = f'{OUT}/{tag}_fold_ideal_ideal_s0_t27_{f}.log'
        if not os.path.exists(log): continue
        for l in open(log):
            if l.startswith('v(vsx) = '): cur[f'S|{tag}|{f}|S'] = (f'S({f})', (float(l.split()[2]) / 2 - S[f]) / S[f])
    if TINY == 'tiny6':
        for r in csv.reader(open(f'{RES}/tiny_terms6_fold.tsv'), delimiter='\t'):
            if r[0] == 'fold' or f'bs6:{r[0]}' not in FOLDS: continue
            for col, nm in ((5, 'carried'), (7, 'injected')):
                if r[col] != '-': cur[f'tiny|fold|{r[0]}|{r[1]}:{nm}'] = (f'{r[1]} {nm}', float(r[col]))
        rows = list(csv.reader(open(f'{RES}/tiny_terms6_bench.tsv'), delimiter='\t'))
        for r in rows[1:]:
            if r[1] != '27' or r[0] not in BENCH: continue
            for j in range(4, len(rows[0])):
                if r[j] != '-' and not rows[0][j].startswith('hold'): cur[f'tiny|bench|{r[0]}|{r[2]}:{rows[0][j]}'] = (f'{r[2]} {rows[0][j]}', float(r[j]))
    return cur
def main():
    mode = sys.argv[1] if len(sys.argv) > 1 else 'check'
    if mode == 'emit':
        print('key\tname\tbaseline_rel_error')
        for k, (nm, e) in sorted(current().items()): print(f'{k}\t{nm}\t{e:.6e}')
        return 0
    base = os.path.join(R, '../locked_ci/baseline_errors.tsv') if len(sys.argv) < 3 else sys.argv[2]
    cur = current(); bad = 0; n = 0; worst = (0, '')
    S = os.environ.get('GITHUB_STEP_SUMMARY', '/dev/null')
    with open(S, 'a') as so:
        so.write('\n### Regression check (|err| <= 2 x max(|baseline|, 1e-6), ideal 27 C)\n\n| key | baseline | current | limit | result |\n|---|---|---|---|---|\n')
        for r in csv.DictReader(open(base), delimiter='\t'):
            b = float(r['baseline_rel_error']); lim = FACTOR * max(abs(b), FLOOR); n += 1
            if r['key'] not in cur: bad += 1; so.write(f"| {r['key']} | {b:.3e} | MISSING | {lim:.3e} | FAIL |\n"); print('MISSING', r['key']); continue
            e = cur[r['key']][1]; ratio = abs(e) / lim; worst = max(worst, (ratio, r['key']))
            if abs(e) > lim: bad += 1; so.write(f"| {r['key']} | {b:.3e} | {e:.3e} | {lim:.3e} | FAIL |\n"); print(f"FAIL {r['key']} baseline {b:.3e} current {e:.3e} limit {lim:.3e}")
        so.write(f'\n{n} quantities checked, {bad} violations, largest |err|/limit = {worst[0]:.3f} ({worst[1]})\n')
    print(f'REGRESS_{"OK" if bad == 0 else "FAIL"} checked={n} violations={bad} max_err_over_limit={worst[0]:.3f} ({worst[1]})')
    return 1 if bad else 0
sys.exit(main())
