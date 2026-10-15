# FSOT Omni-Solver: completeness audit (engine vs. circuit)

**Authority:** FSOT-2.1-Cpp @ `3466e8d4211d786fa7a430de1ee75131fb3b2e97` (`include/fsot/engine.hpp`, `closed_forms.gen.inc`; engine pin AEB2AD).
**Formal mirror:** FSOT-2.1-Lean hub @ `fb76270` (`FSOT/Formal/Scalar.lean`).
**Hub dynamics (not part of the pinned engine):** `vendor/fsot_dynamics.py`, sha256 `837817ba3b0422d98ab9138223029a51454f9e99fc279063a530213ad7121436`.

**Machine dump:** `phase2/cpp/audit.cpp` → `phase2/results/audit_constants.tsv` (all seeds and layer constants, with line references) and `phase2/results/audit_folds.tsv` (all 35 folds, every factor of every term). For each fold, `S_check` (rebuilt factor by factor) equals the engine `S` bit for bit.

**Rules:**
- Every item is listed, however small numerically.
- "Phase 1" is what the published Phase-1 slice (main @ 96b4c0c) actually realises.
- "Phase 2 block" is the pure-analog block that realises the item. Every Phase 2 block is FSOT-derived, locked and built on `feat/phase2`.
- Status: **B** = built and simulated in Phase 2, **B\*** = built, but the honest error is at or above the term's own size, **P1** = already in Phase 1, **R** = realised as an input or constant, **–** = not a circuit quantity.
- The final status column is filled in from the Phase-2 results (see "Status after Phase 2" at the end).

Notation used below:
- **V_NEP** = 10·V_T, the PTAT "neper bus": 1 neper corresponds to 10·kT/q (0.2587 V at 27 °C).
- **I_U** is the unit current: 1 FSOT unit corresponds to 5 µA.

## 1. Seeds (engine.hpp:110-115)
All seeds are dimensionless.

| Item | Value | Phase 1 | Phase 2 block |
|---|---|---|---|
| π | 3.14159265 | as resistor ratios | Seed input: precision divider ratio (π sets R ratios in the constant generator, CG) |
| e | 2.71828183 | as V_T exponent (antilog of 1 neper) | CG: antilog of 1 neper (I_U·e) |
| φ | 1.61803399 | resistor ratio | Seed input: ratio; φ⁴ = 3φ+2 identity |
| γ (Euler) | 0.57721566 | in K only | Seed input: ratio |
| G (Catalan) | 0.91596559 | in K only (through C_eff) | Seed input: ratio |

## 2. Layer 1 (engine.hpp:116-124)

| Item | Formula | Value | Meaning | Phase 1 | Phase 2 block |
|---|---|---|---|---|---|
| α | ln π/(e φ¹³) | 8.0829e-4 | hits damping / growth rate (nepers) | folded into the constant | Exponent voltage α·V_NEP = 209 µV on the T1 bus (a3 and a4); CG: log π − 1 − 13 log φ → antilog |
| ψ_con | 1 − e⁻¹ | 0.63212 | consciousness offset of the phase argument (rad) | folded into the constant | Phase-summer input (rad on the 1 V/rad bus); CG: I_U − I_U·e⁻¹ |
| η_eff | 1/(π−1) | 0.46694 | phase gain divisor | folded into the constant | Phase-summer gain (π−1); CG ratio |
| β | e^{−(π^π+e−1)} | 2.6209e-17 | valve/T3 amplitude | **omitted** | T3 log channel: ln β = −38.18 neper on the ×1 junction bus (−0.988 V) |
| Γ_c | −ln2/φ | −0.42839 | chaos numerator | folded | CG: log 2 / φ |
| Ω | √2·sin(π/e) | 1.29413 | chaos denominator | folded | CG: sine series |
| θ_s | sin(ψ_con·η) | 0.29090 | valve angle (rad) | folded | CG: sine series |
| POOF | exp((−ln π/e)/(η ln φ)) | 0.15348 | valve "poof" amplitude | folded | CG: log-domain; HEP fold look 1 − POOF/π |

## 3. Layer 2 (engine.hpp:125-138)

