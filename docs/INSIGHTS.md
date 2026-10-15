# FSOT Omni-Solver Phase 2: insights (with numbers)

All numbers come from `phase2/results/` (ngspice-44.2). The targets are the C++20 engine values (FSOT-2.1-Cpp @3466e8d), locked before simulation in `phase2/locked/` (Amendments 1–4 are all listed in LOCK.md). "rel" means (circuit − engine)/engine.

## 1. Accuracy is set by how many nepers of *constants* a term carries
- Every neper constant is a fraction of V_NEP = 10·V_T taken from resistor ratios. A relative error ε in V_NEP or in a ratio becomes an error of ε × (constant in nepers) in that term's logarithm.
- T1 carries about 4.8 Np of constants; FSOT itself supplies ρ + B_in·δψ + C_F·P_var ≈ 2.06 Np of that. With ideal op-amps and exact resistor values, T1 is off by +2.0e-4 to +2.8e-4 across all 35 folds. That residual is the high-injection (IKF = 66.78 mA) log-conformity of the 2N3904 Gummel–Poon card.
- The two-cell PTAT (64:4 µA and 32:2 µA, V_NEP = (10/ln16)(2ΔV_B − ΔV_A)) cancels current-proportional errors to first order:
  - V_NEP is within 1.0e-5 / 7.1e-6 / 5.3e-6 of 10·kT/q at 0 / 27 / 50 °C. Before Amendment 1 the error was 2.9e-4.
  - 1/α = 137.0628 / 137.0615 / 137.0607 at 0 / 27 / 50 °C (+1.94e-4 / +1.85e-4 / +1.79e-4). Phase 1 swung from 184 to 110 over the same range.

## 2. 1 − S amplifies; Ω_b h² is the most sensitive output
- 1 − S_chem = 0.0447, so an error δ in S_chem becomes −δ·S/(1−S), about −21δ, in 1 − S_chem.
- The current-mode 1 − S node removes the Phase-1 voltage-subtraction loss.
  - With ideal op-amps and exact values, Ω_b h² = 0.022395, −2.95e-3. That is inside the 6.7e-3 bar; Phase 1 nominal was +0.89% and failed it.
  - With E24-realised resistors it is −8.7e-3, which **fails** the bar.
  - Adding the OPA187 input bias current (vendor model, about 100 pA per input) gives −1.2e-2, which **fails**.
- The bar needs about 3e-4 on S_chem. The E24 realisation (+3.5e-4) or 100 pA of bias current into µA-level log nodes (+1.7e-4) each uses up that whole budget.

## 3. Bias current, not offset, dominates the precision op-amps
- Each OPA187 was solved alone at its own operating point (inputs, output and load current taken from the full ideal solve).
  - Effective input error |V+ − V−| ≤ 0.80 µV everywhere (154 instances per channel).
  - I_B+ ≈ 100 pA.
- Injecting I_B alone moves S_chem from +3.49e-4 to +5.20e-4 and S_HEP from +3.58e-4 to +5.79e-4.
- I_B / I_signal ≈ 100 pA / 1–5 µA = 2–10e-5 per log block, summed over about 10 blocks per term.
- Fixes: a lower-I_B part (the OPA140 JFET class, about 1 pA) or larger unit currents (bounded above by IKF high injection). That is an amendment for Phase 3.

## 4. T3 is resolvable only in the log domain
- T3/S ≤ 3.2e-17. Its linear current is about 1e-22 A, so four or five of the T3 antilog cascode outputs carry no regulable current: the ideal op-amp outputs run to −1e5 V; a real part sits at its rail.
- ln T3 is still computed to 3–6e-4 Np in the ideal netlists, and to 4.4–5.9e-3 Np with E24 values.
- The T3 contribution to S is exactly as negligible as the engine says. This is reported, not dropped.

## 5. Small terms ride on offsets
- α·h/N and the growth exponent α(1−h/N)γ/φ are tens of µV on the junction bus, the same size as the class-B Vos (10 µV). HEP (hits = 1) is the only fold where h/N ≠ 0.
- cos((ψ+δψ)/η) sits near its extremum (y = x − π is small), so the series to y⁶ is more than enough. The error is in the constants, not the series.

## 6. Temperature: the fold channels have a residual TC; the oven pins it
- With ideal op-amps, S_chem rel is +1.60e-4 at 27 °C, −4.0e-6 at 50 °C, −1.81e-4 at 58.9 °C and −2.17e-4 at 60.2 °C.
- That is a channel TC of about −1.1e-5 /K (Atomic −1.6e-5 /K, Cosmology −0.6e-5 /K), even though V_NEP itself is PTAT-exact to 1e-5.
- The residual comes from blocks referenced to VR (the 1 V/rad angle bus, the R_pin unit currents), which are temperature-flat by design. They meet PTAT neper constants inside the same exponent.
- Oven (Amendment 3; ideal-op PI loop with the TIP31 heater):
  - The sensor die regulates at 60.4219–60.4220 °C for ambient 0 / 27 / 50 °C (0.15 mK spread).
  - With the assumed sensor-to-core gradient of 1 K/W, the core sits at 58.91 / 59.59 / 60.16 °C, a 1.25 K spread. Times the channel TC, that is ±7e-6 in S.
  - Warm-up overshoot ≤ 1.5 K; settles within 0.1 K in 10–74 s.
