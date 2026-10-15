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

## 10. Per-part error coefficients (input to the worst-to-best datasheet plan)
These come from `spice/sens.sh` (16 seeds per row). One draw class is active at a time, on the E24 design with ideal op-amps, at 27 °C. The table gives the sd of the relative error of each output; results/sensitivities.tsv has the raw numbers.

| draw (class-B magnitude) | S(Atomic) | S(Fluid) | 1 − S_chem | Ω_b h² | coefficient, S(Atomic) | coefficient, Ω_b h² |
|---|---|---|---|---|---|---|
| resistor tolerance ±0.1 % (uniform) | 4.6e-3 | 8.5e-3 | 1.0e-1 | 9.3e-2 | 4.6 per unit tol | 93 per unit tol |
| op-amp Vos σ = 10 µV | 2.2e-3 | 7.2e-3 | 5.5e-2 | 5.3e-2 | 2.2e-4 per µV | 5.3e-3 per µV |
| transistor ΔV_BE (IS mismatch) σ = 100 µV | 1.2e-2 | 2.2e-2 | 3.4e-1 | 3.1e-1 | 1.2e-4 per µV | 3.1e-3 per µV |
| VREF ±0.1 % | 6.6e-4 | 6.6e-4 | 6.5e-4 | 6.5e-4 | 0.66 per unit | 0.65 per unit |
| β ±30 % | 1.1e-5 | 1.5e-5 | 2.3e-4 | 2.0e-4 | 3.6e-5 per unit | 6.7e-4 per unit |
| V_A ±20 % | 9e-8 | <1e-8 | 1.9e-6 | 1.8e-6 | negligible (cascodes work) | |
| CD4053 R_on 75–175 Ω | <3e-7 | | | | negligible | |
| resistor TC ±25 ppm/K at 27 °C | 0 by construction (zero at t_nom); see §6 for drift | | | | | |

- **The ranking is ΔV_BE ≫ resistor ratio > Vos ≫ VREF ≫ β, V_A, R_on.** Transistor matching, not resistors, is the first lever. Even σ = 100 µV (a good matched pair) costs 1.2e-2 on S and 0.31 on Ω_b h². This is why class C (σ = 10 µV, monolithic pairs) is the first viable class.
- Ω_b h² carries about 20× the S coefficients, from the 1 − S amplification (§2).
- VREF enters S almost one for one (0.66), and 1/α not at all at first order (ratio design).

## 11. Base engine: stability, start-up, noise, temperature, e³ ceiling
- **Loop stability.** `spice/loopgain.sh` measures every op-amp loop with an OPA187-class dynamic model (Amendment 7): 154 loops per fold.
  - As locked before Amendment 8, every LOGR reference-side loop (B) was unstable, with phase margin −75 to −158°. Two integrators coincide (the op-amp, and C_B with R_r), and the µA cascode adds a pole.
  - Fixes (Amendments 8–11, compensation only, DC untouched): C_B = 10 µF + 330 Ω zero, and an integrating cascode servo (100 k / 100 pF).
  - After the fixes, the minimum phase margin is **57.6°** (LOGR_y, which has the lowest reference current, 0.15 µA). There are no loops under 45° in Chemistry, Atomic, Cosmology or Fluid. The rest are 68–99°.
  - Not measurable: the cut-off T3 output (a3) and the cut-off series-tap cascodes (sw3, sv4, st5, and cy2/cy3 in Atomic). These loops are open by physics (§9). The a1/g cascodes in Atomic/Cosmology need the AC operating point fixed (they are 92° in Chemistry).
- **Start-up.** A block-sequential transient (exact tearing in time): supplies ramp over 0–1 ms, VREF over 1–3 ms, and the run lasts 400 ms. No latch-up. All outputs settle to within 1e-5: vnep at 21 ms, the log blocks at 20–70 ms, S at 93 ms, T1 at 139 ms. Final S matches the DC value to 1.8e-6, and the residual is still converging. Before compensation, the PTAT and log blocks oscillated rail to rail.
- **Noise** (OPA187 e_n = 20 nV/√Hz plus all resistor, BJT and MOSFET noise; switches resolved, Amendment 12):

  | output | 0.1–10 Hz rms | relative | 0.1 Hz–10 kHz rms | relative |
  |---|---|---|---|---|
  | S(Chemistry) | 63 µV | 3.3e-5 | 9.6 mV | 5.0e-3 |
  | S(Cosmology) | 76 µV | 7.5e-5 | 9.5 mV | 9.4e-3 |

  The largest contributors are op-amp voltage noise in the LOGR B loops (PTAT B, y, w, a5) and in the st1 summer. The wideband figure says S must be read through a ≤10 Hz filter or averaged; within 0.1–10 Hz noise sits two decades below the ideal-circuit error.
