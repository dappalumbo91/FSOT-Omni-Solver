# LOCK (CI regression baseline) — phase3/locked_ci

## Approval and rule
Approved by Damian (relayed 2026-10-15, 7:21 PM ET): a 2x regression check in the CI phase3-sim job. Exact rule:

> For every quantity CI scores — each compiled leaf, each tiny term and each S channel — take the ideal-parts error versus golden (S channels:
> versus the engine S; tiny terms: versus the engine term) at 27 C. CI fails if |error| > 2 x max(|locked baseline error|, 1e-6).
> The 1e-6 (relative) floor is part of the approved rule: it keeps leaves whose baseline is near zero (leaf-block exactness ~1e-6) from failing on noise.
> A baseline row with no current value (run not converged, leaf missing) also fails.
> The baseline changes only when a new lock improves results; the new baseline is committed with that lock (the refinements 1-3 lock rebaselines
> in its own commit).

This is a regression check against the project's own locked results, not an accuracy threshold against physics: nothing else in CI is gated on
accuracy.

## Baseline (this file set)
baseline_errors.tsv: 458 quantities from the runs CI reproduces (tools/reproduce5.sh), computed by tools/regress_ci.py emit from the same ideal
27 C logs that produced the committed results (results/leaves5_summary.tsv, leaves6_summary.tsv, tiny_terms6_fold.tsv, tiny_terms6_bench.tsv):
- leaf|bs5|<key>|<gid>: LOCK5 compiled leaves, keys GEN T1 2_M 2_P 2_T 2_X 2a_M 2a_P 2a_Q 2a_T (313); leaf|bs6|T1, leaf|bs6|2a_X: LOCK6 (58);
- S|bs6|<fold>|S: base-engine S channel v(vsx)/2 vs the engine S, folds Cosmology, Chemistry, Quantum_Mechanics (3);
- tiny|fold|<fold>|<tap>:carried/injected (42) and tiny|bench|Cosmology|<tap>:<column> at 27 C (42; carried, divided-back, direct, integrator).
26 rows have |baseline| < 1e-6 and are checked at 2e-6. Key format: kind|run|netlist-or-fold|id.
Checker: tools/regress_ci.py check (CI step "Regression check (2x locked baseline)", after the reproduction; sha256 of this lock verified first).

## SHA256
    19ec048fd06f603c8144b9e0d0efff729a2d3c10915ccda31c63796f4a00eb02  baseline_errors.tsv
    55ef69d8280f54a6ed46c6e6240af0d86bedb3b3db4efa890ca8c3deb4a4c403  ../tools/regress_ci.py

## Self-test (local, before commit)
A deliberately degraded run (LOCK5 2_X, ideal 27 C, with the first resistor P_zT1xf_0 raised 1% in a scratch copy of locked5) fails:
`FAIL leaf|bs5|2_X|sec.wave2.4 baseline -8.807e-06 current 1.241e-03 limit 1.761e-05`, `REGRESS_FAIL checked=458 violations=1`;
with the committed log restored: `REGRESS_OK checked=458 violations=0 max_err_over_limit=0.500`.