| Item | Value | Meaning | Phase 1 | Phase 2 block |
|---|---|---|---|---|
| C_eff | 0.95775 | growth coupling | folded | a4 antilog input current I_U·C_eff |
| A_bleed | 1.04697 | acoustic bleed amplitude | **omitted** | T3 acoustic factor |
| P_var | 0.95799 | observer phase / phase-term variable | folded into the observer constant O | Observer block: cos(δψ+P_var) argument and exp(C_F·P_var); T3 phase factor |
| B_in | 0.78798 | input coupling (nepers per rad of δψ) | folded | a3 exponent weight B_in·δψ; T3 phase factor |
| A_in | 1.66685 | acoustic input amplitude | **omitted** | T3 acoustic factor |
| SUCTION | 0.14703 | valve suction | **omitted** | T3 valve factor |
| CHAOS | −0.33102 | scale chaos | **omitted** | T3 factor (1 + CHAOS(D−25)/25), a current-mode linear block |
| P_base | 0.21235 | γ/e | in P_new | CG |
| P_new | 0.30030 | log-dimension coupling | P1 (R_x) | a5 block (PTAT-referenced) |
| C_FACTOR | 0.28761 | **consciousness factor** C_eff·P_new | folded into the constant O | Observer block exponent C_F·P_var·V_NEP = 71.3 mV |
| K | 0.42011 | output gain | P1 | S summer weight |
| C_cosm | 0.062620 | Cosmology interpretation constant | – | CG output (input to closed forms only) |

## 4. Core law S = K(T1+T2+T3): compute_scalar, engine.hpp:201-221 (Lean Scalar.lean)

| Factor | Exact formula | Meaning / units | Phase 1 | Phase 2 block |
|---|---|---|---|---|
| a1 | N·P/√D | amplitude per √dimension (dimensionless) | D^{−1/2} only (N = P = 1 hard-wired) | Log-domain: ln N + ln P − ½ ln(D/25) − ln 5 (LOGR blocks for N, P, D; constant −ln5·V_NEP) |
| a2 | cos((ψ_con+δψ)/η) | phase response | constant (look = 1 only) | **Trig block:** y = x − π (1 V/rad), \|y\| rectifier, LOGR, antilogs for y², y⁴, y⁶ → cos y = 1 − y²/2 + y⁴/24 − y⁶/720; sign by comparator plus CD4053 polarity switch |
| a3 | exp(−α h/N + ρ + B_in δψ) | input/hits exponent | constant | Neper-bus exponent: ρ·V_NEP + B_in·δψ·V_NEP − α·(h/N)·V_NEP |
| h/N | recent_hits / N | hit rate | absent | Translinear divider I_PT·I_U/I_N; hits switch (CD4053) |
| growth | exp(α(1−h/N)γ/φ) | growth (≈ 1.000288) | folded | Antilog with base −αγ/φ·(1−h/N)·V_T (48 µV at the junction) |
| a4 | 1 + C_eff·growth | coupling | folded | I_U + C_eff·I_U·growth → TIA → LOGR |
| a5 | 1 + P_new ln(D/25) | log-dimension correction | P1 | Current I_PT0 + u_D/R_x into a LOGR (PTAT reference, T-invariant ratio) |
| **quirkMod (observer)** | observed ? e^{C_F·P_var}·cos(δψ+P_var) : 1 | **observer / consciousness effect** (Scalar.lean:73-76) | **only a fixed resistor ratio O for δψ = 1** | **Observer block:** exponent C_F·P_var·V_NEP (+71.3 mV) plus a sine block for cos(δψ+P_var) = −sin(w), w = δψ+P_var−π/2. Gated by CD4053 on `observed`; the sign enters the polarity chain by comparator |
| T2 | scale·amplitude + trend_bias | linear modulation | constant K from VR (scale = amp = 1, tb = 0) | Translinear multiplier I_U·scale·amp plus a trend_bias current |
| T3 valve | β·cos δψ·(NP/√D)·(1+CHAOS(D−25)/25)·(1+POOF cos(θs+π)+SUCTION sin θs) | valve term | **omitted** (bounded by 3.2e-17 of S) | T3 log channel (×1 bus) with a cos δψ sine block, a1 shared, chaos current block, valve constant |
| T3 acoustic | 1 + A_bleed sin²δθ/φ + A_in cos²δθ/φ | acoustic modifier | **omitted** | sin δθ series block; cos²δθ = 1 − sin²δθ; log channel |
| T3 phase | 1 + B_in·P_var | phase modifier | **omitted** | Constant current ratio into a LOGR |
| K·(…) | output gain | S in volts (2 V per unit S) | P1 | S summer |

## 5. ScalarInput (24 fields; engine.hpp:187-191). Every field is a circuit input.