- Below about 8 °C *without* the oven, the ideal-op netlist has no ngspice solution (the temperature walk 27 → 8 °C converges, 7 °C fails). This is tied to the idle T3 outputs (section 4). The oven makes cold ambient irrelevant.

## 7. Monte Carlo: every class converges (block-sequential solve, LOCK Amendment 6)
40 seeds per class. Each seed is the Ω_b h² netlist (Chemistry + Cosmology channels) plus four folds. Converged: class C **200/200**, class B **199/200**.

| quantity | class C (0.02 %, 5 ppm/K, Vos 2 µV): mean / sd / worst \|rel\| | class B (0.1 %, 25 ppm/K, Vos 10 µV): mean / sd / worst |
|---|---|---|
| S_chem | +3.7e-4 / 1.5e-3 / 4.5e-3 | (39 seeds) see mc_omb_mcB_t27.tsv |
| S_cosm | +4.0e-4 / 3.7e-3 / 1.1e-2 | |
| 1 − S_chem | −6.7e-3 / 3.3e-2 / 9.6e-2 | |
| Ω_b h² (bar 6.7e-3) | −7.7e-3 / 3.1e-2 / 8.9e-2; pass 9/40 | pass 0/39 |
| S(Atomic) | +3.7e-4 / 1.4e-3 / 4.2e-3 | +1.4e-3 / 1.3e-2 / 3.5e-2 |
| S(HEP) | +3.7e-4 / 1.5e-3 / 4.5e-3 | +1.4e-3 / 1.4e-2 / 3.7e-2 |
| S(QM) | +3.0e-4 / 1.5e-3 / 4.3e-3 | +1.4e-3 / 1.4e-2 / 3.6e-2 |
| S(Fluid) | +1.5e-3 / 3.3e-3 / 8.9e-3 | +2.0e-3 / 3.0e-2 / 6.4e-2 |
| ln T3 (Np, log domain) | sd 2.1e-2, worst 4.6e-2 | sd 0.19, worst 0.46 |
| D (rounded) | exact 160/160 | exact 160/160 |

- The one class-B failure (Ω_b h² seed 39) is the circuit leaving its domain, not the solver. That draw puts 1 − S_chem far enough off (×~8 sensitivity, section 2) that the Ω_b h² antilog exponent becomes v_x = +56.9 mV; nominal is −38.9 mV. With v_x > 0 the output cascode needs its source above its drain, which sits at a virtual ground. Across the 40 class-B draws v_x spans −47.7 … +56.9 mV.
- Real-part accuracy is deprioritised; see section 10 for per-part error coefficients.

## 8. Vendor macro-models in ngspice
- The TI OPAx187 (Green-Williams-Lis architecture) solves alone and in a two-inverter test. It fails as soon as one transdiode loop and one inverter share a netlist. The failure appears as a "singular matrix" at internal nodes; the clamp/limit B-sources and self-controlled VSWITCH ideal diodes are the suspects.
- A full-state nodeset from per-instance jigs did not help, nor did a transient with `uic` from the full state, nor softer clamp switches. The OPAx277 (same architecture) behaves the same.
- What was done instead (`phase2/spice/vendor_extract.sh`): every vendor instance is solved alone at its operating point, and its measured V+ − V− and I_B are injected into the full netlist.
  - For the constants netlist both Vos and I_B converge: 1/α = 137.1081 at 27 °C (+5.2e-4, versus +4.8e-4 E24 without vendor errors).
  - For the fold and Ω_b h² netlists only I_B converges.

## 9. Why ngspice failed, and why it now converges
- The full netlists (about 930 nodes, pA transdiodes inside gain-1e6 loops) are badly conditioned. With ngspice's default KLU solver, plain Newton gave NaN by iteration 2 every time (first at the ladder + LOGR_Db + chaos-TIA stage). Gmin and source stepping landed only by luck, so convergence looked chaotic: removing one cascode flipped the outcome (Amendment 5, withdrawn).
- Switching to `.options sparse` (Sparse 1.3, with better pivoting) alone made the nominal plain run converge where KLU gave NaN. MC draws still failed on the full matrix.
- The fix is **exact tearing** at stiff nodes: ideal op-amp, comparator and supply outputs (`cpp/blocksolve.cpp`). The Ω_b h² netlist splits into 218 blocks, a fold into 108; the largest block has about 113 elements. A fold solves in about 1 s, and 35 folds take 5 s on 8 cores. Agreement with the full netlist is 4.5e-10 in S.
- Pitfall found on the way: copying interface voltages with ngspice's default 6 digits gives a 4e-5 error in S, because a 1 µV error in an exponent is 1e-6/V_T = 4e-5 after the antilog. Interfaces must carry at least 12 digits (`set numdgt=15`).
- The T3 hypothesis, tested: the T3 antilog output (a3), and the highest series taps (cy2, cy3, sv4, sw3, st5), are physically **cut off**. Their design currents (about 1e-17 … 1e-22 A) are below any junction leakage, so their cascode op-amps drive the 2N7000 gates to about −1e6 V (ideal sources) or to the rail. They were *not* the cause of the non-convergence: block solves converge with them in place and are gmin-independent. They do mean that T3 exists in this circuit only as its log-domain voltage v_x3. As a current it is about 0.01 electron/s. That is the FSOT pressure point: a term 1e-17 below the sum cannot be an additive physical signal, only a logarithmic one.
