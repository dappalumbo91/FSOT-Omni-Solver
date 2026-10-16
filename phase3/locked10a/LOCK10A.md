# LOCK10A (locked10a) — per-cell error budget: isolated-cell benches, closed-form predictions, leaf-level physics attribution

Locked 2026-10-15, before the LOCK10A bench and attribution runs. Purpose (Damian, 8:59 PM, task 1): itemise every per-cell error
contribution on the 7.1 compiler + ratio-256 (LOCK7) base, with closed-form predictions from the device model written before each bench run.
New files: phase3/cpp/cells10.hpp (cell definitions), phase3/cpp/netgen9.cpp (fork of netgen8, unchanged), phase3/cpp/cellpred.cpp
(predictions; output cellpred.tsv, below), tools/run_cell10.sh, tools/cellscore.py, tools/run_phys10.sh, tools/abl10.py.
netgen9 with FSOT_PHYS unset and an existing mode writes the netgen8 netlist byte for byte (checked on GEN before locking).

Mechanisms (FSOT_PHYS names): hi (IKF), rb + ib (base current through RB, incl. the ISE term), ib (base current alone), early (VAF with RC),
gain (ideal op-amp gain 1e6 vs 1e10), round (netlist print rounding: netgen2..8 print ideal resistor values at 6 significant digits, found
while building this bench), vnep (the real PTAT generator vs an ideal 10 kT/q source; leaf level). "none" = all off: what remains is the
I_S(e^x - 1) "-1" term and, for CALU, the reverse B-C current; both are predicted.
Cells: LOGRa/b/p/k (7.1 signal, base-cell, PTAT-referenced and LOCK7 base-channel LOGRs), CALa/b/c (7.1 antilog at absorption 0, +1.5, -3),
CALt (base T1 antilog, 7 uA), CALUa (uncascoded), SUMa/b (noise gain 3.3 / 13), TIAa, VNEP (the base PTAT generator as built).
Leaf level: tools/run_phys10.sh full | none | no_<m> on the 7.1/8.1 leaves (locked9a/v71), ideal 27 C, 10 netlists.

Disclosure: while building netgen9's cell mode, a toolchain smoke test (before cellpred existed) solved LOGRa, CALa, CALUa, SUMb, TIAa and
VNEP with full physics; LOGRa's full value (e = -2.2e-5) was seen. No prediction was changed after that (cellpred was written from the model).

## SHA256
    602bf534aceab08001a0604c32901e2adb6f86959bc1977630d5cca860059647  cellpred.tsv
    eb55d3fac44dc9ed9d9726bd06d946f0546c7b20a386218ad0d43340dd7113c9  ../cpp/cells10.hpp
    b0e8cb4821a75efeead7ec52c5dab9103381349a4c8f1db96f714f787167ae61  ../cpp/netgen9.cpp
    3d796e4b6b26771862f684ec59e63160fb9259960b9881edcd5e5d181a92993c  ../cpp/cellpred.cpp
    ffed8a5ee265163af039fea850337a051b85cab64783bc55c16da4729f43243e  ../tools/run_cell10.sh
    24e5e679df6b67e9f7bf8560c05bd79d9d50dcd23e209ba462c9edb0a1ac6bf9  ../tools/cellscore.py
    f8d2e152dc00d3bc9853f5b683dd3becb6476e90a4cb424b65604a97c8499466  ../tools/run_phys10.sh
    3d37d8221987af16d1273393d56439ebe598a82d3de084fbe47f04680644987d  ../tools/abl10.py
