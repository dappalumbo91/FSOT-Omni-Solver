# LOCK10A.2 — cascoded antilog cells: "gain off" at 1e9 (amendment, before those runs)

Locked 2026-10-15 ~22:20 ET. First-pass scoring of the LOCK10A bench (results/cells10/budget_pass1.tsv, kept): LOGR, CALU, SUM and VNEP
cells match cellpred.tsv to <=3e-9 per mechanism; the cascoded CAL cells match in "full" (gain 1e6) to 3e-8 but not in the runs with
op-amp gain 1e10 (none, only_hi, ...: deviations up to 8e-6). Diagnostic (scratch, CALa none with only the E gain changed):
e = -1.82e-7 (1e8), -1.37e-7 (1e9), +2.30e-6 (1e10), -2.21e-5 (1e11), -3.62e-4 (1e12); predicted -1.29e-7 (+1.2e-9 at 1e9).
So 1e10 is past ngspice's conditioning limit for this block. The CAL cells are rerun with tools/run_cell10c.sh (1e9 for gain off;
everything else identical; the gain prediction at 1e9 differs from 1e10 by <2e-9). LOCK10A.1 seeding also covers the early runs, seeded
from a converged run with the same internal nodes (RC present).

## SHA256
    f0f046414f83fa4b831eec0b91bd4e8263cd3a792dff5e8a64d87a45c76bbf2b  ../tools/run_cell10c.sh
