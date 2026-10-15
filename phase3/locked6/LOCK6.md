# LOCK6 — FSOT Omni-Solver Phase 3d (tiny terms carried in the log domain; low-leakage rectifier)

Locked 2026-10-15 before any LOCK6 simulation. Builds on LOCK5 (compiled-leaf recipes and values in phase3/locked5 are used unchanged).
No FSOT rule is new or changed; no locked file is edited (phase3/cpp/netgen5.cpp is a fork of netgen4.cpp; tapc6.cpp is new).

## 6.1 Log-carry of the base-engine tiny taps (Damian, 6:11 PM: "carried, not lost")
The 247 taps that LOCK5 builds as single uncascoded transistors (term t < 1e-4 of I_U = 5 uA; cy2/cy3, sw2/sw3, sv3/sv4, st4/st5 and the T3 tap a3)
are carried as log-domain voltages. For each, tapc6 derives from the locked folds35_targets.tsv (long double, same term formulas as LOCK5)
an integer lift n = floor(-3 - ln t) nepers, so the scaled term t e^n lies in (e^-4, e^-3] (49-92 nA; the LOCK5 headroom band). The circuit:
vx (= V_T ln t, the engine's own tap summer) -> summer + inverter -> vx + n V_T (n from the PTAT V_NEP = 10 V_T bus through R_lcn = R_SUM 10/n)
-> cascoded antilog (I_U in) -> TIA, R_lcT = 1 V/(I_U e^-3) = 4.017 M: v(vlc) = I_U t e^n R_lcT (0.37-1 V), the carried readout;
-> divide-back antilog: input v(vlc)/R_lcT, base at -n V_T (same bus, same R_lcn value) -> uncascoded output, sensed (VSE_/VSB_), injected into
the engine's summing node exactly where LOCK5's tap went. Up- and down-lift use the same E24 realisation of R_lcn, so its E24 error cancels; V_NEP
errors cancel (common bus). n is locked per fold and tap in taps6.tsv (n = 7..36; smallest distance of -3 - ln t from an integer: 0.0094).
Scale factors are derived, never fitted. Grouping several tiny terms into one scaled partial sum would need a common n, leaving the smaller terms
tiny inside that sum, so each term gets its own n.

## 6.2 Bench: uncascoded vs log-carry vs charge integrator (netgen5 mode tapbench, run_tb6.sh, plain ngspice transient)
Per tiny tap of a fold: stimulus base voltage V_T ln t (V_NEP bus, R = R_SUM 10/|ln t|), one input transdiode (I_U), and three readouts from the
same stimulus: (a) the LOCK5 direct uncascoded transistor into a TIA, (b) the LOCK6 log-carry, (c) a charge integrator (uncascoded transistor into
an op-amp integrator, C_int = 1 nF C0G with a 100 G insulation-resistance model assumption, from 0 V at t = 0 for T_int = 1 s; read at T_int/2
and T_int). C_int and T_int are locked in design6_taps_values.tsv.

## 6.3 Rectifier diodes (netgen5 FWR primitive)
LOCK5's ideal 2a_X run failed at the LOGR fed by the Chain_consistency_% rectifier: with 100 k input resistors the signal current is 2.6 nA, the
same as the 1N4148 model's reverse saturation current (I_S 2.52 nA), so the rectifier read 2.65 uV for a 261 uV input. netgen5 builds the two
rectifier diodes as diode-connected 2N3904s (I_S 6.7 fA), a standard low-leakage substitute. Resistor values unchanged (locked5).

## Harness
tools/run_bs6.sh (leafc5 keys, fold:<name>), tools/run_tb6.sh (bench), tools/verify_lock6.sh.

## SHA256 (recorded before simulation)
    0bafb0b56742cd097598c9914c60403331c4e7d354dbf139a6bfe78f215a4e8f  taps6.tsv
    1ebf1a6b0041dd1659373059415646446f2ef36dc83f4e5dabab5620acfd646e  design6_taps_values.tsv
    e388f298e517abc4eceb50f620772e9eb9eee361690bb1f3168f208ff996fb16  ../cpp/tapc6.cpp
    88f5cf7d7957e855b75664c9e851f9e401df74a65fd1194d06636054023cffce  ../cpp/netgen5.cpp
    1812f98a4c09b6dcb9575d30faa2980a578591484e1e6ad1430851875d51a7c9  ../tools/run_bs6.sh
    29bfec9e46cde76e79d78f1f6d0c14295c5cff35d31e528a48df19bf7f1dd024  ../tools/run_tb6.sh
    c6045c41ce4c111a27d3b6ddf5f449f2650ebc065036ec2c554fc0b63b893b78  ../tools/verify_lock6.sh
