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

## 13. Phase 3b: the constant generator, tiers 2a and 2, MC convergence, and the remaining loops

**What converges.**
- Ideal: all 11 compiled netlists at 0, 27 and 50 °C (33/33).
- Monte Carlo at 27 °C: 115/132 (mcC 79/88, mcB 36/44), using the blocksolve4 fork (Amendments 4.3 re-anchor and 4.4 ideal seed; verify tolerances unchanged).
- The two solver amendments cleared 5 of the 22 original failures: 2_X mcC1, 2_T mcC5 and 2a_T mcB1 by re-anchor, and 2a_P mcC1 and mcB1 by the seed.

**The remaining failures are real circuit limits, not solver artefacts.**
1. **Cascode headroom.** In every LOGR and CAL, the 2N7000 cascode drain sits on a 0 V virtual ground. Its source (the BJT collector) is servoed to the BJT base voltage. The base sits at the log-domain baseline, about −V_T ln 2 = −17.9 mV for Ir = Ix/2. So the cascode works only while the base is below 0 V: a margin of 0.69 neper on the low side of a LOGR input, or the high side of an antilog output.
   - When an upstream error pushes a LOGR input below ~50 % of nominal, the cascode reverses and the block has no DC solution. Verified: T1 mcB s2, zL83, input 0.53 V vs 1.78 V ideal.
   - The same happens when an antilog output exceeds ~2× nominal. Verified: GEN mcB s3, BETA readout, base +0.112 V.
   - **Proposed fix, a topology change (needs a leafc fork and a new lock).** Deepen the baseline: Ir = Ix/e³ in LOGRs and a larger CAL input current, so a base stays negative over ±3 nepers. The other option, offsetting the collector clamp below the base, costs a systematic Early error of V_off/V_AF ≈ 1.4e-3 at 0.1 V.
2. **A difference below component resolution: Chain_consistency_%** = |(m_τ/m_μ)(m_μ/m_e) − 9πφ¹⁰| / (9πφ¹⁰) · 100 = 0.00667 %.
   - The two products differ by 6.7e-5 relative. That is below the E24 realisation error (up to 4e-4), so even the ideal circuit is off by 29 % on this leaf.
   - It is below the mcC 0.02 % resistor tolerance as well, so under MC the sign of the difference is random. The fabs() is compiled for a fixed sign, so the LOGR then sees a negative current and fails (12/12 MC runs).
   - **FSOT pressure point.** This row is FSOT's own internal consistency check, and it sits at the 1e-4 level. No analog circuit built from separate products can resolve it.
   - A precision full-wave rectifier would make it converge, but the output would still be noise. The honest analog verdict is "consistent to better than ~2e-4".

**The most sensitive leaves (MC σ) and why.** Each is an analog amplification of an upstream error by a structural factor of the FSOT form, |∂ln f/∂ln x|:
- **BETA = exp(−(π^π + e − 1)):** a 36.5-neper exponent amplifies the π^π error 36.5×. mcC σ 13 %, mcB worst −95 %. It dominates the GEN errors. FSOT pressure point: the core constant BETA (2.6e-17) is exponentially sensitive to π^π.
- **η_baryon = POOF¹¹ / (πγ):** an 11th power amplifies POOF's error 11× (σ 6.9 %).
- **Cancelling sums**, where the result is much smaller than its terms:
  - γ₁ Stieltjes = G² + cos e: terms about 12× the result (σ 4.9 %).
  - γ₂ = π⁻² − γ⁴: 11× (σ 4.1 %).
  - θ* = P_base·φ − γ²: ~33× (σ 3.8 %).
  - Hashing bound: σ 4.0 %.
  - Ω_b h² and Ω_DM h²: (1 − S_chem) and similar (σ 4.4 %).
  - |V_cb| = S_q/C_eff − S_q (σ 2.5 %).
  - FSOT pressure point: these closed forms are near-cancellations of order-1 constants.
