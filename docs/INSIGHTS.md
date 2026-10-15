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

## 7. Monte Carlo (class C = 0.02 %, 5 ppm/K, Vos 2 µV)
- Only 17 of 160 fold runs converged in ngspice (Atomic 3/40, HEP 4/40, QM 4/40, Fluid 6/40). The 40 Ω_b h² runs converged 0/40.
- In the converged runs:
  - S spread is about 1e-3 (Fluid: sd 3.5e-3, worst 5.0e-3).
  - The analog D before rounding is within ±0.006 of the integer, so D rounds correctly in 17/17.
- Class B was not run: it has the same convergence failure and larger draws.
- Convergence finding:
  - The E24/MC netlists solve **only** from their own exact solution.
  - Any perturbation, even a uniform 0.1 µV offset on every op-amp, defeats Newton, gmin stepping, source stepping and transient-op.
  - So does a draw homotopy (scale 0 → 0.25 already fails), rail-limited ideal op-amps, and dropping the nodesets of railed nodes.
  - The suspected cause is the unregulated T3 cascodes (section 4) together with 1e6-gain sources. This is open.

## 8. Vendor macro-models in ngspice
- The TI OPAx187 (Green-Williams-Lis architecture) solves alone and in a two-inverter test. It fails as soon as one transdiode loop and one inverter share a netlist. The failure appears as a "singular matrix" at internal nodes; the clamp/limit B-sources and self-controlled VSWITCH ideal diodes are the suspects.
- A full-state nodeset from per-instance jigs did not help, nor did a transient with `uic` from the full state, nor softer clamp switches. The OPAx277 (same architecture) behaves the same.
- What was done instead (`phase2/spice/vendor_extract.sh`): every vendor instance is solved alone at its operating point, and its measured V+ − V− and I_B are injected into the full netlist.
  - For the constants netlist both Vos and I_B converge: 1/α = 137.1081 at 27 °C (+5.2e-4, versus +4.8e-4 E24 without vendor errors).
  - For the fold and Ω_b h² netlists only I_B converges.