- **Temperature, all 35 folds converge at every point** (block solve; before, they failed below 8 °C). Ideal components:

  | T (°C) | S rel mean | S rel range | D | Ω_b h² rel | 1/α rel |
  |---|---|---|---|---|---|
  | 0 | +2.3e-5 | −3.9e-4…+2.2e-4 | 35/35 | −3.6e-3 | +1.94e-4 |
  | 27 | −2.4e-6 | −4.0e-4…+1.9e-4 | 35/35 | −2.9e-3 | +1.85e-4 |
  | 50 | −1.4e-4 | −4.9e-4…+3.7e-5 | 35/35 | +5.2e-4 | +1.79e-4 |
  | oven core, ambient 0 / 27 / 50 °C (58.91 / 59.59 / 60.16 °C) | −2.92 / −3.08 / −3.22e-4 | | 35/35 | +4.3 / +4.7 / +5.0e-3 | +1.770 / +1.769 / +1.768e-4 |

  - Without the oven, mean S moves 1.6e-4 over 0–50 °C. With the oven it moves 3e-5 over the same ambient range.
  - The temperature-tracking angle bus (priority 3 in the earlier list) was not built. It is deprioritised with real-part accuracy.
- **e³ trimmed ceiling.**
  - With ideal components, 1/α is +1.85e-4 high at 27 °C. Each e¹ stage reads high by 2.6–3.8e-5 (ve1 0.4349417 V against 0.4349250 V exact; ve2 against ve1·e/4), from transistor ISE/Early residuals. The rest comes through the e⁻¹ and −1 terms.
  - A one-resistor trim at 27 °C (on R_e3c or R_aiT) removes the static part. What remains is the temperature drift of the same residual: 1/α moves −9.3e-6 / +5.8e-6 over 0 / 50 °C, and 2e-7 across the oven's 58.9–60.2 °C.
  - So the trimmed e³ ceiling is about 1e-5 without the oven and about 2e-7 with it, before part tolerances. Per §10, ΔV_BE and Vos then set the floor (VREF cancels in 1/α).

## 12. Leaves, tier 1: the circuit solves the closed forms fed by S_cosm, S_quant, S_chem (Phase 3)
- **Map.** The full map is in docs/LEAVES.md: 356 rows parsed from the engine's `closed_forms_core.gen.inc` by `phase3/cpp/leafmap`.
  - 29 literal.
  - 14 tier 1 (fed by S).
  - 111 tier 2 (derived constants).
  - 202 tier 2a (base constants only).
- **Tier 1 is affine.** Every tier-1 form is `b + k·x` with x one circuit signal: |S_cosm|, S_quant, 1−S_chem, 1−S_quant, or |S_cosm|·S_quant.
  - Each form is one inverting summer. The only extra block is σ_8's product, built with the Ω_b h² LOGR–SUM–CAL–TIA topology.
  - Values were locked before simulating (phase3/locked/LOCK3.md, with a disclosure on when the hashes were recorded).
- **Block accuracy.** The leaf blocks themselves are exact to ≤1.1e-5 (ideal op-amps). With E24 parts at mcC tolerances they are ≤7.4e-4.
- **Error source.** The error versus golden is almost entirely the upstream S error.
  - Forms in S directly read 1e-5…4e-4 (ideal).
  - Forms in 1−S (Ω_DM h², |V_us|, Ising2D γ, Ω_b h²) amplify the S error by about 20×: −3.4e-3, −6.4e-4, −2.5e-4 and −3.0e-3 at 27 °C.
  - mcC sd on those forms: 4.5e-2, 8.5e-3, 3.2e-3 and 4.4e-2.
  - Pressure point: one shared fix (ΔVbe matching and the e³ stage trim, §10/§11) improves every 1−S leaf at once.
- **Domain limit.** One mcB draw (seed 2) put 1−S_chem at 0.013, against a nominal 0.0447. That drives the Ω_b h² LOGR input positive enough to reverse-bias the reference cascode (n > 0).
  - This is the same domain exit as Phase-2 mcB seed 39, not a solver failure.
  - mcB tier 1 converged 5/6; mcC 8/8; ideal 0/27/50 °C 3/3.
