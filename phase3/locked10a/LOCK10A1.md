# LOCK10A.1 — solver seeding for stalled cascode blocks (amendment, before those runs)

Locked 2026-10-15 ~21:45 ET. In the first LOCK10A bench pass, 11 of 104 runs stalled (>10 min) in blocksolve4's cascode output block
(CALa/CALb only_early, CALc none/only_hi/only_ib/only_round/full, CALt none/only_ib/only_round, LOGRk only_rbib): the sub-uA VDMOS
cascode under a 1e10 servo makes ngspice fall into the pseudo-transient op. Their results were not seen (no log written). They are rerun
with tools/run_cell10b.sh: identical netlist plus .nodeset lines from a converged run of the same cell. blocksolve4's verify is
unchanged. Netlists, predictions (cellpred.tsv) and the scorer are unchanged.

## SHA256
    18f2ff098c34c569c95e9c55ddf947d4487da9927b38c705e7bf53bd643defb0  ../tools/run_cell10b.sh