| Field | Default | Phase 1 | Phase 2 realisation |
|---|---|---|---|
| N | 1 | hard-wired | I_N current input (LOGR) |
| P | 1 | hard-wired | I_P current input (LOGR) |
| D_eff | fold | P1 ladder | ladder (LM139 vendor model) → I_D, with two CD4053 poles (core + T3) |
| psi_con | ψ_con | folded | phase-summer input |
| delta_psi | look | 1 only | look selector: dividers 1, e/π, 1−POOF/π (CD4053) |
| recent_hits | hits | absent | hits switch |
| rho | 1 | folded | neper-bus input ρ·V_NEP |
| B_in, C_eff, P_new | layer | folded | weights / currents |
| observed | fold | resistor choice | CD4053 control line |
| beta | β | absent | T3 −38.18 neper input |
| chaos, poof, suction, theta_s | layer | absent | T3 blocks |
| delta_theta | 1 | absent | acoustic sine block input (1 V/rad) |
| A_bleed, A_in, P_var | layer | absent/folded | T3/observer |
| scale, amplitude | 1 | constant | T2 multiplier inputs |
| trend_bias | 0 | absent | T2 current input (0 by default; the input is built) |
| alpha | α | folded | neper bus |

## 6. Folds and domain branches (engine.hpp:33-98, 151-184)

| Item | Phase 1 | Phase 2 |
|---|---|---|
| nest_generations (20 generations) | P1 g-string | P1 |
| D_eff = nearbyint(5·5^{g/19}) | P1 comparator ladder (0 mis-rounds) | ladder with LM139 vendor model |
| MEDIUM_ORIFICES (12 unobserved) | P1 (resistor select) | `observed` line into the observer block |
| fold_look: Atomic e/π, HEP 1−POOF/π | **not built** (33/35 folds) | look selector + trig/observer blocks → **35/35** |
| fold_hits: HEP = 1 | **not built** | hits switch + h/N divider |
| fold_C (interpretation C) | – (does not enter S) | CG outputs the constants it is built from; label only |

## 7. Everything else in the engine

| Item | Where | Phase 2 |
|---|---|---|
| 346 closed forms | closed_forms.gen.inc | Inputs = seeds, layer 1/2, S_cosm/S_quant/S_chem → CG plus FCL outputs. Phase 1 realised 4 closed forms (α_s, 1/α, Ω_b h², m_H/m_W). Phase 2 rebuilds Ω_b h² from a current-mode 1−S_chem and 1/α from e³ = three e¹ stages. The other 342 are formulas *on* these voltages; they are not realised individually (see "Status after Phase 2") |
| Floor rows (N_Layers ⌊2π⌋ etc., 9 rows) | closed_forms | comparator ladder on the CG voltage (same block as the D ladder); integers are exact |
| fsot_correct m(1+\|S\|α) | engine.hpp:240 | Ledger B, not a prediction; one antilog-free multiplier: 1 + \|S\|·α (proposal; α·\|S\| ≈ 8e-4) |
| Mode parity/corrected | engine.hpp:100-104 | affects closed-form exponents only; no core-law effect |
| core.hpp ternary BTFloat, fsotb_vm trinary ISA | include/ | number-representation / VM layers; not analog quantities (–) |
| host leaves (seed_leaves, axial_residue) | include/fsot | host-side numerics (–) |

## 8. Time / evolution dynamics
The pinned engine has **no time dynamics**: compute_scalar is static. The hub `vendor/fsot_dynamics.py` (not the authority) defines toy continuum equations:

| Item | Formula | Phase 2 block |
|---|---|---|
| scalar relaxation | ∂_t S = −v∂_xS + κ∂_xxS − γ_rel(S − S_eq), with γ_rel = \|CHAOS\| + ψ_con·POOF and κ = (POOF+SUCTION)·A_bleed | 0-D (uniform) mode: op-amp leaky integrator relaxing the S output to S_eq at γ_rel/τ₀ |
| viscous mode + observer source | ∂_t v = (μ/ρ)∂_xx v + J_obs/ρ, with μ = \|CHAOS\|·\|D−25\|/25 + A_bleed·POOF and J_obs = C_F·cos(δψ+θ_s) (observed only) | single-mode leaky integrator: steady state v = J_obs/(μk²) |
| continuity, advection, NSE cartoon | – | not built (spatial lattice; stated as remaining) |

## Status after Phase 2
Status codes are defined in the rules at the top. "rel" is (circuit − engine)/engine with ideal op-amps and exact resistor values at 27 °C, unless a row says otherwise.