- **|V_cb| is a case where the math could be adapted without changing FSOT.** As S_q·(1/C_eff − 1) it does not cancel. The compiler follows the engine expression literally. Factoring it would be an algebraic identity, not a rule change, but it is a design decision and needs approval.
- **Threshold rows (floor):** N_Drives = Binding_Window = ⌊π²⌋ = 9 has a margin of 1.3 %. One of four mcB samples (1 % unmatched BJT I_S) flipped it to 10, an 11 % error. N_Columns = ⌊e³⌋ = 20 has the smallest margin (0.43 %) but did not flip, because e³ is built with fewer independent parts. Both are FSOT rows that sit near an integer threshold: an analog realisation needs better than 0.4 % matching, or matched pairs (mcC never flipped).
- **mcB vs mcC:** mcB has unmatched BJTs, with I_S spread about ±1 %, i.e. 0.26 mV per LOGR. Through weights like ln φ × 13 in ALPHA, that gives 10 % constant errors. mcC, with matched pairs and 0.02 % resistors, keeps the constants at 0.05–0.7 % σ (BETA excepted). Matched transistor pairs are the single most important real-part choice.

**The constant generator's effect on tier 1.** The ideal errors change by ≤ 6e-5 (|V_cb| +2.75e-4), so the S-channel error dominates. The MC σ grows on cancelling forms (|V_cb| 12×, T_CMB 3.6×), because the generator's constant errors now enter where a single locked resistor ratio used to stand.

**Remaining stability loops** (`phase3/results/stepresp_remaining_loops.tsv`, `phase2/spice/stepresp.sh`). The 1 mV step is injected at the servo output, and the transient starts from the .ic-pinned operating point.
- **Measurable loops:** the Cosmology tiny-tap cascodes cy2 (3.3 nA) and cy3 (13.6 pA) are linear. Overshoot is 8.3e-4 and 1.9e-4, so ζ ≈ 0.91–0.94 and the phase margin is about 74–75°: stable.
- **Railed taps** (a3, sw3, sv4, st5 in every fold; cy2 and cy3 in Atomic Physics):
  - **What happens:** in the static ideal solution the servo output sits at −3e5 to −1e6 V. That is a non-physical value for an unclamped ideal op-amp; a real one would rail at −8.9 V. The 2N7000 VDMOS model has no subthreshold conduction, so it cannot carry these tap currents: 1e-22 A (a3), 0.1–3 pA (sw3, sv4, st5, cy2 Atomic) and 5e-16 A (cy3 Atomic).
  - **Stability:** these loops are not oscillating, but neither are they in linear operation.
  - **The tap currents are effectively dropped in the Phase 2 netlist.** Relative to µA sums that is ≤ 3e-6, below the block accuracy. Damian's rule is not to drop tiny terms, though, so this is a disclosed gap.
  - **Fix:** Phase 3b's CALU (uncascoded antilog, for terms below 1e-4 of the sum) already avoids it for the leaves. The base engine needs the same change in a forked netgen, because netgen2 is LOCK2-covered.

## 14. Phase 3c/3d: headroom, exact rewrites, the rectifier, and carrying the tiny terms (LOCK5, LOCK6)

**Headroom (LOCK5 5.1, leafc5): convergence up, spread up.** Every compiled LOGR now runs Ir = Ix e⁻³ and every compiled antilog output IIN e⁻³, so a log node sits at −3 V_T (was −V_T ln 2).
- Convergence (`phase3/results/leaves5_convergence.tsv`, 2a_X excluded because LOCK5 2a_X failed its ideal run, see below): mcC 79/80 → 80/80; mcB 36/40 → 37/40. GEN mcC s3 (the unresolved LOCK4 NaN at base cell rzb194) now converges. T1 mcB s2 (verified headroom) and 2_M mcB s3 (probable headroom) converge, which confirms 2_M. 2a_T mcB s3 still fails at the same LOGR (rzl60, 4 attempts, about 35 min): LOCK4's "probable headroom" is **not** confirmed for it, because a 3-neper margin did not help.
- Caveat on per-seed comparisons: a seed draws its random numbers in netlist order, so a LOCK5 netlist with different parts gets different draws per device. Seed-for-seed "before/after" is not a like-for-like test; the counts are.
- Remaining mcB failures: GEN s3, 2_X s3 (base cell zB8, G_CAT LOGR, whose base cells still run at m·c ≥ 1.4, margin 0.6 neper) and 2a_T s3. Traced GEN s3: the error enters at the π^π antilog (exponent summer zS25 −1.8 mV = −0.07 neper), grows through π^π + e − 1 (−24 %), and the BETA readout takes that sum as a 38-neper exponent, so its antilog base lands at +0.19 V (ideal −0.078 V), beyond any fixed headroom. The −0.07 neper comes mostly from V_NEP being +0.72 % high in that sample (mcB unmatched PTAT pair) times the large summer constants.
- **The price, found by the MC: the spread grew about 1.5–2×** (median mcC σ e.g. 2_M 2.4e-3 → 3.7e-3, GEN 1.8e-3 → 5.2e-3; mcB σ about doubles). Mechanism: each summer constant is c nepers taken from the PTAT bus V_NEP, so a V_NEP error δ costs c·δ nepers. The e⁻³ baseline nearly doubles the mean |c|: 6.4 → 11.8 nepers (max 39 → 92). See refinement (1).