| Item | Phase 1 | Missing before Phase 2 | Phase 2 status | Honest error / note |
|---|---|---|---|---|
| Seeds π, e, φ, γ, G | ratios | – | **R** | exact resistor ratios (E24 realisation errors are listed in design2_values.tsv) |
| α, ψ_con, η_eff, Γ_c, Ω, θ_s, POOF | folded into one constant | as separate quantities | **B** (as neper-bus / angle-bus constants) | V_NEP within 7e-6 of 10kT/q; T1 rel +2.0…+2.8e-4 (IKF) |
| β (T3 amplitude) | omitted | yes | **B\*** | ln β on the T3 log channel; ln T3 within 3–6e-4 Np; linear T3 ≈ 1e-22 A is not regulable |
| C_eff, P_new, K, B_in, P_var | folded | B_in, P_var as separate inputs | **B** | – |
| A_bleed, A_in, SUCTION, CHAOS | omitted | yes | **B\*** (T3 acoustic / valve / chaos blocks) | inside T3 (T3/S ≤ 3.2e-17) |
| C_FACTOR (consciousness factor) | folded into O | yes | **B** | exponent C_F·P_var·V_NEP = 71.3 mV |
| C_cosm, P_base | – | – | **R** | constant-generator inputs (CG only as resistor ratios; full CG is remaining work) |
| a1 = NP/√D | D^{−1/2} only | N, P inputs | **B** | – |
| a2 = cos((ψ+δψ)/η) | constant (look 1) | δψ dependence | **B** | series to y⁶ through cos x = −cos(x−π); truncation is the y⁸/8! term (in the locked derivation the factor rebuild matches the engine to 1e-15 with the exact cos; the circuit series truncation is part of the T1 residual) |
| a3 exponent incl. −α h/N | constant | h/N | **B** | α·h/N ≈ 209 µV·h/N: offset-limited (class-B Vos 10 µV) |
| growth | folded | yes | **B** | 48 µV at the junction: offset-limited |
| a4, a5 | folded / P1 | a4 | **B** | – |
| **quirkMod (observer)** | fixed ratio for δψ = 1 | **yes (first built)** | **B** | observed folds S rel +1.4…+1.9e-4; unobserved −2.7…−4.0e-4; sign comparator + CD4053 |
| T2 = scale·amp + trend_bias | constant | scale, amp, tb inputs | **B** | – |
| T3 valve / acoustic / phase | omitted | yes | **B\*** | see β |
| K summer | P1 | – | **P1/B** | – |
| 24 ScalarInput fields | 4 | 20 | **B** (all 24 are circuit inputs) | – |
| D ladder (20 generations) | P1 | – | **B** (LM139 vendor model in e24_opa netlists; behavioural in the ideal netlists) | D exact 35/35 nominal; 17/17 converged MC runs |
| fold_look (Atomic e/π, HEP 1−POOF/π) | not built | yes | **B** | **35/35 folds** (Phase 1 had 33/35) |
| fold_hits (HEP) | not built | yes | **B** | HEP S rel +1.57e-4 |
| MEDIUM_ORIFICES / observed | resistor select | gating | **B** | – |
| Ω_b h² (closed form) | +0.89% (fail) | current-mode 1−S | **B** | ideal −2.95e-3 (pass, bar 6.7e-3); E24 −8.7e-3 (fail); E24 + OPA187 I_B −1.2e-2 (fail) |
| α_s, 1/α (closed forms) | P1 | e³ as three e¹ stages | **B** | 1/α +1.85e-4 ideal (flat 0–50 °C); +5.2e-4 E24 + OPA187 |
| PTAT reference + oven | single PTAT | 2-cell PTAT, oven | **B** | V_NEP ≤ 1e-5 at 0/27/50 °C; oven core 58.9–60.2 °C over ambient 0–50 °C |
| the other 342 closed forms | – | – | **–** (formulas on CG/FCL voltages; not realised one by one) | remaining work |
| floor rows, fsot_correct | – | – | not built | remaining work (proposal in §7) |
| Mode parity, ternary/VM, host leaves | – | – | **–** | not analog quantities |
| dynamics: scalar relaxation, viscous mode + observer source (hub) | – | yes | **not built** | remaining work (0-D leaky-integrator design in §8) |
| continuity / advection / NSE cartoon | – | – | not built | needs a spatial lattice |

Summary: every core-law factor and all 24 inputs are built, including the observer and all of T3. T3's linear current cannot be regulated, so T3 is resolved only in the log domain. The circuit gives 35/35 folds (Phase 1: 33/35). Not built: the full constant generator, the remaining 342 closed forms, fsot_correct, the floor rows and the hub dynamics.