**Exact rewrites (5.2).** |V_cb| = S_q(POOF sin θ_s (1+g) − g)/C_eff, g = G_cat/(π⁵φ), is identical to S_q/C_eff − S_q in exact arithmetic, checked to 1e-15 against golden. Its condition number (Σ|terms|/|result|) drops from 46 to 1.09. MC on T1: mcC σ 2.48e-2 (LOCK4, generator) → 8.1e-3 (LOCK5), mcB σ 0.21 → 0.048, a 3× and 4× reduction against a general 1.5–2× growth. The intrinsic ones did not improve: mcC σ θ* 3.8e-2 → 4.6e-2, Hashing bound 4.0e-2 → 4.9e-2, γ₂ 4.1e-2 → 4.6e-2, γ₁ 4.9e-2 → 5.2e-2. γ₁ (24), γ₂ (22; factoring makes it 44), θ* = γ(φ/e − γ) (65 either way) and the Hashing bound (18) are cancellations between independent FSOT constants: no exact rearrangement lowers them, so they are intrinsic. Ω_b h² and Ω_DM h² already use the exact rearrangement (the base engine's single 1 − S current node).

**Chain_consistency_% (5.3 and 6.3).** The full-wave rectifier makes the sign question disappear. It needed low-leakage diodes, though: with 100 k resistors the signal is 2.6 nA, the same size as the 1N4148 model's 2.5 nA reverse current (LOCK5 ideal read 2.65 µV for 261 µV and the next LOGR failed). With diode-connected 2N3904s (LOCK6), the ideal circuit converges at 0/27/50 °C and reads 0.0052 % against 0.00667 % (−22 %). Under MC the difference node (before the log readout) spreads with σ ≈ 0.045 V in mcC (≈ 0.9 in Chain-% units) and 0.27 V in mcB (≈ 5.4). **Verdict: consistent to better than ~1 % (mcC parts; ~5 % with mcB parts).** The engine's 0.0067 % is about 150× below what the circuit resolves. The log readout after the rectifier still fails in 10 of 12 MC runs, because |x| reaches 30–230× its nominal and pushes the readout antilog above its 3-neper headroom. A readout sized for the resolution, not the nominal, is listed as a refinement.

**Tiny terms carried, not lost (LOCK6, Damian 6:11 PM).** All 247 base-engine taps below 1e-4 of I_U are now carried in the log domain: lifted by an exact locked integer n = floor(−3 − ln t) nepers (7…36), read at scale on a TIA (0.37–1 V), and divided back by a matched antilog before injection. Results (`phase3/results/tiny_terms6_fold.tsv`, `tiny_terms6_bench.tsv`):
- **Carried readout vs engine, inside the full ideal fold channel (35 folds, 27 °C):** cy3 −1.1e-4 (worst −1.8e-3, Atomic Physics t = 1.1e-10), sw3 −1.4e-4, sv3 +2.3e-4, sv4 +3.2e-4, st4 +4.5e-4, st5 +6.1e-4, **a3 (the T3 valve, t = 7.8e-18…2.4e-17) +8.8e-4**, cy2 −1.2e-3, sw2 −2.5e-4. These include the engine's own upstream errors.
- **Bench (same stimulus V_T ln t, 0/27/50 °C):** carried readout 1.1–4.0e-4 for every tap at every temperature.
- **Uncascoded direct stage (LOCK5) and the divided-back current:** +0.3–0.8 % for terms ≥ 7e-7 (the Early effect: V_cb = |V_x| over V_AF = 74 V). For terms of 3e-8 and below (st5, sv4) the error is +7–8 % at 27 °C and +140–170 % at 50 °C; for a3 the current is pure floor (~1e-14 A against 4e-23 A). The floor is junction leakage, which grows with temperature, plus the simulator's gmin. **A sub-pA term cannot be carried as a current; carried as a log voltage, it is accurate to 1e-4 even at 1e-17.**
- **Charge integrator (C = 1 nF ‖ 100 G, T = 1 s):** ideal op-amp: +0.02 % … −0.1 % for terms ≥ 7e-7 at 27 °C (the stage's +0.5 % minus the 0.5 % leakage droop T/2RC), the same floor as the direct stage below that. Hold drift −1 %/s (V/RC). Settling: by construction it needs the full T (1 s), against about 1 ms for the log-carry. With OPA187 (100 pA typical bias) the bias current exceeds every tiny term (+59× to +2.5e12×). Even an electrometer-class 25 fA input gives +0.2 % at 13 pA and +22–28 % at 0.15 pA. **RC storage verdict:** usable only for terms ≳ 10 pA with an electrometer op-amp, and then it is 1000× slower than the log-carry, with 1 %/s droop. It is a cross-check, not a carrier.
- **Physical limit:** a 5 µA summing node with an OPA187 (100 pA bias) resolves about 2e-5 of I_U. The carried terms are exact as readouts, but adding them back into that node cannot change S by a resolvable amount when t < 2e-5. So they are carried and readable, and any downstream form that needs them can take the carried voltage directly.

**Refinements (B) found or confirmed this round, in priority order.**
1. **Summer constants from V_NEP (the new top error source).** Choose each LOGR/antilog reference ratio, which is resistor-exact and PTAT-free, so that the summer's V_NEP constant is near zero. The e⁻³ shifts cancel exactly when a summer's input weights add to 1. Also take V_NEP from a larger PTAT ratio (ln 256 rather than ln 16 halves its relative error) and build the PTAT pair from a matched/common-centroid pair or quad.
2. **Base-engine LOGR headroom.** The LOCK2 channel LOGRs run near Ir/Ix ≈ 1 (n ≈ −9 mV, margin 0.36 neper). LOCK6 T1 mcC s6 failed at LOGR_vp in the Chemistry channel. The e⁻³ baseline should be applied to the base engine through a forked design2 deriver (new lock).
3. **Early effect of uncascoded output stages** (+0.3–0.5 %): bias the divide-back input transdiode at the same V_cb as its output transistor.
4. **ΔV_BE matching:** mcB (unmatched) vs mcC (matched) is a 5–10× spread difference on every netlist. Matched pairs/quads or common-centroid arrays at every LOGR/antilog pair are the largest single hardware lever. Not yet modelled as a separate MC class.
5. **Antilog-node offset:** measured input-node offsets of 1–15 µV (op-amp V_os) against 2.5 V across R_in, i.e. ≤ 6e-6. Negligible at present.
6. **Temperature-tracking angle bus:** angles enter as V_REF-referenced (temperature-flat) currents, which is correct for linear quantities. T1's worst error moves only 3.4e-3 → 4.1e-3 over 0–50 °C. No change needed.
7. **Op-amp input bias** sets the summing-node resolution (2e-5 of I_U with OPA187) and rules out RC storage of sub-pA terms (above).
8. **Chain_consistency readout** sized for the resolution rather than the nominal value.

## 15. LOCK7/7.1/7.2 (refinements 1–3), LOCK8 (leaves part C) and Amendment 8.1 (host-independent derivation)

**Refinements 1–3 (LOCK7, Amendment 7.1): MC spread and convergence improved; ideal error got worse.** Flipped signal LOGRs (t = +1, constant ln av + 3), antilog input absorption, base cells at m·c ≥ e, a deeper base-engine LOGR baseline (KD = e⁻³/0.7, PTAT ratio 256, matched PTAT pair), Early-corrected uncascoded stages and range-sized readouts (`phase3/locked7*/LOCK7*.md`, results `phase3/results/leaves7b_*.tsv`).
- Mean |summer constant| (nepers) LOCK5 → 7.1: GEN 11.9 → 6.0, T1 7.2 → 2.4, 2_M 9.3 → 6.0, 2_X 7.2 → 2.8, 2a_T 6.3 → 5.6.
- mcC median σ LOCK5 → 7.1: GEN 5.2e-3 → 1.9e-3, 2_M 3.7e-3 → 2.4e-3, 2_P 4.8e-3 → 2.4e-3, 2_X 5.1e-3 → 2.5e-3, 2a_T 3.6e-3 → 2.0e-3; T1 4.4e-3 → 6.2e-3 (the base engine's S now carries more high-injection offset, below). mcB σ roughly halves on every netlist.
- Convergence: all 12 netlists 8/8 mcC and 4/4 mcB (144/144), including the LOCK5 failures GEN s3, 2_X s3, 2a_T s3 and the Chain_consistency readout (2a_X).
- **Price:** ideal 27 °C errors are 3–5× LOCK5's (233 of 357 leaves exceed 2× the LOCK5/6 baseline, 49 improved). Main causes: the antilog input absorption runs input currents up to 4.5 µA (≈ 6.7e-5 per stage of high-injection error, IKF 66.8 mA) and the deeper base references move the S offset +1.6e-4 → +5.4e-4. The CI regression baseline was **not** rebaselined (the rule says the baseline only changes when a lock improves results); this needs Damian's decision.
- Chain_consistency_% (range-sized readout, ρ = 100): ideal reads 0.0196 % vs 0.00667 % (+193 %, absolute 1.3e-4 %), MC spread 0.1–0.6 % (mcC): consistent at the 1 % level, as in LOCK6, and now converges in every MC run.
- GEN BETA (a 38-neper exponent of π^π + e − 1) is intrinsic: σ 0.27 (mcC). The exact split π^π = π³·exp((π−3) ln π) (GENpp) is worse (σ 0.42): the (π − 3) cancellation dominates.

**Leaves part C (LOCK8, `phase3/locked8/LOCK8.md`, results `leaves8_*.tsv`): every new block converges (C_S 15/15 runs, C_H 15/15).**
- domain_scalar("X") is the S output of one full-core-law fold channel per named domain (netgen7 `DCHAN`, 10 channels in C_S). S(species) = S_Neuroscience reads +5.5e-4 at 27 °C: this is the base engine's S offset (§ above), not a new error. Ratios of two channels cancel it: Cross_Opt_QO / Cross_MatSci_CM / Cross_Astro_PS +2.4…3.2e-5. Density(species) = N/(|S|·V) −4.9e-4 (the same offset, inverted).
- fsot_correct(m, X) = m(1 + |S_X|·ALPHA) is a Ledger-B correction of a measured value, not a prediction. The factor is realised to −2.5e-6 (27 °C), i.e. the correction |S|·ALPHA ≈ 7.7e-4 itself to ≈ 3e-3. mcC σ 2.5–3.3e-4 of the factor.
- Hub dynamics (vendor/fsot_dynamics.py on the engine constants; cross-checked in Python): relaxation rate γ_rel −1.6e-5, viscosity μ at D_eff(Fluid_Dynamics) = 12 −7e-6 and at D = 25 (the bleed floor A_BLEED·POOF) −1.4e-5, observer source J_obs −8.6e-5, c_s² = C_EFF/φ +7.0e-5, coupling κ = (POOF + SUCTION)·A_BLEED +2.4e-4 (worst; a sum of a small and a large term, then a product).
- Floor rows: already compiled in 2a_Q (LOCK7c, 9 rows, ideal ≤ 6e-6).

**Amendment 8.1: why CI failed, and the fix (`phase3/locked8b/LOCK8b.md`).** Four CI runs failed in the LOCK7 re-derivation. Cause: `realise()` (cpp/design.hpp) searches 3 E24 pieces only when the best 2-piece error is > 5e-4, and a summer-constant resistor 10000/0.15 = 66666.67 Ω has exactly 5.000e-4 with 62000 + 4700. glibc's long double logl/expl/powl/sinl/cosl use x87 transcendental instructions whose last bit depends on the CPU model, and GitHub's runner pool mixes models, so the decision flipped on some runners. The glibc/g++ version is not the cause: an Ubuntu 24.04 (glibc 2.39, g++ 13.3) chroot reproduces locked7c on this Intel host. A ±1-ulp perturbation of exp/log/sin/cos reproduces the failure class. Fix: `cpp/detmath.cpp` (deterministic transcendentals from + − × ÷ only, linked into the derivers), E24 decisions made on the 12-digit printed value, and computed columns printed with 12 digits. leafc8b re-derives byte-identically under ±1 ulp, −O0 and −O3 -march=native. 7 parts in 6 netlists changed from 3 to 2 E24 pieces (5e-4 vs 1e-4 on a 1 % part); the ideal results are unchanged, and the MC reruns (`leaves8b_*.tsv`) match 7.1 within seed noise (e.g. GEN mcC σ 1.89e-3 → 1.72e-3, T1 6.19e-3 → 6.25e-3). The libm-based re-derivations of locked7/7b/7c leaves and locked8 are retired from CI, their hashes are still checked, and this is disclosed in LOCK8b.md.
