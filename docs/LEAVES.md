# LEAVES — the FSOT closed forms as circuit blocks

Source: `include/fsot/closed_forms_core.gen.inc` of FSOT-2.1-Cpp at the authority commit 3466e8d (engine pin AEB2AD), parsed by
`phase3/cpp/leafmap.cpp` (C++20; `leafmap <gen.inc>` writes the TSV below). The golden value of every form is
`golden/golden_AEB2AD.tsv` (`sec.*` rows). No form is added, changed or re-fitted: the circuit computes the engine's own expression.

**Tiers (dependency order).**
- **0**: literal (a number in the table; realised as a resistor ratio off V_REF, no computation).
- **1**: fed directly by the circuit's S_cosm, S_quant, S_chem channel outputs (plus constants).
- **2**: uses derived FSOT constants (A_bleed, C_eff, Chaos, θ_S, ψ_con, …) but no S. Needs the constant generator.
- **2a**: uses only the base constants π, e, φ, γ, G (log-domain products/powers of V_REF ratios).

**Analog block vocabulary** (all already exist in the Phase-2 engine): LOGR (transdiode log), SUM (neper summer),
CAL (antilog), TIA (current→voltage), powers (trig series taps), FW (precision full-wave), comparator ladder (quantiser).
Tier 1 forms are all `b + k·x` with x one circuit signal, so each is **one inverting summer** off the S signals; the only product
(σ_8's |S_cosm|·S_quant) reuses the Ω_b h² LOGR–SUM–CAL–TIA product topology.

The `analog block` column is leafmap's generic, conservative block list from the expression tree; where a form reduces to `b + k·x` (all of tier 1) the built block is the single summer described above.

**Status key.** `SOLVED-P3T1` = solved by the circuit in Phase 3 tier 1 (netgen2 mode `leaf1`, values locked in `phase3/locked`);
`SOLVED-P2` = solved by the Phase 2 circuit (omb / consts modes); `planned` = mapped, not yet built.

**Constant generator: status, stated plainly.** In tier 1 the FSOT constants b and k (A_bleed/A_in, P_base, C_cosm·φ^(1/π), ln π, γ², 1/e, φ·A_in, |Chaos|, θ_S/φ, 1/C_eff−1, 1/ln φ, ψ_con, γ⁴/G⁴) are **resistor ratios** off V_REF. They are computed in C++20 from the engine and locked, the same practice as Phase 2's K and reference ratios. The circuit therefore solves each form *given* its constants; it does not yet *generate* the constants.

The circuit constant generator is the next build. It is a log-domain block: V_REF ratios for π, e, φ, γ, G; LOGR, SUM, CAL for products and powers; the powers block for sin/cos. It computes the derived constants, and those outputs then feed tiers 2a and 2 (313 forms). The tier-1 summers can be re-pointed from the resistor constants to the generator outputs without any change to their structure.

## Tier 1 results (ideal op-amps, exact values, 27 °C, blocksolve; `phase2/spice/run_bs.sh leaf1 ideal_ideal 0 27`)

`block_rel` is the leaf block's own error given the circuit's own measured x; `rel_vs_golden` includes the upstream S-channel error
(this ideal run: |S_cosm| +3.7e-4, S_quant +1.5e-4, 1−S_chem −3.4e-3, 1−S_quant −2.9e-3; the S rel error of the e³-gain T-stages, see INSIGHTS §7/§11, becomes ×20 larger in 1−S), which the 1−S forms amplify.

| id | form | circuit | golden | rel vs golden | x measured | block rel |
|---|---|---|---|---|---|---|
| H0 | H0 | 68.4324484 | 68.4449956 | -1.83e-04 | 0.502565466 | -1.07e-05 |
| TCMB | T_CMB | 2.72474586 | 2.72471169 | +1.25e-05 | 0.502565466 | -2.12e-06 |
| ns | n_s | 0.963316754 | 0.963333837 | -1.77e-05 | 0.502565466 | -3.48e-06 |
| MWZ | M_W/M_Z | 0.87553877 | 0.875373665 | +1.89e-04 | 0.950343419 | -2.25e-06 |
| OL | Omega_Lambda | 0.682787043 | 0.682736038 | +7.47e-05 | 0.950343419 | -3.94e-06 |
| ODM | Omega_DM_h2 | 0.120171771 | 0.12058582 | -3.43e-03 | 0.0445573638 | -5.04e-06 |
| s8 | sigma_8 | 0.808677802 | 0.808381839 | +3.66e-04 | 0.47765469 | -1.32e-06 |
| Vus | \|V_us\| | 0.229439987 | 0.229586479 | -6.38e-04 | 0.0496571137 | -4.69e-06 |
| Vcb | \|V_cb\| | 0.0419251473 | 0.0419188788 | +1.50e-04 | 0.950343419 | -4.06e-06 |
| Is2 | Ising2D_gamma | 1.24791806 | 1.24822387 | -2.45e-04 | 0.0496571137 | -2.91e-06 |
| mtW | m_t/m_W | 2.15032949 | 2.14976593 | +2.62e-04 | 0.502565466 | -2.24e-06 |
| mHW | m_H/m_W | 1.5510718 | 1.55083683 | +1.52e-04 | 0.950343419 | -2.08e-06 |
| C2 | Twin_prime_C2 | 0.66026293 | 0.660078806 | +2.79e-04 | 0.502565466 | -6.09e-06 |
| Omb | Omega_b_h2 | 0.0223950901 | 0.0224616214 | -2.96e-03 | - | - |

Worst |rel vs golden|: 3.43e-03 (Ω_DM h², the 1−S_chem form). All leaf blocks are within 1.1e-5 of their own inputs.

### Tier 1 across temperature and Monte Carlo (`phase3/results/leaf1_summary.tsv`)

The table gives the rel error vs golden.
- ideal: exact values with ideal op-amps.
- mcC: E24 parts, R 0.02 %, Vos 2 µV, ΔVbe 1e-5 (8 seeds, all converged).
- mcB: R 0.1 %, Vos 10 µV, ΔVbe 1e-4 (5 of 6 converged; seed 2 is a domain exit, see INSIGHTS §12).

`mcC_worst_block` is the leaf block's own error given its measured inputs.

| leaf | ideal_0C | ideal_27C | ideal_50C | block_rel_27C | mcC_n | mcC_mean | mcC_sd | mcC_worst | mcC_worst_block | mcB_n | mcB_sd | mcB_worst |
|---|---|---|---|---|---|---|---|---|---|---|---|---|
| H0 | -1.89e-04 | -1.83e-04 | -1.62e-04 | -1.07e-05 | 8 | +6.33e-04 | 1.06e-03 | 1.52e-03 | 7.4e-04 | 5 | 8.78e-03 | 1.46e-02 |
| TCMB | +1.30e-05 | +1.25e-05 | +1.08e-05 | -2.12e-06 | 8 | -7.14e-05 | 2.68e-04 | 4.65e-04 | 5.4e-04 | 5 | 5.77e-04 | 1.51e-03 |
| ns | -1.82e-05 | -1.77e-05 | -1.60e-05 | -3.48e-06 | 8 | +5.51e-05 | 2.79e-04 | 4.72e-04 | 4.0e-04 | 5 | 9.73e-04 | 1.41e-03 |
| MWZ | +2.30e-04 | +1.89e-04 | -1.48e-05 | -2.25e-06 | 8 | +8.89e-05 | 2.53e-03 | 4.21e-03 | 3.6e-04 | 5 | 2.27e-02 | 3.41e-02 |
| OL | +9.16e-05 | +7.47e-05 | -9.13e-06 | -3.94e-06 | 8 | +7.38e-05 | 9.98e-04 | 1.70e-03 | 3.5e-04 | 5 | 9.21e-03 | 1.38e-02 |
| ODM | -4.14e-03 | -3.43e-03 | +8.02e-05 | -5.04e-06 | 8 | +1.07e-03 | 4.52e-02 | 8.11e-02 | 3.4e-04 | 5 | 2.54e-01 | 5.50e-01 |
| s8 | +3.95e-04 | +3.66e-04 | +2.41e-04 | -1.32e-06 | 8 | -6.79e-04 | 2.95e-03 | 4.55e-03 | 3.0e-04 | 5 | 2.42e-02 | 3.51e-02 |
| Vus | -7.75e-04 | -6.38e-04 | +3.96e-05 | -4.69e-06 | 8 | +4.00e-04 | 8.47e-03 | 1.57e-02 | 6.8e-04 | 5 | 7.67e-02 | 1.33e-01 |
| Vcb | +1.83e-04 | +1.50e-04 | -1.42e-05 | -4.06e-06 | 8 | +6.72e-05 | 2.06e-03 | 3.28e-03 | 2.5e-04 | 5 | 1.85e-02 | 2.77e-02 |
| Is2 | -2.97e-04 | -2.45e-04 | +1.40e-05 | -2.91e-06 | 8 | +1.37e-04 | 3.19e-03 | 5.78e-03 | 4.2e-04 | 5 | 2.87e-02 | 4.94e-02 |
| mtW | +2.71e-04 | +2.62e-04 | +2.30e-04 | -2.24e-06 | 8 | -1.03e-03 | 1.79e-03 | 2.90e-03 | 2.7e-04 | 5 | 1.31e-02 | 2.12e-02 |
| mHW | +1.84e-04 | +1.52e-04 | -1.22e-05 | -2.08e-06 | 8 | +1.10e-04 | 1.97e-03 | 3.17e-03 | 2.9e-04 | 5 | 1.77e-02 | 2.65e-02 |
| C2 | +2.89e-04 | +2.79e-04 | +2.44e-04 | -6.09e-06 | 8 | -1.11e-03 | 1.97e-03 | 2.99e-03 | 2.0e-04 | 5 | 1.46e-02 | 2.31e-02 |
| Omb | -3.66e-03 | -2.96e-03 | +5.07e-04 | - | 8 | -6.29e-04 | 4.35e-02 | 8.22e-02 | - | 5 | 2.32e-01 | 5.02e-01 |

## Phase 3b: compiled leaves (LOCK4, constant generator, tiers 2a and 2)

Every compiled leaf is the engine's own C++ expression, compiled by `phase3/cpp/leafc` into LOGR/SUM/CAL, TIA, Taylor and comparator-ladder blocks. The derived constants come from the **constant generator** (netlist GEN), which uses its own `core.hpp` definitions built from four base reference ratios (π, φ, γ, G_cat) and e (1 neper). The values were locked before simulation (LOCK4 at a1d5620; targets re-pinned at 18 digits in locked4b, Amendment 4.5). Solves use `phase3/tools/run_bs4.sh` (blocksolve4 fork, Amendments 4.3 and 4.4), with ideal op-amps.

**Counts.**
- 363 rows are compiled: 20 GEN constants, 14 tier 1 (T1, re-pointed to the generator), 112 tier 2 (2_M 44, 2_P 49, 2_T 5, 2_X 14) and 211 tier 2a (2a_M 66, 2a_P 80, 2a_Q 9, 2a_T 12, 2a_X 44).
- 13 rows are tier-0 literals.
- 13 rows are not compiled: they use `domain_scalar`, which is not an FSOT S signal in this circuit.

**Convergence.**
- Ideal: 33/33 (11 netlists × 0, 27, 50 °C).
- Monte Carlo at 27 °C: mcC 79/88 and mcB 36/44 (115/132 overall), from `phase3/results/leaves4_convergence.tsv`.
- The 17 failures are diagnosed in INSIGHTS §13:
  - **12 (all of 2a_X): one leaf, Chain_consistency_%.** It is a 6.7e-5 relative difference, below component resolution. Its sign flips under MC, and the log of a negative current has no solution.
  - **2: cascode headroom, verified.** T1 mcB s2 and GEN mcB s3: a log or antilog base voltage rises above the 0 V virtual ground.
  - **2: probably the same headroom limit, not yet verified.** 2_M mcB s3 and 2a_T mcB s3.
  - **1: unresolved.** GEN mcC s3: ngspice fails numerically on a three-device base LOGR whose physical point is near ideal. The ideal-seeded attempt also returns NaN.

Errors are relative to golden (GEN: relative to the engine constant). They come from `phase3/results/leaves4_netlists.tsv` and the per-leaf table `leaves4_summary.tsv`.

| netlist | leaves | ideal27_median_abs | ideal27_worst | ideal27_worst_leaf | ideal_0..50_worst | mcC_runs_ok | mcC_median_sd | mcC_worst | mcB_runs_ok | mcB_median_sd | mcB_worst |
|---|---|---|---|---|---|---|---|---|---|---|---|
| GEN | 20 | 2.19e-05 | 1.69e-03 | BETA | 2.73e-03 | 7/8 | 1.80e-03 | 3.62e-01 | 3/4 | 1.57e-02 | 9.45e-01 |
| T1 | 14 | 2.81e-04 | 3.37e-03 | Omega_DM_h2 | 4.09e-03 | 8/8 | 2.60e-03 | 8.08e-02 | 3/4 | 1.86e-02 | 5.28e-01 |
| 2_M | 44 | 2.05e-05 | 1.18e-03 | eta_baryon_photon | 1.18e-03 | 8/8 | 2.36e-03 | 1.58e-01 | 3/4 | 1.67e-02 | 5.63e-01 |
| 2_P | 49 | 1.58e-05 | 3.13e-04 | Hashing_bound | 3.18e-04 | 8/8 | 1.95e-03 | 1.34e-01 | 4/4 | 1.49e-02 | 5.40e-01 |
| 2_T | 5 | 7.72e-06 | 4.47e-05 | Water_triple_K | 5.03e-05 | 8/8 | 1.17e-03 | 1.47e-02 | 4/4 | 9.98e-03 | 1.21e-01 |
| 2_X | 14 | 1.59e-05 | 1.23e-04 | tau_reion | 1.58e-04 | 8/8 | 2.13e-03 | 2.47e-02 | 4/4 | 1.97e-02 | 2.28e-01 |
| 2a_M | 66 | 9.86e-06 | 9.64e-05 | BP_C₂H₅OH | 1.06e-04 | 8/8 | 1.04e-03 | 1.87e-02 | 4/4 | 7.36e-03 | 9.92e-02 |
| 2a_P | 80 | 1.22e-05 | 4.04e-04 | gamma_2_Stieltjes | 4.38e-04 | 8/8 | 1.31e-03 | 7.09e-02 | 4/4 | 1.18e-02 | 4.12e-01 |
| 2a_Q | 9 | 3.19e-06 | 5.98e-06 | N_Lobes | 5.98e-06 | 8/8 | 2.25e-04 | 6.78e-04 | 4/4 | 5.22e-04 | 1.12e-01 |
| 2a_T | 12 | 1.57e-05 | 1.38e-04 | gamma1_Stieltjes | 1.75e-04 | 8/8 | 2.15e-03 | 7.66e-02 | 3/4 | 1.72e-02 | 6.21e-01 |
| 2a_X | 44 | 2.14e-05 | 2.88e-01 | Chain_consistency_% | 3.11e-01 | 0/8 | - | - | 0/4 | - | - |

**Constant generator vs the tier-1 resistor ratios** (`phase3/results/generator_vs_ratio_T1.tsv`). The ideal errors are almost unchanged, because they are dominated by the upstream S-signal error, not by the constants. The ideal 27 °C change is ≤ 6e-5 on every leaf except |V_cb| (+2.75e-4). The Monte Carlo spread grows on leaves whose engine expression cancels: |V_cb| = S_q/C_eff − S_q has a mcC σ of 2.1e-3 with the ratio and 2.5e-2 with the generator. A resistor ratio carries (1/C_eff − 1) = 0.044 as one part. The literal compile forms the difference of two terms 23× larger, so the generator's C_eff error is amplified 23×. T_CMB and Ω_Λ grow 2–4×. Ising2D_γ, m_t/m_W and |V_us| shrink in mcB, because there are fewer independent resistor tolerances.

| leaf | ratio_ideal27 | gen_ideal27 | delta_ideal27 | ratio_mcC_sd | gen_mcC_sd | ratio_mcB_sd | gen_mcB_sd |
|---|---|---|---|---|---|---|---|
| H0 | -1.83e-04 | -1.86e-04 | -3.00e-06 | 1.06e-03 | 1.55e-03 | 8.78e-03 | 9.30e-04 |
| T_CMB | +1.25e-05 | +2.95e-05 | +1.70e-05 | 2.68e-04 | 9.64e-04 | 5.77e-04 | 5.43e-03 |
| n_s | -1.77e-05 | -2.00e-05 | -2.30e-06 | 2.79e-04 | 3.67e-04 | 9.73e-04 | 1.35e-03 |
| M_W/M_Z | +1.89e-04 | +1.95e-04 | +6.00e-06 | 2.53e-03 | 3.74e-03 | 2.27e-02 | 2.81e-02 |
| Omega_Lambda | +7.47e-05 | +9.49e-05 | +2.02e-05 | 9.98e-04 | 2.34e-03 | 9.21e-03 | 1.25e-02 |
| Omega_DM_h2 | -3.43e-03 | -3.37e-03 | +6.00e-05 | 4.52e-02 | 4.33e-02 | 2.54e-01 | 2.67e-01 |
| sigma_8 | +3.66e-04 | +3.27e-04 | -3.90e-05 | 2.95e-03 | 3.06e-03 | 2.42e-02 | 2.18e-02 |
| |V_us| | -6.38e-04 | -6.42e-04 | -4.00e-06 | 8.47e-03 | 8.04e-03 | 7.67e-02 | 4.91e-02 |
| |V_cb| | +1.50e-04 | +4.25e-04 | +2.75e-04 | 2.06e-03 | 2.48e-02 | 1.85e-02 | 2.10e-01 |
| Ising2D_gamma | -2.45e-04 | -2.42e-04 | +3.00e-06 | 3.19e-03 | 2.44e-03 | 2.87e-02 | 1.46e-02 |
| m_t/m_W | +2.62e-04 | +3.20e-04 | +5.80e-05 | 1.79e-03 | 2.50e-03 | 1.31e-02 | 6.30e-03 |
| m_H/m_W | +1.52e-04 | +1.52e-04 | +0.00e+00 | 1.97e-03 | 2.48e-03 | 1.77e-02 | 1.86e-02 |
| Twin_prime_C2 | +2.79e-04 | +3.22e-04 | +4.30e-05 | 1.97e-03 | 2.69e-03 | 1.46e-02 | 1.85e-02 |
| Omega_b_h2 | -2.96e-03 | -3.00e-03 | -4.00e-05 | 4.35e-02 | 4.36e-02 | 2.32e-01 | 2.59e-01 |

## Phase 3c/3d: LOCK5 (headroom, exact rewrites, rectifier) and LOCK6 (tiny terms carried)

Locks: `phase3/locked5/LOCK5.md` (+ Amendment 5.1), `phase3/locked6/LOCK6.md` (+ Amendment 6.1), hashes recorded and pushed before simulation.
Runs: `phase3/tools/batch5.sh`, `batch6.sh`; summaries `phase3/results/leaves5_*.tsv`, `leaves6_*.tsv`, `tiny_terms6_fold.tsv`, `tiny_terms6_bench.tsv`.

LOCK5 vs LOCK4, every compiled netlist (ideal 0/27/50 °C, mcC seeds 1–8, mcB seeds 1–4; 2a_X LOCK5 failed in the ideal run, so its MC was not run under LOCK5; see LOCK6):

| Netlist | ideal 27 °C median / worst (LOCK5) | mcC conv LOCK4 → LOCK5 | mcC median σ LOCK4 → LOCK5 | mcB conv LOCK4 → LOCK5 | mcB median σ LOCK4 → LOCK5 |
|---|---|---|---|---|---|
| GEN | 1.45e-05 / 1.37e-03 (BETA) | 7/8 → 8/8 | 1.80e-03 → 5.22e-03 | 3/4 → 3/4 | 1.57e-02 → 2.82e-02 |
| T1 | 2.09e-04 / 3.38e-03 (Omega_DM_h2) | 8/8 → 8/8 | 2.60e-03 → 4.44e-03 | 3/4 → 4/4 | 1.86e-02 → 4.57e-02 |
| 2_M | 2.61e-05 / 1.24e-03 (eta_baryon_photon) | 8/8 → 8/8 | 2.36e-03 → 3.69e-03 | 3/4 → 4/4 | 1.67e-02 → 3.42e-02 |
| 2_P | 1.41e-05 / 5.65e-04 (Hashing_bound) | 8/8 → 8/8 | 1.95e-03 → 4.82e-03 | 4/4 → 4/4 | 1.49e-02 → 3.33e-02 |
| 2_T | 1.74e-06 / 5.17e-05 (Water_triple_K) | 8/8 → 8/8 | 1.17e-03 → 1.88e-03 | 4/4 → 4/4 | 9.98e-03 → 1.76e-02 |
| 2_X | 9.55e-06 / 1.29e-04 (tau_reion) | 8/8 → 8/8 | 2.13e-03 → 5.07e-03 | 4/4 → 3/4 | 1.97e-02 → 3.10e-02 |
| 2a_M | 8.22e-06 / 1.32e-04 (BP_C₂H₅OH) | 8/8 → 8/8 | 1.04e-03 → 1.82e-03 | 4/4 → 4/4 | 7.36e-03 → 1.65e-02 |
| 2a_P | 1.01e-05 / 4.58e-04 (gamma_2_Stieltjes) | 8/8 → 8/8 | 1.31e-03 → 2.14e-03 | 4/4 → 4/4 | 1.18e-02 → 1.79e-02 |
| 2a_Q | 3.19e-06 / 5.98e-06 (N_Lobes) | 8/8 → 8/8 | 2.25e-04 → 2.18e-04 | 4/4 → 4/4 | 5.22e-04 → 6.18e-04 |
| 2a_T | 1.32e-05 / 3.79e-04 (gamma1_Stieltjes) | 8/8 → 8/8 | 2.15e-03 → 3.55e-03 | 3/4 → 3/4 | 1.72e-02 → 2.36e-02 |
| 2a_X | - / - (-) | 0/8 → 0/8 | - → - | 0/4 → 0/4 | - → - |

LOCK6 reruns (netgen5: log-carried base-engine taps in T1's channels, low-leakage rectifier in 2a_X) — `leaves6_netlists.tsv`:
| netlist | leaves | ideal27_median_abs | ideal27_worst | ideal27_worst_leaf | ideal_0..50_worst | mcC_runs_ok | mcC_median_sd | mcC_worst | mcB_runs_ok | mcB_median_sd | mcB_worst |
|---|---|---|---|---|---|---|---|---|---|---|---|
| T1 | 14 | 2.09e-04 | 3.38e-03 | Omega_DM_h2 | 4.13e-03 | 7/8 | 6.26e-03 | 5.51e-02 | 4/4 | 4.98e-02 | 6.03e-01 |
| 2a_X | 44 | 1.99e-05 | 2.19e-01 | Chain_consistency_% | 2.38e-01 | 2/8 | 9.39e-04 | 1.33e+01 | 0/4 | - | - |

- |V_cb| (exact rewrite): mcC σ 2.48e-2 → 8.1e-3, mcB σ 0.21 → 0.048 (LOCK4 → LOCK5).
- Chain_consistency_%: ideal converges (LOCK6, 0/27/50 °C), reads 0.0052 % vs 0.00667 %; MC resolution of the difference node σ ≈ 0.9 (mcC) / 5.4 (mcB) in Chain-% units: consistent to better than ~1 %.
- Base-engine tiny terms (247 taps, 35 folds): carried readout vs engine within 1.1e-4 … 1.8e-3 in the full ideal fold channel (a3 = T3 valve at 8e-18…2.4e-17: +8.8e-4), 1.1–4.0e-4 on the bench at 0/27/50 °C. Current-mode carriers (uncascoded stage, divided-back current, charge integrator) hit a leakage floor below ~1e-13 A. Details: INSIGHTS §14.
- (Superseded by LOCK8 below: the 13 domain_scalar forms, fsot_correct and the hub coefficients are compiled.) Not yet compiled at LOCK6: the 13 domain_scalar forms (S(Human) … Cross_Astro_PS) — FSOT gives them a physical input, the fold channel's S of the named domain, which the base engine already computes; compiling them needs one CHAN per named domain (7 domains) in a forked compiler. fsot_correct, the floor rows and the hub dynamics are not started.

## LOCK7 / 7.1 / 7.2 (refinements 1–3), LOCK8 (part C) and 8.1

Locks: `phase3/locked7/LOCK7.md`, `locked7b/LOCK7b.md` (7.1), `locked7c/LOCK7c.md` (7.2), `locked8/LOCK8.md`, `locked8b/LOCK8b.md` (8.1), all hashed and pushed before simulation.
Runs: `phase3/tools/batch7b.sh`, `batch8.sh`, `batch8b.sh`; summaries `phase3/results/leaves7b_*.tsv`, `leaves8_*.tsv` (C_S, C_H), `leaves8b_*.tsv` (8.1 MC reruns).

**Leaf count: 379 rows, 366 compiled + 13 literal (tier 0), 0 not compiled** (the 13 domain_scalar forms are compiled in C_S; 16 part-C rows added: 10 fsot_correct factors, 6 hub coefficients). Every netlist converges in every run (ideal 0/27/50 °C, mcC 1–8, mcB 1–4).

| Netlist | leaves | ideal 27 °C median / worst | mcC median σ (LOCK5 → 7.1 / 8.1) | mcB median σ (LOCK5 → 7.1 / 8.1) |
|---|---|---|---|---|
| GEN | 20 | 5.0e-05 / 6.4e-03 (BETA) | 5.22e-03 → 1.89e-03 / 1.72e-03 | 2.82e-02 → 1.39e-02 / 1.12e-02 |
| GENpp | 1 | 8.5e-03 (BETA_pp) | – → 4.16e-01 / 4.21e-01 | – → 9.53 / 9.58 |
| T1 | 14 | 7.8e-04 / 1.12e-02 (Omega_DM_h2) | 4.44e-03 → 6.19e-03 / 6.25e-03 | 4.57e-02 → 2.94e-02 / 2.98e-02 |
| 2_M | 44 | 6.7e-05 / 9.0e-04 (eta_baryon_photon) | 3.69e-03 → 2.39e-03 / 2.39e-03 | 3.42e-02 → 1.46e-02 / 1.42e-02 |
| 2_P | 49 | 5.9e-05 / 6.8e-04 (Hashing_bound) | 4.82e-03 → 2.41e-03 / 2.41e-03 | 3.33e-02 → 1.51e-02 / 1.51e-02 |
| 2_T | 5 | 5.0e-05 / 1.0e-04 (Water_triple_K) | 1.88e-03 → 1.66e-03 | 1.76e-02 → 1.04e-02 |
| 2_X | 14 | 6.2e-05 / 4.1e-04 (tau_reion) | 5.07e-03 → 2.54e-03 | 3.10e-02 → 1.12e-02 |
| 2a_M | 66 | 3.5e-05 / 2.7e-04 (BP_C₂H₅OH) | 1.82e-03 → 1.61e-03 | 1.65e-02 → 1.06e-02 |
| 2a_P | 80 | 3.2e-05 / 8.7e-04 (gamma_2_Stieltjes) | 2.14e-03 → 1.66e-03 | 1.79e-02 → 0.96e-02 |
| 2a_T | 12 | 5.9e-05 / 2.7e-04 (Perc_SC_site) | 3.55e-03 → 2.02e-03 | 2.36e-02 → 1.34e-02 |
| 2a_Q | 9 | 3.2e-06 / 6.0e-06 (N_Lobes) | 2.18e-04 → 2.18e-04 | 6.18e-04 → 6.18e-04 |
| 2a_X | 44 | 4.3e-05 / 1.93 (Chain_consistency_%) | – → 2.26e-03 / 2.28e-03 | – → 1.10e-02 / 1.09e-02 |
| C_S (LOCK8) | 23 | 3.1e-05 / 5.5e-04 (S(species)) | 1.97e-03 | 1.05e-02 |
| C_H (LOCK8) | 6 | 4.3e-05 / 2.4e-04 (kappa) | 3.34e-03 | 2.22e-02 |

- Ideal errors under 7.1 are 3–5× LOCK5's (antilog input absorption, base S offset +5.4e-4); MC spread is 1.5–3× smaller. See INSIGHTS §15.
- Part C: S(species) +5.5e-4 is the base-engine S offset; the Cross_* ratios cancel it (+2–3e-5). fsot_correct factors −2.5e-6; hub coefficients −1.6e-5 … +2.4e-4.

## LOCK9A/9B/9C ablations (why 7.1 is less accurate) and the 0-D hub bench

Ideal 27 °C, 348 leaves, against the LOCK5/6 CI baseline (`phase3/results/ablation9*.tsv`; details in INSIGHTS §16). Median ratio / leaves over the 2× rule:
- 7.1 on the LOCK7 base 2.97 / 234; 7.1 on the LOCK6 base 1.24 / 108. Cause: the LOCK7 PTAT neper ratio 256 (ratio 16: 1.36 / 121), plus KD for T1.
- Reverting one compiler change on the LOCK6 base: flip 1.13 / 71, absorption 1.01 / 75, base cells 1.00 / 62; Early correction and range sizing: no effect.
- High-injection compensation (leafc10 `hi`): LOCK7 base 1.68 / 153, LOCK6 base 0.98 / 120; with the LOCK5 compiler forms on the LOCK6 base 1.06 / 109.

No variant brings every leaf to LOCK5's level while keeping 7.1's spread, so the baseline stays LOCK5/6 and no 9-series netlist replaces a LOCK7/8 netlist.

Hub bench (`phase3/results/hub9b_summary.tsv`): the compiled γ_rel, μ(12), μ(25), J_obs and S_eq drive a transistor-level translinear integrator bench. It tracks the exact relaxation to ≤ 1.2e-4 (state units) over 8 hub units, and to 5.4e-4 for S, set by the S_eq channel offset. Python and C++ references agree to 1e-15.

## Full map (356 rows)

| # | section | name | formula | S inputs | constants | analog block | tier | status |
|---|---|---|---|---|---|---|---|---|
| 1 | wave1 | alpha_s(M_Z) | 1/(e·π) | - | E,PI | LOGR+SUM+CAL (log-domain product/power) | 2a | P3b 2a_M: ideal27 +5.75e-07, mcC σ 8.55e-04 (8/8 conv) |
| 2 | wave1 | H0 | 100·(1 + S_cosm·A_bleed/A_in) | S_COSM | A_BLEED,A_IN | LOGR+SUM+CAL (log-domain product/power) + current-mode summing TIA | 1  | P3b T1: ideal27 -1.86e-04, mcC σ 1.55e-03 (8/8 conv) |
| 3 | wave1 | T_CMB | φ² + P_base·\|S_cosm\| | S_COSM | PHI,P_BASE | LOGR+SUM+CAL (log-domain product/power) + current-mode summing TIA + precision full-wave | 1  | P3b T1: ideal27 +2.95e-05, mcC σ 9.64e-04 (8/8 conv) |
| 4 | wave1 | n_s | 1 + S_cosm·C_cosm·φ^(1/π) | S_COSM | C_COSM,PHI,PI | LOGR+SUM+CAL (log-domain product/power) + current-mode summing TIA | 1  | P3b T1: ideal27 -2.00e-05, mcC σ 3.67e-04 (8/8 conv) |
| 5 | wave1 | Omega_b_h2 | \|S_cosm\|·(1 − S_chem) | S_CHEM,S_COSM | - | LOGR+SUM+CAL (log-domain product/power) + current-mode summing TIA + precision full-wave | 1  | P3b T1: ideal27 -3.00e-03, mcC σ 4.36e-02 (8/8 conv) |
| 6 | validation_suite | alpha_FSOT | ln(π)/(e·φ¹³) | - | ALPHA | buffer of a generated constant | 2  | P3b 2_M: ideal27 -4.05e-05, mcC σ 7.09e-03 (8/8 conv) |
| 7 | validation_suite | Tetrahedral_refined | acos(−1/3)·180/π | - | PI | LOGR+SUM+CAL (log-domain product/power) + inverse trig: servo around trig series (new block) | 2a | P3b 2a_M: ideal27 -4.78e-08, mcC σ 8.12e-04 (8/8 conv) |
| 8 | validation_suite | Tetrahedral_FSOT | acos(−γ/2)·180/π | - | GAMMA,PI | LOGR+SUM+CAL (log-domain product/power) + inverse trig: servo around trig series (new block) | 2a | P3b 2a_T: ideal27 +1.93e-05, mcC σ 9.42e-04 (8/8 conv) |
| 9 | validation_suite | Water_bond_angle | (π/φ − 1/(πe))·180/π | - | E,PHI,PI | LOGR+SUM+CAL (log-domain product/power) + current-mode summing TIA | 2a | P3b 2a_P: ideal27 +2.25e-05, mcC σ 1.30e-03 (8/8 conv) |
| 10 | validation_suite | DNA_base_pair | 2πa₀(1 + γ/(π²e)) | - | E,GAMMA,PI | LOGR+SUM+CAL (log-domain product/power) + current-mode summing TIA | 2a | P3b 2a_P: ideal27 +1.18e-05, mcC σ 1.44e-03 (8/8 conv) |
| 11 | validation_suite | Golden_angle | 360/φ² | - | PHI | LOGR+SUM+CAL (log-domain product/power) | 2a | P3b 2a_M: ideal27 +2.70e-06, mcC σ 9.78e-04 (8/8 conv) |
| 12 | validation_suite | Proton_radius | exact | - | - | resistor ratio / literal (no computation) | 0  | planned(literal) |
| 13 | validation_suite | Dark_energy_wa | −γeφ/π | - | E,GAMMA,PHI,PI | LOGR+SUM+CAL (log-domain product/power) | 2a | P3b 2a_M: ideal27 +1.14e-05, mcC σ 6.46e-04 (8/8 conv) |
| 14 | validation_suite | Richardson_D= | (25/D)^0.2 | - | E,GAMMA,PHI,PI | LOGR+SUM+CAL (log-domain product/power) | 2a | planned |
| 15 | validation_suite | sin2_theta_W | (γ − 1/π²)/φ^(3/2) | - | E,GAMMA,PHI,PI | LOGR+SUM+CAL (log-domain product/power) | 2a | P3b 2a_X: ideal27 +2.27e-05, mcC σ - (0/8 conv) |
| 16 | validation_suite | M_Z/M_W | (√π − γ³)/(1/√e + 1/√φ) | - | E,GAMMA,PHI,PI | LOGR+SUM+CAL (log-domain product/power) + current-mode summing TIA | 2a | P3b 2a_P: ideal27 +1.29e-05, mcC σ 1.08e-03 (8/8 conv) |
| 17 | validation_suite | Feigenbaum_delta | (e² − γ^(1/3))/(1/φ + 1/√φ) | - | E,GAMMA,PHI | LOGR+SUM+CAL (log-domain product/power) + current-mode summing TIA | 2a | P3b 2a_X: ideal27 +1.90e-05, mcC σ - (0/8 conv) |
| 18 | validation_suite | Feigenbaum_alpha | (π² − φ³)/(π^(1/3) + 1/√φ) | - | PHI,PI | LOGR+SUM+CAL (log-domain product/power) + current-mode summing TIA | 2a | P3b 2a_X: ideal27 +1.74e-05, mcC σ - (0/8 conv) |
| 19 | validation_suite | Apery_zeta3 | (e − 1/√π)/(1/φ + φ^(1/3)) | - | E,PHI,PI | LOGR+SUM+CAL (log-domain product/power) + current-mode summing TIA | 2a | P3b 2a_X: ideal27 +1.25e-05, mcC σ - (0/8 conv) |
| 20 | validation_suite | Water_anomaly_C | (π³ − γ^(1/3))/(e² + γ³) | - | E,GAMMA,PI | LOGR+SUM+CAL (log-domain product/power) + current-mode summing TIA | 2a | P3b 2a_X: ideal27 +3.20e-05, mcC σ - (0/8 conv) |
| 21 | validation_suite | GUT_coupling | (π²−e²)/(2π²(π+γ)+1/(2e³)) | - | E,GAMMA,PI | LOGR+SUM+CAL (log-domain product/power) + current-mode summing TIA | 2a | P3b 2a_P: ideal27 +2.80e-05, mcC σ 5.21e-03 (8/8 conv) |
| 22 | validation_suite | Identity_pi_4 | π/4 | - | PI | LOGR+SUM+CAL (log-domain product/power) | 2a | P3b 2a_M: ideal27 +1.25e-05, mcC σ 8.09e-04 (8/8 conv) |
| 23 | validation_suite | Identity_2sqrtpi_5 | 2√π/5 | - | PI | LOGR+SUM+CAL (log-domain product/power) | 2a | P3b 2a_M: ideal27 +9.95e-06, mcC σ 8.89e-04 (8/8 conv) |
| 24 | validation_suite | Age_over_T_CMB | 5π√(γφ)/3 | - | GAMMA,PHI,PI | LOGR+SUM+CAL (log-domain product/power) | 2a | P3b 2a_M: ideal27 +1.93e-05, mcC σ 1.79e-03 (8/8 conv) |
| 25 | validation_suite | CMB_asymmetry | γ/(πe) | - | E,GAMMA,PI | LOGR+SUM+CAL (log-domain product/power) | 2a | P3b 2a_M: ideal27 +8.76e-06, mcC σ 9.70e-04 (8/8 conv) |
| 26 | validation_suite | Mantle_Vp_Vs | γ·π | - | GAMMA,PI | LOGR+SUM+CAL (log-domain product/power) | 2a | P3b 2a_M: ideal27 +2.03e-05, mcC σ 1.51e-03 (8/8 conv) |
| 27 | wave2 | 1/alpha_em | e³·φ⁴ − ψ_con | - | E,PHI,PSI_CON | LOGR+SUM+CAL (log-domain product/power) + current-mode summing TIA | 2  | P3b 2_P: ideal27 +2.54e-05, mcC σ 2.87e-03 (8/8 conv) |
| 28 | wave2 | sin2_theta_W | √e·P_new·η_eff | - | E,ETA_EFF,P_NEW | LOGR+SUM+CAL (log-domain product/power) | 2  | P3b 2_M: ideal27 -1.27e-05, mcC σ 1.92e-03 (8/8 conv) |
| 29 | wave2 | M_W/M_Z | S_quant·ln(π) − P_base | S_QUANT | PI,P_BASE | LOGR+SUM+CAL (log-domain product/power) + current-mode summing TIA + LOGR (log of a computed quantity) | 1  | P3b T1: ideal27 +1.95e-04, mcC σ 3.74e-03 (8/8 conv) |
| 30 | wave2 | Omega_Lambda | S_quant/e + γ² | S_QUANT | E,GAMMA | LOGR+SUM+CAL (log-domain product/power) + current-mode summing TIA | 1  | P3b T1: ideal27 +9.49e-05, mcC σ 2.34e-03 (8/8 conv) |
| 31 | wave2 | Omega_m | C_eff·C·ln(π) | - | C_EFF,C_FACTOR,PI | LOGR+SUM+CAL (log-domain product/power) + LOGR (log of a computed quantity) | 2  | P3b 2_X: ideal27 +1.53e-05, mcC σ 5.03e-03 (8/8 conv) |
| 32 | wave2 | Omega_DM_h2 | (1 − S_chem)·φ·A_in | S_CHEM | A_IN,PHI | LOGR+SUM+CAL (log-domain product/power) + current-mode summing TIA | 1  | P3b T1: ideal27 -3.37e-03, mcC σ 4.33e-02 (8/8 conv) |
| 33 | wave2 | sigma_8 | \|S_cosm\|·S_quant + \|Chaos\| | S_COSM,S_QUANT | CHAOS | LOGR+SUM+CAL (log-domain product/power) + current-mode summing TIA + precision full-wave | 1  | P3b T1: ideal27 +3.27e-04, mcC σ 3.06e-03 (8/8 conv) |
| 34 | wave2 | tau_reion | φ·\|Chaos\| − ln(φ) | - | CHAOS,PHI | LOGR+SUM+CAL (log-domain product/power) + current-mode summing TIA + LOGR (log of a computed quantity) + precision full-wave | 2  | P3b 2_X: ideal27 -1.23e-04, mcC σ 1.28e-02 (8/8 conv) |
| 35 | wave2 | m_pi/m_p | K·P_new·ln(π) | - | K,PI,P_NEW | LOGR+SUM+CAL (log-domain product/power) + LOGR (log of a computed quantity) | 2  | P3b 2_X: ideal27 +2.37e-05, mcC σ 1.81e-03 (8/8 conv) |
| 36 | wave2 | N_eff | P_new·e·π + ln(φ) | - | E,PHI,PI,P_NEW | LOGR+SUM+CAL (log-domain product/power) + current-mode summing TIA + LOGR (log of a computed quantity) | 2  | P3b 2_X: ideal27 +1.36e-05, mcC σ 1.42e-03 (8/8 conv) |
| 37 | wave3 | \|V_us\| | θ_S/φ + (1 − S_quant) | S_QUANT | PHI,THETA_S | LOGR+SUM+CAL (log-domain product/power) + current-mode summing TIA | 1  | P3b T1: ideal27 -6.42e-04, mcC σ 8.04e-03 (8/8 conv) |
| 38 | wave3 | \|V_cb\| | S_quant/C_eff − S_quant | S_QUANT | C_EFF | LOGR+SUM+CAL (log-domain product/power) + current-mode summing TIA | 1  | P3b T1: ideal27 +4.25e-04, mcC σ 2.48e-02 (8/8 conv) |
| 39 | wave3 | sin_theta_C | φ/e³ + Suction | - | E,PHI,SUCTION | LOGR+SUM+CAL (log-domain product/power) + current-mode summing TIA | 2  | P3b 2_P: ideal27 -5.93e-05, mcC σ 2.40e-03 (8/8 conv) |
| 40 | wave3 | Age_Gyr | A_in·φ + φ⁵ | - | A_IN,PHI | LOGR+SUM+CAL (log-domain product/power) + current-mode summing TIA | 2  | P3b 2_P: ideal27 +2.61e-05, mcC σ 2.86e-03 (8/8 conv) |
| 41 | wave3 | z_eq | π⁵·φ⁵ | - | PHI,PI | LOGR+SUM+CAL (log-domain product/power) | 2a | P3b 2a_M: ideal27 +6.37e-05, mcC σ 5.77e-03 (8/8 conv) |
| 42 | wave3 | theta_star | P_base·φ − γ² | - | GAMMA,PHI,P_BASE | LOGR+SUM+CAL (log-domain product/power) + current-mode summing TIA | 2  | P3b 2_P: ideal27 +1.47e-05, mcC σ 3.84e-02 (8/8 conv) |
| 43 | wave3 | r_star_Mpc | π³/P_base − φ | - | PHI,PI,P_BASE | LOGR+SUM+CAL (log-domain product/power) + current-mode summing TIA | 2  | P3b 2_P: ideal27 +2.53e-05, mcC σ 2.39e-03 (8/8 conv) |
| 44 | wave3 | Deuteron_binding_MeV | √e/e + φ | - | E,PHI | LOGR+SUM+CAL (log-domain product/power) + current-mode summing TIA | 2a | P3b 2a_P: ideal27 +8.87e-06, mcC σ 1.22e-03 (8/8 conv) |
| 45 | wave3 | Neutron_lifetime_s | π⁷·θ_S | - | PI,THETA_S | LOGR+SUM+CAL (log-domain product/power) | 2  | P3b 2_M: ideal27 +7.36e-05, mcC σ 4.33e-03 (8/8 conv) |
| 46 | wave3 | Ising2D_beta | θ_S/φ⁵ + P_new | - | PHI,P_NEW,THETA_S | LOGR+SUM+CAL (log-domain product/power) + current-mode summing TIA | 2  | P3b 2_P: ideal27 +1.06e-05, mcC σ 9.26e-04 (8/8 conv) |
| 47 | wave3 | Ising2D_nu | √φ/ln(π) − ln(φ) | - | PHI,PI | LOGR+SUM+CAL (log-domain product/power) + current-mode summing TIA + LOGR (log of a computed quantity) | 2a | P3b 2a_X: ideal27 +4.50e-05, mcC σ - (0/8 conv) |
| 48 | wave3 | Ising2D_gamma | (1−S_quant)/ln(φ) + ln(π) | S_QUANT | PHI,PI | LOGR+SUM+CAL (log-domain product/power) + current-mode summing TIA + LOGR (log of a computed quantity) | 1  | P3b T1: ideal27 -2.42e-04, mcC σ 2.44e-03 (8/8 conv) |
| 49 | wave3 | m_t/m_W | \|S_cosm\|/\|Chaos\| + ψ_con | S_COSM | CHAOS,PSI_CON | LOGR+SUM+CAL (log-domain product/power) + current-mode summing TIA + precision full-wave | 1  | P3b T1: ideal27 +3.20e-04, mcC σ 2.50e-03 (8/8 conv) |
| 50 | wave3 | m_H/m_W | S_quant·ψ_con + S_quant | S_QUANT | PSI_CON | LOGR+SUM+CAL (log-domain product/power) + current-mode summing TIA | 1  | P3b T1: ideal27 +1.52e-04, mcC σ 2.48e-03 (8/8 conv) |
| 51 | wave3 | m_tau/m_e | π⁷·ln(π) + e³ | - | E,PI | LOGR+SUM+CAL (log-domain product/power) + current-mode summing TIA + LOGR (log of a computed quantity) | 2a | P3b 2a_X: ideal27 +8.08e-05, mcC σ - (0/8 conv) |
| 52 | wave4 | sin2_theta12 | 2·Poof | - | POOF | LOGR+SUM+CAL (log-domain product/power) | 2  | P3b 2_M: ideal27 -9.89e-05, mcC σ 6.18e-03 (8/8 conv) |
| 53 | wave4 | sin2_theta23 | \|Chaos\|·√e | - | CHAOS,E | LOGR+SUM+CAL (log-domain product/power) + precision full-wave | 2  | P3b 2_M: ideal27 -3.20e-05, mcC σ 1.81e-03 (8/8 conv) |
| 54 | wave4 | sin2_theta13 | γ⁶·φ/e | - | E,GAMMA,PHI | LOGR+SUM+CAL (log-domain product/power) | 2a | P3b 2a_M: ideal27 +4.40e-05, mcC σ 3.19e-03 (8/8 conv) |
| 55 | wave4 | Dm2_21/Dm2_32 | γ³·Poof | - | GAMMA,POOF | LOGR+SUM+CAL (log-domain product/power) | 2  | P3b 2_M: ideal27 -7.73e-05, mcC σ 5.16e-03 (8/8 conv) |
| 56 | wave4 | \|V_ub\| | C_cosm² | - | C_COSM | LOGR+SUM+CAL (log-domain product/power) | 2  | P3b 2_M: ideal27 -1.49e-05, mcC σ 3.28e-03 (8/8 conv) |
| 57 | wave4 | \|V_td\| | √φ/e⁵ | - | E,PHI | LOGR+SUM+CAL (log-domain product/power) | 2a | P3b 2a_M: ideal27 +1.03e-05, mcC σ 7.04e-04 (8/8 conv) |
| 58 | wave4 | \|V_ts\| | γ⁵·ψ_con | - | GAMMA,PSI_CON | LOGR+SUM+CAL (log-domain product/power) | 2  | P3b 2_M: ideal27 +3.96e-05, mcC σ 3.71e-03 (8/8 conv) |
| 59 | wave4 | Jarlskog_J | G/(π⁹) | - | G_CAT,PI | LOGR+SUM+CAL (log-domain product/power) | 2a | P3b 2a_M: ideal27 -3.76e-05, mcC σ 5.24e-03 (8/8 conv) |
| 60 | wave4 | Feigenbaum_delta | ln(π)·(e/G) + √φ | - | E,G_CAT,PHI,PI | LOGR+SUM+CAL (log-domain product/power) + current-mode summing TIA + LOGR (log of a computed quantity) | 2a | P3b 2a_X: ideal27 -2.40e-06, mcC σ - (0/8 conv) |
| 61 | wave4 | Feigenbaum_alpha | \|Chaos\|·φ·e + A_bleed | - | A_BLEED,CHAOS,E,PHI | LOGR+SUM+CAL (log-domain product/power) + current-mode summing TIA + precision full-wave | 2  | P3b 2_P: ideal27 -6.36e-06, mcC σ 1.16e-03 (8/8 conv) |
| 62 | wave4 | r_p_fm | G⁷ + P_new | - | G_CAT,P_NEW | LOGR+SUM+CAL (log-domain product/power) + current-mode summing TIA | 2  | P3b 2_P: ideal27 +3.52e-05, mcC σ 3.98e-03 (8/8 conv) |
| 63 | wave4 | m_n-m_p_MeV | γ⁴/\|Chaos\| + P_var | - | CHAOS,GAMMA,P_VAR | LOGR+SUM+CAL (log-domain product/power) + current-mode summing TIA + precision full-wave | 2  | P3b 2_P: ideal27 +4.56e-06, mcC σ 4.20e-03 (8/8 conv) |
| 64 | wave4 | mu_p_muN | π·(1 − γ⁴) | - | GAMMA,PI | LOGR+SUM+CAL (log-domain product/power) + current-mode summing TIA | 2a | P3b 2a_P: ideal27 +1.12e-05, mcC σ 1.18e-03 (8/8 conv) |
| 65 | wave4 | w0 | −P_new·π/G | - | G_CAT,PI,P_NEW | LOGR+SUM+CAL (log-domain product/power) | 2  | P3b 2_M: ideal27 +1.48e-05, mcC σ 1.53e-03 (8/8 conv) |
| 66 | wave4 | m_c/m_b | C_cosm/P_base | - | C_COSM,P_BASE | LOGR+SUM+CAL (log-domain product/power) | 2  | P3b 2_M: ideal27 -1.45e-05, mcC σ 2.40e-03 (8/8 conv) |
| 67 | wave4 | alpha_s_ratio | π⁶·γ⁸ | - | GAMMA,PI | LOGR+SUM+CAL (log-domain product/power) | 2a | P3b 2a_M: ideal27 +8.32e-05, mcC σ 5.66e-03 (8/8 conv) |
| 68 | wave5 | Gamma_Z/M_Z | φ⁵/e⁶ | - | E,PHI | LOGR+SUM+CAL (log-domain product/power) | 2a | P3b 2a_M: ideal27 +2.49e-05, mcC σ 2.08e-03 (8/8 conv) |
| 69 | wave5 | R_ell | G³/γ⁶ | - | GAMMA,G_CAT | LOGR+SUM+CAL (log-domain product/power) | 2a | P3b 2a_M: ideal27 +6.20e-06, mcC σ 3.01e-03 (8/8 conv) |
| 70 | wave5 | R_b | G/φ³ | - | G_CAT,PHI | LOGR+SUM+CAL (log-domain product/power) | 2a | P3b 2a_M: ideal27 +4.25e-06, mcC σ 1.55e-03 (8/8 conv) |
| 71 | wave5 | R_c | −ln(2) + e/π | - | E,PI | LOGR+SUM+CAL (log-domain product/power) + current-mode summing TIA + LOGR (log of a computed quantity) | 2a | P3b 2a_X: ideal27 +9.26e-05, mcC σ - (0/8 conv) |
| 72 | wave5 | A_FB_ell | A_in/π⁴ | - | A_IN,PI | LOGR+SUM+CAL (log-domain product/power) | 2  | P3b 2_M: ideal27 +1.17e-05, mcC σ 1.72e-03 (8/8 conv) |
| 73 | wave5 | A_ell_SLD | γ³/√φ | - | GAMMA,PHI | LOGR+SUM+CAL (log-domain product/power) | 2a | P3b 2a_M: ideal27 +2.56e-05, mcC σ 1.19e-03 (8/8 conv) |
| 74 | wave5 | m_H/m_t | A_bleed·ln(2) | - | A_BLEED | LOGR+SUM+CAL (log-domain product/power) + LOGR (log of a computed quantity) | 2  | P3b 2_X: ideal27 -4.86e-06, mcC σ 2.97e-03 (8/8 conv) |
| 75 | wave5 | BR_H_bb | C_eff/√e | - | C_EFF,E | LOGR+SUM+CAL (log-domain product/power) | 2  | P3b 2_M: ideal27 +1.46e-05, mcC σ 1.73e-03 (8/8 conv) |
| 76 | wave5 | BR_H_WW | ln(φ)/√5 | - | PHI | LOGR+SUM+CAL (log-domain product/power) + LOGR (log of a computed quantity) | 2a | P3b 2a_X: ideal27 -1.89e-05, mcC σ - (0/8 conv) |
| 77 | wave5 | BR_H_tautau | η_eff/e² | - | E,ETA_EFF | LOGR+SUM+CAL (log-domain product/power) | 2  | P3b 2_M: ideal27 -1.81e-05, mcC σ 1.65e-03 (8/8 conv) |
| 78 | wave5 | Y_p_He4 | θ_S·sin(1) | - | THETA_S | LOGR+SUM+CAL (log-domain product/power) + trig series (powers block) | 2  | P3b 2_M: ideal27 -1.04e-05, mcC σ 1.62e-03 (8/8 conv) |
| 79 | wave5 | D_H_ratio | 1/(π⁴·e⁶) | - | E,PI | LOGR+SUM+CAL (log-domain product/power) | 2a | P3b 2a_M: ideal27 -1.34e-05, mcC σ 2.01e-03 (8/8 conv) |
| 80 | wave5 | Khinchin_K0 | A_bleed/ln(3) + 1/γ | - | A_BLEED,GAMMA | LOGR+SUM+CAL (log-domain product/power) + current-mode summing TIA + LOGR (log of a computed quantity) | 2  | P3b 2_X: ideal27 +1.61e-05, mcC σ 1.12e-03 (8/8 conv) |
| 81 | wave5 | Glaisher_A | G⁴√5 − G/π | - | G_CAT,PI | LOGR+SUM+CAL (log-domain product/power) + current-mode summing TIA | 2a | P3b 2a_P: ideal27 +3.14e-05, mcC σ 2.71e-03 (8/8 conv) |
| 82 | wave5 | Twin_prime_C2 | γ⁴/G⁴ + \|S_cosm\| | S_COSM | GAMMA,G_CAT | LOGR+SUM+CAL (log-domain product/power) + current-mode summing TIA + precision full-wave | 1  | P3b T1: ideal27 +3.22e-04, mcC σ 2.69e-03 (8/8 conv) |
| 83 | wave5 | Mertens_M | −Poof·ln(2) + 1/e | - | E,POOF | LOGR+SUM+CAL (log-domain product/power) + current-mode summing TIA + LOGR (log of a computed quantity) | 2  | P3b 2_X: ideal27 +6.05e-05, mcC σ 2.48e-03 (8/8 conv) |
| 84 | wave5 | Dottie_number | −G⁶/e³ + G³ | - | E,G_CAT | LOGR+SUM+CAL (log-domain product/power) + current-mode summing TIA | 2a | P3b 2a_P: ideal27 +2.56e-05, mcC σ 2.14e-03 (8/8 conv) |
| 85 | wave5 | Omega_constant | Suction/φ⁴ + sin(γ) | - | GAMMA,PHI,SUCTION | LOGR+SUM+CAL (log-domain product/power) + current-mode summing TIA + trig series (powers block) | 2  | P3b 2_T: ideal27 +1.68e-06, mcC σ 1.17e-03 (8/8 conv) |
| 86 | wave5 | Conway_lambda | G⁵ln(3) + φ/e | - | E,G_CAT,PHI | LOGR+SUM+CAL (log-domain product/power) + current-mode summing TIA + LOGR (log of a computed quantity) | 2a | P3b 2a_X: ideal27 +2.73e-05, mcC σ - (0/8 conv) |
| 87 | wave5 | Plastic_number | −G³πγ + e | - | E,GAMMA,G_CAT,PI | LOGR+SUM+CAL (log-domain product/power) + current-mode summing TIA | 2a | P3b 2a_P: ideal27 -2.81e-05, mcC σ 3.44e-03 (8/8 conv) |
| 88 | wave5 | Landau_Ramanujan | −1/(G³π⁵) + G³ | - | G_CAT,PI | LOGR+SUM+CAL (log-domain product/power) + current-mode summing TIA | 2a | P3b 2a_P: ideal27 +2.70e-05, mcC σ 2.10e-03 (8/8 conv) |
| 89 | wave5 | Laplace_limit | G⁸C_cosm + ψ_con | - | C_COSM,G_CAT,PSI_CON | LOGR+SUM+CAL (log-domain product/power) + current-mode summing TIA | 2  | P3b 2_P: ideal27 -9.16e-06, mcC σ 3.86e-04 (8/8 conv) |
| 90 | wave6 | zeta_5 | π/3 − π⁻⁴ | - | PI | LOGR+SUM+CAL (log-domain product/power) + current-mode summing TIA | 2a | P3b 2a_P: ideal27 +1.42e-05, mcC σ 8.64e-04 (8/8 conv) |
| 91 | wave6 | zeta_7 | −√2·cos(1) + √π | - | PI | LOGR+SUM+CAL (log-domain product/power) + current-mode summing TIA + trig series (powers block) | 2a | P3b 2a_P: ideal27 +1.70e-05, mcC σ 1.32e-03 (8/8 conv) |
| 92 | wave6 | Levy_constant | ψ_con·P_base + π | - | PI,PSI_CON,P_BASE | LOGR+SUM+CAL (log-domain product/power) + current-mode summing TIA | 2  | P3b 2_P: ideal27 +1.33e-05, mcC σ 1.03e-03 (8/8 conv) |
| 93 | wave6 | Erdos_Borwein | √2/G⁶ − B_in | - | B_IN,G_CAT | LOGR+SUM+CAL (log-domain product/power) + current-mode summing TIA | 2  | P3b 2_P: ideal27 -4.20e-05, mcC σ 6.47e-03 (8/8 conv) |
| 94 | wave6 | Bernstein | −γ⁵φ/e + 1/π | - | E,GAMMA,PHI,PI | LOGR+SUM+CAL (log-domain product/power) + current-mode summing TIA | 2a | P3b 2a_P: ideal27 -3.52e-06, mcC σ 1.18e-03 (8/8 conv) |
| 95 | wave6 | Backhouse | √π/3 + e/π | - | E,PI | LOGR+SUM+CAL (log-domain product/power) + current-mode summing TIA | 2a | P3b 2a_P: ideal27 +4.77e-06, mcC σ 7.51e-04 (8/8 conv) |
| 96 | wave6 | Viswanath | πγ/φ³ + G⁴ | - | GAMMA,G_CAT,PHI,PI | LOGR+SUM+CAL (log-domain product/power) + current-mode summing TIA | 2a | P3b 2a_P: ideal27 +2.43e-05, mcC σ 2.08e-03 (8/8 conv) |
| 97 | wave6 | Kepler_Bouwkamp | −γln(2) + φ/π | - | GAMMA,PHI,PI | LOGR+SUM+CAL (log-domain product/power) + current-mode summing TIA + LOGR (log of a computed quantity) | 2a | P3b 2a_X: ideal27 +7.29e-05, mcC σ - (0/8 conv) |
| 98 | wave6 | Sphere_packing_3D | √2φ/e − 1/π² | - | E,PHI,PI | LOGR+SUM+CAL (log-domain product/power) + current-mode summing TIA | 2a | P3b 2a_P: ideal27 +1.05e-05, mcC σ 1.08e-03 (8/8 conv) |
| 99 | wave6 | Gauss_AGM | A_bleed − P_base | - | A_BLEED,P_BASE | current-mode summing TIA | 2  | P3b 2_P: ideal27 +2.34e-05, mcC σ 1.33e-03 (8/8 conv) |
| 100 | wave6 | Lemniscate | (e/φ)/G + B_in | - | B_IN,E,G_CAT,PHI | LOGR+SUM+CAL (log-domain product/power) + current-mode summing TIA | 2  | P3b 2_P: ideal27 +7.90e-06, mcC σ 1.00e-03 (8/8 conv) |
| 101 | wave6 | Hashing_bound | −G⁷√3 + A_bleed | - | A_BLEED,G_CAT | LOGR+SUM+CAL (log-domain product/power) + current-mode summing TIA | 2  | P3b 2_P: ideal27 -3.13e-04, mcC σ 4.02e-02 (8/8 conv) |
| 102 | wave6 | von_Karman | A_bleed/φ² | - | A_BLEED,PHI | LOGR+SUM+CAL (log-domain product/power) | 2  | P3b 2_M: ideal27 +1.60e-05, mcC σ 1.81e-03 (8/8 conv) |
| 103 | wave6 | Madelung_NaCl | e/3 + sin(1) | - | E | LOGR+SUM+CAL (log-domain product/power) + current-mode summing TIA + trig series (powers block) | 2a | P3b 2a_P: ideal27 +3.34e-06, mcC σ 5.40e-04 (8/8 conv) |
| 104 | wave6 | Madelung_CsCl | G⁸C_cosm + √3 | - | C_COSM,G_CAT | LOGR+SUM+CAL (log-domain product/power) + current-mode summing TIA | 2  | P3b 2_P: ideal27 -2.52e-06, mcC σ 3.16e-04 (8/8 conv) |
| 105 | wave6 | Methane_angle | e⁴/G⁷ + eπ | - | E,G_CAT,PI | LOGR+SUM+CAL (log-domain product/power) + current-mode summing TIA | 2a | P3b 2a_P: ideal27 -2.32e-05, mcC σ 2.82e-03 (8/8 conv) |
| 106 | wave6 | O_N_electronegativity | e⁻⁴ + G⁶ | - | G_CAT | LOGR+SUM+CAL (log-domain product/power) + current-mode summing TIA + CAL (antilog of a computed exponent) | 2a | P3b 2a_X: ideal27 +3.65e-05, mcC σ - (0/8 conv) |
| 107 | wave6 | Water_max_density_C | ln(π)/C | - | C_FACTOR,PI | LOGR+SUM+CAL (log-domain product/power) + LOGR (log of a computed quantity) | 2  | P3b 2_X: ideal27 -1.04e-05, mcC σ 1.33e-03 (8/8 conv) |
| 108 | wave6 | Perc_sq_site | −C_cosm·sin(1) + G⁵ | - | C_COSM,G_CAT | LOGR+SUM+CAL (log-domain product/power) + current-mode summing TIA + trig series (powers block) | 2  | P3b 2_P: ideal27 +3.96e-05, mcC σ 4.31e-03 (8/8 conv) |
| 109 | wave6 | Polya_3D_return | √π/2 − sin(γ) | - | GAMMA,PI | LOGR+SUM+CAL (log-domain product/power) + current-mode summing TIA + trig series (powers block) | 2a | P3b 2a_T: ideal27 +9.83e-06, mcC σ 2.83e-03 (8/8 conv) |
| 110 | wave6 | Random_walk_CN | G⁷/K − γ⁴ | - | GAMMA,G_CAT,K | LOGR+SUM+CAL (log-domain product/power) + current-mode summing TIA | 2  | P3b 2_P: ideal27 +4.13e-05, mcC σ 5.15e-03 (8/8 conv) |
| 111 | wave6 | Nats_per_bit | −η_eff·cos(φ) + G⁻⁴ | - | ETA_EFF,G_CAT,PHI | LOGR+SUM+CAL (log-domain product/power) + current-mode summing TIA + trig series (powers block) | 2  | P3b 2_T: ideal27 -7.72e-06, mcC σ 1.64e-03 (8/8 conv) |
| 112 | wave7 | Apery_zeta3_w7 | γ³/θ_S + G⁷ | - | GAMMA,G_CAT,THETA_S | LOGR+SUM+CAL (log-domain product/power) + current-mode summing TIA | 2  | P3b 2_P: ideal27 +5.96e-05, mcC σ 3.68e-03 (8/8 conv) |
| 113 | wave7 | Soldner | √5/2 + 1/3 | - | - | resistor ratio / literal (no computation) | 0  | planned(literal) |
| 114 | wave7 | Mills | γ³φ/e + G⁻² | - | E,GAMMA,G_CAT,PHI | LOGR+SUM+CAL (log-domain product/power) + current-mode summing TIA | 2a | P3b 2a_P: ideal27 -1.57e-06, mcC σ 7.81e-04 (8/8 conv) |
| 115 | wave7 | Sierpinski_const | φ³/2 + η_eff | - | ETA_EFF,PHI | LOGR+SUM+CAL (log-domain product/power) + current-mode summing TIA | 2  | P3b 2_P: ideal27 +1.23e-05, mcC σ 1.87e-03 (8/8 conv) |
| 116 | wave7 | Niven | 1/(G³√e) + G | - | E,G_CAT | LOGR+SUM+CAL (log-domain product/power) + current-mode summing TIA | 2a | P3b 2a_P: ideal27 +1.68e-06, mcC σ 5.45e-04 (8/8 conv) |
| 117 | wave7 | Artin | C_eff/φ⁵ + C | - | C_EFF,C_FACTOR,PHI | LOGR+SUM+CAL (log-domain product/power) + current-mode summing TIA | 2  | P3b 2_P: ideal27 +1.99e-05, mcC σ 1.72e-03 (8/8 conv) |
| 118 | wave7 | Hafner_Sarnak | ln(2)/e⁴ + φ/e | - | E,PHI | LOGR+SUM+CAL (log-domain product/power) + current-mode summing TIA + LOGR (log of a computed quantity) | 2a | P3b 2a_X: ideal27 +7.92e-06, mcC σ - (0/8 conv) |
| 119 | wave7 | Porter | 1/(πφ⁴) + G⁻⁴ | - | G_CAT,PHI,PI | LOGR+SUM+CAL (log-domain product/power) + current-mode summing TIA | 2a | P3b 2a_P: ideal27 -1.08e-05, mcC σ 1.56e-03 (8/8 conv) |
| 120 | wave7 | Thue_Morse | C_eff/(eπ) + P_new | - | C_EFF,E,PI,P_NEW | LOGR+SUM+CAL (log-domain product/power) + current-mode summing TIA | 2  | P3b 2_P: ideal27 +1.32e-05, mcC σ 1.08e-03 (8/8 conv) |
| 121 | wave7 | MRB | C/φ⁴ + φ⁻⁴ | - | C_FACTOR,PHI | LOGR+SUM+CAL (log-domain product/power) + current-mode summing TIA | 2  | P3b 2_P: ideal27 +1.51e-06, mcC σ 1.75e-03 (8/8 conv) |
| 122 | wave7 | Gauss_Kuzmin | γ/(eπ) + φ⁻³ | - | E,GAMMA,PHI,PI | LOGR+SUM+CAL (log-domain product/power) + current-mode summing TIA | 2a | P3b 2a_P: ideal27 -1.23e-06, mcC σ 1.45e-03 (8/8 conv) |
| 123 | wave7 | Universal_parabolic | B_in/G⁷ + G² | - | B_IN,G_CAT | LOGR+SUM+CAL (log-domain product/power) + current-mode summing TIA | 2  | P3b 2_P: ideal27 -2.70e-06, mcC σ 2.95e-03 (8/8 conv) |
| 124 | wave7 | Lieb_square_ice | B_in/G⁴ + K | - | B_IN,G_CAT,K | LOGR+SUM+CAL (log-domain product/power) + current-mode summing TIA | 2  | P3b 2_P: ideal27 +1.03e-05, mcC σ 2.29e-03 (8/8 conv) |
| 125 | wave7 | Komornik_Loreti | (π/e)·A_bleed + γ | - | A_BLEED,E,GAMMA,PI | LOGR+SUM+CAL (log-domain product/power) + current-mode summing TIA | 2  | P3b 2_P: ideal27 +2.27e-05, mcC σ 1.53e-03 (8/8 conv) |
| 126 | wave7 | Bloch_Landau | 1/(G²√5) + π⁻⁴ | - | G_CAT,PI | LOGR+SUM+CAL (log-domain product/power) + current-mode summing TIA | 2a | P3b 2a_P: ideal27 -5.03e-06, mcC σ 1.10e-03 (8/8 conv) |
| 127 | wave7 | Golden_angle_deg | (φ/π)/cos(φ) + e⁵ | - | E,PHI,PI | LOGR+SUM+CAL (log-domain product/power) + current-mode summing TIA + trig series (powers block) | 2a | P3b 2a_T: ideal27 +2.41e-05, mcC σ 3.55e-03 (8/8 conv) |
| 128 | wave7 | Ising3D_eta | √φ/π⁵ + 1/π³ | - | PHI,PI | LOGR+SUM+CAL (log-domain product/power) + current-mode summing TIA | 2a | P3b 2a_P: ideal27 -1.45e-05, mcC σ 1.79e-03 (8/8 conv) |
| 129 | wave7 | Ising3D_alpha | Suction − γ⁶ | - | GAMMA,SUCTION | LOGR+SUM+CAL (log-domain product/power) + current-mode summing TIA | 2  | P3b 2_P: ideal27 -1.51e-04, mcC σ 5.34e-03 (8/8 conv) |
| 130 | wave7 | Ising3D_delta | √5/η_eff | - | ETA_EFF | LOGR+SUM+CAL (log-domain product/power) | 2  | P3b 2_M: ideal27 +3.32e-05, mcC σ 2.48e-03 (8/8 conv) |
| 131 | wave7 | XY_nu | G⁴ − 1/π³ | - | G_CAT,PI | LOGR+SUM+CAL (log-domain product/power) + current-mode summing TIA | 2a | P3b 2a_P: ideal27 +2.84e-05, mcC σ 2.54e-03 (8/8 conv) |
| 132 | wave7 | XY_eta | γ⁵φ/e | - | E,GAMMA,PHI | LOGR+SUM+CAL (log-domain product/power) | 2a | P3b 2a_M: ideal27 +4.15e-05, mcC σ 2.55e-03 (8/8 conv) |
| 133 | wave7 | Heisenberg_nu | cos(1)/√γ | - | GAMMA | LOGR+SUM+CAL (log-domain product/power) + trig series (powers block) | 2a | P3b 2a_M: ideal27 +3.20e-06, mcC σ 6.69e-04 (8/8 conv) |
| 134 | wave7 | Heisenberg_eta | C_cosm/√e | - | C_COSM,E | LOGR+SUM+CAL (log-domain product/power) | 2  | P3b 2_M: ideal27 -7.03e-06, mcC σ 1.35e-03 (8/8 conv) |
| 135 | wave7 | Perc_honeycomb_site | π/(3φ) + e⁻³ | - | PHI,PI | LOGR+SUM+CAL (log-domain product/power) + current-mode summing TIA + CAL (antilog of a computed exponent) | 2a | P3b 2a_X: ideal27 +1.17e-05, mcC σ - (0/8 conv) |
| 136 | wave7 | Perc_SC_bond | G⁷/e + e⁻³ | - | E,G_CAT | LOGR+SUM+CAL (log-domain product/power) + current-mode summing TIA + CAL (antilog of a computed exponent) | 2a | P3b 2a_X: ideal27 +4.14e-05, mcC σ - (0/8 conv) |
| 137 | wave7 | Perc_SC_site | cos(φ)/G² + 1/e | - | E,G_CAT,PHI | LOGR+SUM+CAL (log-domain product/power) + current-mode summing TIA + trig series (powers block) | 2a | P3b 2a_T: ideal27 -3.33e-05, mcC σ 7.39e-03 (8/8 conv) |
| 138 | wave7 | m_u/m_d | √3 − √φ | - | PHI | LOGR+SUM+CAL (log-domain product/power) + current-mode summing TIA | 2a | P3b 2a_P: ideal27 -4.28e-05, mcC σ 3.87e-03 (8/8 conv) |
| 139 | wave7 | m_s/m_d | e³ + γ⁴ | - | E,GAMMA | LOGR+SUM+CAL (log-domain product/power) + current-mode summing TIA | 2a | P3b 2a_P: ideal27 +8.70e-06, mcC σ 6.11e-04 (8/8 conv) |
| 140 | wave7 | m_tau/m_mu | π²e/φ + φ⁻³ | - | E,PHI,PI | LOGR+SUM+CAL (log-domain product/power) + current-mode summing TIA | 2a | P3b 2a_P: ideal27 +1.88e-05, mcC σ 2.20e-03 (8/8 conv) |
| 141 | wave8 | \|V_ud\| | A_in − ln(2) | - | A_IN | current-mode summing TIA + LOGR (log of a computed quantity) | 2  | P3b 2_X: ideal27 +6.32e-05, mcC σ 4.15e-03 (8/8 conv) |
| 142 | wave8 | \|V_cd\| | √γ·θ_S | - | GAMMA,THETA_S | LOGR+SUM+CAL (log-domain product/power) | 2  | P3b 2_M: ideal27 -8.01e-06, mcC σ 1.82e-03 (8/8 conv) |
| 143 | wave8 | \|V_cs\| | φγ/P_var | - | GAMMA,PHI,P_VAR | LOGR+SUM+CAL (log-domain product/power) | 2  | P3b 2_M: ideal27 +3.41e-05, mcC σ 5.11e-03 (8/8 conv) |
| 144 | wave8 | delta_CP_PMNS | φ³ − 1/e | - | E,PHI | LOGR+SUM+CAL (log-domain product/power) + current-mode summing TIA | 2a | P3b 2a_P: ideal27 +1.94e-05, mcC σ 3.14e-03 (8/8 conv) |
| 145 | wave8 | m_t/m_b | π⁴·K | - | K,PI | LOGR+SUM+CAL (log-domain product/power) | 2  | P3b 2_M: ideal27 +5.89e-05, mcC σ 3.26e-03 (8/8 conv) |
| 146 | wave8 | BR_Z_ee | γ⁶/ln(3) | - | GAMMA | LOGR+SUM+CAL (log-domain product/power) + LOGR (log of a computed quantity) | 2a | P3b 2a_X: ideal27 +5.33e-05, mcC σ - (0/8 conv) |
| 147 | wave8 | BR_Z_had | sin(γ) + Poof | - | GAMMA,POOF | current-mode summing TIA + trig series (powers block) | 2  | P3b 2_T: ideal27 -1.71e-05, mcC σ 9.35e-04 (8/8 conv) |
| 148 | wave8 | BR_Z_inv | (1/3)/A_in | - | A_IN | LOGR+SUM+CAL (log-domain product/power) | 2  | P3b 2_M: ideal27 -1.29e-05, mcC σ 2.12e-03 (8/8 conv) |
| 149 | wave8 | BR_H_ZZ | 1/(e⁴ln(2)) | - | E | LOGR+SUM+CAL (log-domain product/power) + LOGR (log of a computed quantity) | 2a | P3b 2a_X: ideal27 +3.50e-05, mcC σ - (0/8 conv) |
| 150 | wave8 | BR_H_gg | φ⁻⁴ − γ⁵ | - | GAMMA,PHI | LOGR+SUM+CAL (log-domain product/power) + current-mode summing TIA | 2a | P3b 2a_P: ideal27 -4.52e-05, mcC σ 7.39e-03 (8/8 conv) |
| 151 | wave8 | BR_H_cc | Suction/(πφ) | - | PHI,PI,SUCTION | LOGR+SUM+CAL (log-domain product/power) | 2  | P3b 2_M: ideal27 -1.08e-04, mcC σ 4.99e-03 (8/8 conv) |
| 152 | wave8 | BR_H_gamgam | γ⁶·C_cosm | - | C_COSM,GAMMA | LOGR+SUM+CAL (log-domain product/power) | 2  | P3b 2_M: ideal27 +2.88e-05, mcC σ 3.25e-03 (8/8 conv) |
| 153 | wave8 | BR_H_Zgam | η_eff/π⁵ | - | ETA_EFF,PI | LOGR+SUM+CAL (log-domain product/power) | 2  | P3b 2_M: ideal27 -5.79e-05, mcC σ 6.40e-03 (8/8 conv) |
| 154 | wave8 | He4_binding_MeV | πγ/γ⁵ | - | GAMMA,PI | LOGR+SUM+CAL (log-domain product/power) | 2a | P3b 2a_M: ideal27 -3.95e-06, mcC σ 2.10e-03 (8/8 conv) |
| 155 | wave8 | Triton_binding_MeV | e² + G⁻¹ | - | E,G_CAT | LOGR+SUM+CAL (log-domain product/power) + current-mode summing TIA | 2a | P3b 2a_P: ideal27 +6.32e-06, mcC σ 8.33e-04 (8/8 conv) |
| 156 | wave8 | Deuteron_mu_muN | G⁴ + Poof | - | G_CAT,POOF | LOGR+SUM+CAL (log-domain product/power) + current-mode summing TIA | 2  | P3b 2_P: ideal27 +6.48e-06, mcC σ 2.09e-03 (8/8 conv) |
| 157 | wave8 | S_8 | ψ_con/√γ | - | GAMMA,PSI_CON | LOGR+SUM+CAL (log-domain product/power) | 2  | P3b 2_M: ideal27 -2.65e-06, mcC σ 7.93e-04 (8/8 conv) |
| 158 | wave8 | z_reion | e³/φ² | - | E,PHI | LOGR+SUM+CAL (log-domain product/power) | 2a | P3b 2a_M: ideal27 +6.72e-07, mcC σ 6.71e-04 (8/8 conv) |
| 159 | wave8 | XY_beta | ψ_con/(πγ) | - | GAMMA,PI,PSI_CON | LOGR+SUM+CAL (log-domain product/power) | 2  | P3b 2_M: ideal27 -8.94e-06, mcC σ 1.58e-03 (8/8 conv) |
| 160 | wave8 | XY_gamma | φ − P_new | - | PHI,P_NEW | current-mode summing TIA | 2  | P3b 2_P: ideal27 +8.43e-06, mcC σ 1.30e-03 (8/8 conv) |
| 161 | wave8 | Heisenberg_beta | φ/π − φ⁻⁴ | - | PHI,PI | LOGR+SUM+CAL (log-domain product/power) + current-mode summing TIA | 2a | P3b 2a_P: ideal27 +6.38e-06, mcC σ 2.91e-03 (8/8 conv) |
| 162 | wave8 | Heisenberg_gamma | √2 − e⁻⁴ | - | - | resistor ratio / literal (no computation) | 0  | P3b 2a_X: ideal27 -3.55e-06, mcC σ - (0/8 conv) |
| 163 | wave8 | Perc_BCC_bond | G⁷/3 | - | G_CAT | LOGR+SUM+CAL (log-domain product/power) | 2a | P3b 2a_M: ideal27 +5.11e-05, mcC σ 2.80e-03 (8/8 conv) |
| 164 | wave8 | Perc_FCC_site | 1/3 − e⁻² | - | - | resistor ratio / literal (no computation) | 0  | P3b 2a_X: ideal27 -1.07e-05, mcC σ - (0/8 conv) |
| 165 | wave8 | Perc3D_nu | sin(γ) − Chaos | - | CHAOS,GAMMA | current-mode summing TIA + trig series (powers block) | 2  | P3b 2_T: ideal27 -3.31e-06, mcC σ 1.07e-03 (8/8 conv) |
| 166 | wave8 | Perc3D_beta | √π/φ³ | - | PHI,PI | LOGR+SUM+CAL (log-domain product/power) | 2a | P3b 2a_M: ideal27 +1.46e-06, mcC σ 1.55e-03 (8/8 conv) |
| 167 | wave8 | Perc3D_gamma | γ⁷ + √π | - | GAMMA,PI | LOGR+SUM+CAL (log-domain product/power) + current-mode summing TIA | 2a | P3b 2a_P: ideal27 +1.12e-05, mcC σ 8.32e-04 (8/8 conv) |
| 168 | wave8 | Brun_B2 | γ² + eγ | - | E,GAMMA | LOGR+SUM+CAL (log-domain product/power) + current-mode summing TIA | 2a | P3b 2a_P: ideal27 +1.42e-05, mcC σ 1.35e-03 (8/8 conv) |
| 169 | wave8 | Copeland_Erdos | (1/3)/√2 | - | - | resistor ratio / literal (no computation) | 0  | planned(literal) |
| 170 | wave8 | Erdos_Tenenbaum_Ford | γ³/√5 | - | GAMMA | LOGR+SUM+CAL (log-domain product/power) | 2a | P3b 2a_M: ideal27 +2.76e-05, mcC σ 1.75e-03 (8/8 conv) |
| 171 | wave8 | Foias_alpha | eγ − φ⁻² | - | E,GAMMA,PHI | LOGR+SUM+CAL (log-domain product/power) + current-mode summing TIA | 2a | P3b 2a_P: ideal27 +1.31e-05, mcC σ 1.74e-03 (8/8 conv) |
| 172 | wave8 | Madelung_ZnS | eγ/C_eff | - | C_EFF,E,GAMMA | LOGR+SUM+CAL (log-domain product/power) | 2  | P3b 2_M: ideal27 +7.93e-06, mcC σ 1.20e-03 (8/8 conv) |
| 173 | wave8 | Madelung_CaF2 | G⁻⁴ + ln(3) | - | G_CAT | LOGR+SUM+CAL (log-domain product/power) + current-mode summing TIA + LOGR (log of a computed quantity) | 2a | P3b 2a_X: ideal27 -1.17e-05, mcC σ - (0/8 conv) |
| 174 | wave8 | Madelung_TiO2 | G² + eγ | - | E,GAMMA,G_CAT | LOGR+SUM+CAL (log-domain product/power) + current-mode summing TIA | 2a | P3b 2a_P: ideal27 +1.47e-05, mcC σ 1.27e-03 (8/8 conv) |
| 175 | wave8 | Ice_Ih_density | sin(γ)·e/φ | - | E,GAMMA,PHI | LOGR+SUM+CAL (log-domain product/power) + trig series (powers block) | 2a | P3b 2a_T: ideal27 +1.95e-05, mcC σ 9.05e-04 (8/8 conv) |
| 176 | wave8 | H2O_bond_angle | e³/γ³ | - | E,GAMMA | LOGR+SUM+CAL (log-domain product/power) | 2a | P3b 2a_M: ideal27 -1.09e-05, mcC σ 1.47e-03 (8/8 conv) |
| 177 | wave8 | Lorenz_dim | √φ + B_in | - | B_IN,PHI | LOGR+SUM+CAL (log-domain product/power) + current-mode summing TIA | 2  | P3b 2_P: ideal27 +1.39e-05, mcC σ 1.02e-03 (8/8 conv) |
| 178 | wave8 | Henon_dim | P_var/√γ | - | GAMMA,P_VAR | LOGR+SUM+CAL (log-domain product/power) | 2  | P3b 2_M: ideal27 -1.01e-05, mcC σ 5.44e-03 (8/8 conv) |
| 179 | wave8 | Apollonian_dim | ln(3)/sin(1) | - | - | LOGR+SUM+CAL (log-domain product/power) + LOGR (log of a computed quantity) + trig series (powers block) | 0  | P3b 2a_X: ideal27 -5.31e-06, mcC σ - (0/8 conv) |
| 180 | wave8 | SAW_mu_sq | √π + e/π | - | E,PI | LOGR+SUM+CAL (log-domain product/power) + current-mode summing TIA | 2a | P3b 2a_P: ideal27 +8.34e-06, mcC σ 6.97e-04 (8/8 conv) |
| 181 | wave8 | SAW_nu_3D | cos(1) − cos(φ) | - | PHI | current-mode summing TIA + trig series (powers block) | 2a | P3b 2a_T: ideal27 +1.24e-05, mcC σ 3.48e-03 (8/8 conv) |
| 182 | wave8 | SAW_gamma_3D | G⁻² − π⁻³ | - | G_CAT,PI | LOGR+SUM+CAL (log-domain product/power) + current-mode summing TIA | 2a | P3b 2a_P: ideal27 -5.69e-06, mcC σ 5.41e-04 (8/8 conv) |
| 183 | wave8 | Potts3_beta | 1/9 | - | - | resistor ratio / literal (no computation) | 0  | planned(literal) |
| 184 | wave8 | KT_T/J | A_bleed − Poof | - | A_BLEED,POOF | current-mode summing TIA | 2  | P3b 2_P: ideal27 +4.24e-05, mcC σ 1.00e-03 (8/8 conv) |
| 185 | wave8 | Quark_condensate | 1/4 | - | - | resistor ratio / literal (no computation) | 0  | planned(literal) |
| 186 | wave8 | Figure8_knot_vol | G⁻² + cos(γ) | - | GAMMA,G_CAT | LOGR+SUM+CAL (log-domain product/power) + current-mode summing TIA + trig series (powers block) | 2a | P3b 2a_T: ideal27 -8.37e-06, mcC σ 9.39e-04 (8/8 conv) |
| 187 | wave8 | Bessel_J0_zero1 | A_in/ln(2) | - | A_IN | LOGR+SUM+CAL (log-domain product/power) + LOGR (log of a computed quantity) | 2  | P3b 2_X: ideal27 +5.65e-05, mcC σ 2.19e-03 (8/8 conv) |
| 188 | wave8 | Airy_Ai_zero1 | (1/G)/η_eff | - | ETA_EFF,G_CAT | LOGR+SUM+CAL (log-domain product/power) | 2  | P3b 2_M: ideal27 +2.52e-05, mcC σ 2.31e-03 (8/8 conv) |
| 189 | wave8 | gamma_2_Stieltjes | π⁻² − γ⁴ | - | GAMMA,PI | LOGR+SUM+CAL (log-domain product/power) + current-mode summing TIA | 2a | P3b 2a_P: ideal27 +4.04e-04, mcC σ 4.08e-02 (8/8 conv) |
| 190 | wave8 | First_Riemann_zero | e/γ³ | - | E,GAMMA | LOGR+SUM+CAL (log-domain product/power) | 2a | P3b 2a_M: ideal27 -9.80e-06, mcC σ 1.29e-03 (8/8 conv) |
| 191 | wave8 | Spanning_tree_sq | γ⁷ + ln(π) | - | GAMMA,PI | LOGR+SUM+CAL (log-domain product/power) + current-mode summing TIA + LOGR (log of a computed quantity) | 2a | P3b 2a_X: ideal27 -2.97e-06, mcC σ - (0/8 conv) |
| 192 | wave8 | Hard_sq_entropy | eγ·P_var | - | E,GAMMA,P_VAR | LOGR+SUM+CAL (log-domain product/power) | 2  | P3b 2_M: ideal27 -1.22e-06, mcC σ 5.21e-03 (8/8 conv) |
| 193 | wave9 | \|V_tb\| | cos(1/e)/(φγ) | - | E,GAMMA,PHI | LOGR+SUM+CAL (log-domain product/power) + trig series (powers block) | 2a | P3b 2a_T: ideal27 -4.57e-07, mcC σ 8.63e-04 (8/8 conv) |
| 194 | wave9 | Omega_r | γ⁶/e⁶ | - | E,GAMMA | LOGR+SUM+CAL (log-domain product/power) | 2a | P3b 2a_M: ideal27 +3.44e-05, mcC σ 2.89e-03 (8/8 conv) |
| 195 | wave9 | O4_nu | (γ/G)/sin(1) | - | GAMMA,G_CAT | LOGR+SUM+CAL (log-domain product/power) + trig series (powers block) | 2a | P3b 2a_M: ideal27 +8.01e-06, mcC σ 8.44e-04 (8/8 conv) |
| 196 | wave9 | Gluon_condensate | C_cosm − e⁻³ | - | C_COSM | current-mode summing TIA + CAL (antilog of a computed exponent) | 2  | P3b 2_X: ideal27 -5.73e-05, mcC σ 8.53e-03 (8/8 conv) |
| 197 | wave9 | Mandelbrot_boundary | φ + 1/φ² | - | PHI | LOGR+SUM+CAL (log-domain product/power) + current-mode summing TIA | 2a | P3b 2a_P: ideal27 +8.94e-06, mcC σ 8.72e-04 (8/8 conv) |
| 198 | wave9 | Sierpinski_dim | ln(3)/ln(2) | - | - | resistor ratio / literal (no computation) | 0  | P3b 2a_X: ideal27 +2.68e-05, mcC σ - (0/8 conv) |
| 199 | wave9 | gamma1_Stieltjes | G² + cos(e) | - | E,G_CAT | LOGR+SUM+CAL (log-domain product/power) + current-mode summing TIA + trig series (powers block) | 2a | P3b 2a_T: ideal27 -1.38e-04, mcC σ 4.88e-02 (8/8 conv) |
| 200 | wave10 | m_mu/m_e | (π³ − G²)·φ⁴ | - | G_CAT,PHI,PI | LOGR+SUM+CAL (log-domain product/power) + current-mode summing TIA | 2a | P3b 2a_P: ideal27 +6.27e-05, mcC σ 5.01e-03 (8/8 conv) |
| 201 | wave10 | (g-2)/2_electron | (e/π − ln2)/e⁵ | - | E,PI | LOGR+SUM+CAL (log-domain product/power) + current-mode summing TIA + LOGR (log of a computed quantity) | 2a | P3b 2a_X: ideal27 +1.04e-04, mcC σ - (0/8 conv) |
| 202 | wave10 | Logistic_accum | e√e + cos(e) | - | E | LOGR+SUM+CAL (log-domain product/power) + current-mode summing TIA + trig series (powers block) | 2a | P3b 2a_T: ideal27 +5.20e-06, mcC σ 1.35e-03 (8/8 conv) |
| 203 | wave10 | Cahen_constant | (C_eff + K)·η_eff | - | C_EFF,ETA_EFF,K | LOGR+SUM+CAL (log-domain product/power) + current-mode summing TIA | 2  | P3b 2_P: ideal27 +8.05e-06, mcC σ 9.11e-04 (8/8 conv) |
| 204 | wave10 | Reciprocal_Fib | (Poof + C_cosm)/γ⁵ | - | C_COSM,GAMMA,POOF | LOGR+SUM+CAL (log-domain product/power) + current-mode summing TIA | 2  | P3b 2_P: ideal27 -8.14e-05, mcC σ 4.70e-03 (8/8 conv) |
| 205 | wave10 | Water_triple_K | π⁵sin(π/φ)·C_eff | - | C_EFF,PHI,PI | LOGR+SUM+CAL (log-domain product/power) + trig series (powers block) | 2  | P3b 2_T: ideal27 +4.47e-05, mcC σ 6.94e-03 (8/8 conv) |
| 206 | wave10 | CO2_bond_angle | π⁴/cos(1) − C | - | C_FACTOR,PI | LOGR+SUM+CAL (log-domain product/power) + current-mode summing TIA + trig series (powers block) | 2  | P3b 2_P: ideal27 +3.66e-05, mcC σ 2.37e-03 (8/8 conv) |
| 207 | wave10 | SAW_connective_hex | π/A_in − γ⁶ | - | A_IN,GAMMA,PI | LOGR+SUM+CAL (log-domain product/power) + current-mode summing TIA | 2  | P3b 2_P: ideal27 -5.67e-06, mcC σ 2.13e-03 (8/8 conv) |
| 208 | wave10 | Bessel_J1_zero1 | ψ_con/G + π | - | G_CAT,PI,PSI_CON | LOGR+SUM+CAL (log-domain product/power) + current-mode summing TIA | 2  | P3b 2_P: ideal27 +1.05e-05, mcC σ 8.92e-04 (8/8 conv) |
| 209 | wave10 | eta_baryon_photon | Poof¹¹/(πγ) | - | GAMMA,PI,POOF | LOGR+SUM+CAL (log-domain product/power) | 2  | P3b 2_M: ideal27 -1.18e-03, mcC σ 6.92e-02 (8/8 conv) |
| 210 | lepton_ratios | m_tau/m_e_lepton | 9πφ¹⁰ | - | PHI,PI | LOGR+SUM+CAL (log-domain product/power) | 2a | P3b 2a_M: ideal27 +5.36e-05, mcC σ 3.77e-03 (8/8 conv) |
| 211 | lepton_ratios | m_tau/m_mu_lepton | (π³−π)/(π^(1/3)+γ³) | - | GAMMA,PI | LOGR+SUM+CAL (log-domain product/power) + current-mode summing TIA | 2a | P3b 2a_X: ideal27 +3.50e-05, mcC σ - (0/8 conv) |
| 212 | lepton_ratios | m_mu/m_e_lepton | (35φ⁻⁵+145)·e^(1/3) | - | E,PHI | LOGR+SUM+CAL (log-domain product/power) + current-mode summing TIA | 2a | P3b 2a_X: ideal27 +1.02e-05, mcC σ - (0/8 conv) |
| 213 | lepton_ratios | Chain_consistency_% | \|(τ/μ)·(μ/e)−(τ/e)\|/(τ/e)×100 | - | E,GAMMA,PHI,PI | LOGR+SUM+CAL (log-domain product/power) + current-mode summing TIA + precision full-wave | 2a | P3b 2a_X: COMPILED, ideal27 +2.88e-01 (cancellation below E24 resolution), MC 0/12 converge |
| 214 | dynamical_systems | Logistic_R_inf | (γ⁻³+π⁻³)/(γ⁻⁰·²⁵+π⁻¹) | - | GAMMA,PI | LOGR+SUM+CAL (log-domain product/power) + current-mode summing TIA | 2a | P3b 2a_X: ideal27 -3.69e-06, mcC σ - (0/8 conv) |
| 215 | dynamical_systems | Ising2D_Tc_J | (γ⁻³+φ⁻²/³)/(π⁻²/³+π²/³) | - | GAMMA,PHI,PI | LOGR+SUM+CAL (log-domain product/power) + current-mode summing TIA | 2a | P3b 2a_X: ideal27 -7.75e-06, mcC σ - (0/8 conv) |
| 216 | dynamical_systems | Ising3D_beta_dyn | (e⁻¹/³+√π)/(e²+φ⁻³) | - | E,PHI,PI | LOGR+SUM+CAL (log-domain product/power) + current-mode summing TIA | 2a | P3b 2a_X: ideal27 +8.97e-06, mcC σ - (0/8 conv) |
| 217 | dynamical_systems | Ising3D_nu_dyn | (1/γ−π⁻³)/(e²/³+π⁻¹/⁴) | - | E,GAMMA,PI | LOGR+SUM+CAL (log-domain product/power) + current-mode summing TIA | 2a | P3b 2a_X: ideal27 +5.73e-06, mcC σ - (0/8 conv) |
| 218 | dynamical_systems | Henon_Lyapunov | φ·(γ√2/e)/ln(π)·(1−π⁻⁴) | - | E,GAMMA,PHI,PI | LOGR+SUM+CAL (log-domain product/power) + current-mode summing TIA + LOGR (log of a computed quantity) | 2a | P3b 2a_X: ideal27 +2.95e-05, mcC σ - (0/8 conv) |
| 219 | neural_architecture | N_Layers | ⌊2π⌋ | - | PI | LOGR+SUM+CAL (log-domain product/power) + comparator ladder (quantiser) | 2a | P3b 2a_Q: ideal27 -3.19e-06, mcC σ 2.26e-04 (8/8 conv) |
| 220 | neural_architecture | N_Lobes | ⌊π⌋+⌊φ⌋ | - | PHI,PI | current-mode summing TIA + comparator ladder (quantiser) | 2a | P3b 2a_Q: ideal27 -5.98e-06, mcC σ 3.12e-04 (8/8 conv) |
| 221 | neural_architecture | N_Columns | ⌊e³⌋ | - | E | LOGR+SUM+CAL (log-domain product/power) + comparator ladder (quantiser) | 2a | P3b 2a_Q: ideal27 -2.26e-06, mcC σ 1.93e-04 (8/8 conv) |
| 222 | neural_architecture | WM_Capacity | ⌊2π⌋+1 | - | PI | LOGR+SUM+CAL (log-domain product/power) + current-mode summing TIA + comparator ladder (quantiser) | 2a | P3b 2a_Q: ideal27 -5.42e-06, mcC σ 2.73e-04 (8/8 conv) |
| 223 | neural_architecture | N_Attention_Heads | ⌊eπ⌋ | - | E,PI | LOGR+SUM+CAL (log-domain product/power) + comparator ladder (quantiser) | 2a | P3b 2a_Q: ideal27 -3.73e-06, mcC σ 1.95e-04 (8/8 conv) |
| 224 | neural_architecture | N_Drives | ⌊π²⌋ | - | PI | LOGR+SUM+CAL (log-domain product/power) + comparator ladder (quantiser) | 2a | P3b 2a_Q: ideal27 -6.90e-07, mcC σ 2.01e-04 (8/8 conv) |
| 225 | neural_architecture | Binding_Window | ⌊π²⌋ | - | PI | LOGR+SUM+CAL (log-domain product/power) + comparator ladder (quantiser) | 2a | P3b 2a_Q: ideal27 -6.90e-07, mcC σ 2.01e-04 (8/8 conv) |
| 226 | neural_architecture | Seq_Predict_Period | ⌊π⌋+1 | - | PI | current-mode summing TIA + comparator ladder (quantiser) | 2a | P3b 2a_Q: ideal27 -4.68e-06, mcC σ 2.25e-04 (8/8 conv) |
| 227 | neural_architecture | Lateral_Inhibition | 1/φ | - | PHI | LOGR+SUM+CAL (log-domain product/power) | 2a | P3b 2a_M: ideal27 +3.73e-06, mcC σ 6.28e-04 (8/8 conv) |
| 228 | neural_architecture | Spike_Threshold | φ/(1+φ) | - | PHI | LOGR+SUM+CAL (log-domain product/power) + current-mode summing TIA | 2a | P3b 2a_P: ideal27 +3.76e-07, mcC σ 5.53e-04 (8/8 conv) |
| 229 | neural_architecture | Prediction_Gain | e/π | - | E,PI | LOGR+SUM+CAL (log-domain product/power) | 2a | P3b 2a_M: ideal27 +8.36e-07, mcC σ 6.29e-04 (8/8 conv) |
| 230 | neural_architecture | TD_BU_Ratio | φ | - | PHI | buffer of a generated constant | 2a | P3b 2a_M: ideal27 +9.93e-06, mcC σ 7.37e-04 (8/8 conv) |
| 231 | neural_architecture | Recurrent_Decay | 1/e | - | E | LOGR+SUM+CAL (log-domain product/power) | 2a | P3b 2a_M: ideal27 +8.12e-06, mcC σ 7.98e-04 (8/8 conv) |
| 232 | neural_architecture | Hebbian_LR | γ/π | - | GAMMA,PI | LOGR+SUM+CAL (log-domain product/power) | 2a | P3b 2a_M: ideal27 +8.42e-06, mcC σ 8.81e-04 (8/8 conv) |
| 233 | neural_architecture | Consciousness_Threshold | φ⁵ | - | PHI | LOGR+SUM+CAL (log-domain product/power) | 2a | P3b 2a_M: ideal27 +2.24e-05, mcC σ 2.07e-03 (8/8 conv) |
| 234 | neural_architecture | Consciousness_Gate | φ/(1+φ) | - | PHI | LOGR+SUM+CAL (log-domain product/power) + current-mode summing TIA | 2a | P3b 2a_P: ideal27 +3.76e-07, mcC σ 5.53e-04 (8/8 conv) |
| 235 | neural_architecture | Sync_Decay | 1/φ² | - | PHI | LOGR+SUM+CAL (log-domain product/power) | 2a | P3b 2a_M: ideal27 +9.58e-07, mcC σ 1.10e-03 (8/8 conv) |
| 236 | neural_architecture | Cross_Modal_Gain | e/π | - | E,PI | LOGR+SUM+CAL (log-domain product/power) | 2a | P3b 2a_M: ideal27 +8.36e-07, mcC σ 6.29e-04 (8/8 conv) |
| 237 | neural_architecture | Circulant_Spectral_R | 1/φ | - | PHI | LOGR+SUM+CAL (log-domain product/power) | 2a | P3b 2a_M: ideal27 +3.73e-06, mcC σ 6.28e-04 (8/8 conv) |
| 238 | neural_architecture | EEG_1_over_f | φ | - | PHI | buffer of a generated constant | 2a | P3b 2a_M: ideal27 +9.93e-06, mcC σ 7.37e-04 (8/8 conv) |
| 239 | neural_architecture | Layer_ | (1/φ^ | - | - | resistor ratio / literal (no computation) | 0  | planned(literal) |
| 240 | consciousness_model | Metatron_Spheres | 13 | - | - | resistor ratio / literal (no computation) | 0  | planned(literal) |
| 241 | consciousness_model | Metatron_Pathways | 3³=27 | - | - | resistor ratio / literal (no computation) | 0  | planned(literal) |
| 242 | consciousness_model | Consciousness_Gate | φ/(1+φ) | - | PHI | LOGR+SUM+CAL (log-domain product/power) + current-mode summing TIA | 2a | P3b 2a_P: ideal27 +3.76e-07, mcC σ 5.53e-04 (8/8 conv) |
| 243 | consciousness_model | Resonance_Persistence | e/π | - | E,PI | LOGR+SUM+CAL (log-domain product/power) | 2a | P3b 2a_M: ideal27 +8.36e-07, mcC σ 6.29e-04 (8/8 conv) |
| 244 | consciousness_model | Resonance_Rate | γ/e | - | E,GAMMA | LOGR+SUM+CAL (log-domain product/power) | 2a | P3b 2_M: ideal27 +1.26e-05, mcC σ 7.11e-04 (8/8 conv) |
| 245 | consciousness_model | Resonance_Eq_Factor | γπ/(e(π−e)) | - | E,GAMMA,PI | LOGR+SUM+CAL (log-domain product/power) + current-mode summing TIA | 2a | P3b 2a_P: ideal27 -5.26e-05, mcC σ 8.85e-03 (8/8 conv) |
| 246 | consciousness_model | Ignition_Coherence | Gate/Eq | - | E,GAMMA,PHI,PI | LOGR+SUM+CAL (log-domain product/power) + current-mode summing TIA | 2a | P3b 2a_P: ideal27 +6.34e-05, mcC σ 9.08e-03 (8/8 conv) |
| 247 | consciousness_model | W_Integration | φ | - | PHI | buffer of a generated constant | 2a | P3b 2a_M: ideal27 +9.93e-06, mcC σ 7.37e-04 (8/8 conv) |
| 248 | consciousness_model | W_Complexity | φ/(1+φ) | - | PHI | LOGR+SUM+CAL (log-domain product/power) + current-mode summing TIA | 2a | P3b 2a_P: ideal27 +3.76e-07, mcC σ 5.53e-04 (8/8 conv) |
| 249 | consciousness_model | W_Binding | e/π | - | E,PI | LOGR+SUM+CAL (log-domain product/power) | 2a | P3b 2a_M: ideal27 +8.36e-07, mcC σ 6.29e-04 (8/8 conv) |
| 250 | consciousness_model | W_Phase_Sync | 1/φ | - | PHI | LOGR+SUM+CAL (log-domain product/power) | 2a | P3b 2a_M: ideal27 +3.73e-06, mcC σ 6.28e-04 (8/8 conv) |
| 251 | consciousness_model | Hub_coupling | φ/(1+φ) | - | PHI | LOGR+SUM+CAL (log-domain product/power) + current-mode summing TIA | 2a | P3b 2a_P: ideal27 +3.76e-07, mcC σ 5.53e-04 (8/8 conv) |
| 252 | consciousness_model | Inner_coupling | 1/φ² | - | PHI | LOGR+SUM+CAL (log-domain product/power) | 2a | P3b 2a_M: ideal27 +9.58e-07, mcC σ 1.10e-03 (8/8 conv) |
| 253 | consciousness_model | Radial_coupling | e/π | - | E,PI | LOGR+SUM+CAL (log-domain product/power) | 2a | P3b 2a_M: ideal27 +8.36e-07, mcC σ 6.29e-04 (8/8 conv) |
| 254 | consciousness_model | Outer_coupling | 1/φ | - | PHI | LOGR+SUM+CAL (log-domain product/power) | 2a | P3b 2a_M: ideal27 +3.73e-06, mcC σ 6.28e-04 (8/8 conv) |
| 255 | consciousness_model | Cross_coupling | γ | - | GAMMA | buffer of a generated constant | 2a | P3b 2a_M: ideal27 +1.22e-05, mcC σ 1.23e-03 (8/8 conv) |
| 256 | homeostasis | Replay_Passes | ⌊2π⌋ | - | PI | LOGR+SUM+CAL (log-domain product/power) + comparator ladder (quantiser) | 2a | P3b 2a_Q: ideal27 -3.19e-06, mcC σ 2.26e-04 (8/8 conv) |
| 257 | homeostasis | Homeostatic_Tau | 1/\|Chaos\| | - | CHAOS | LOGR+SUM+CAL (log-domain product/power) + precision full-wave | 2  | P3b 2_M: ideal27 +5.05e-05, mcC σ 2.12e-03 (8/8 conv) |
| 258 | homeostasis | Novelty_Threshold | 1/φ | - | PHI | LOGR+SUM+CAL (log-domain product/power) | 2a | P3b 2a_M: ideal27 +3.73e-06, mcC σ 6.28e-04 (8/8 conv) |
| 259 | homeostasis | Consolidation_Rate | γ/π | - | GAMMA,PI | LOGR+SUM+CAL (log-domain product/power) | 2a | P3b 2a_M: ideal27 +8.42e-06, mcC σ 8.81e-04 (8/8 conv) |
| 260 | homeostasis | Attention_Inhibition | B_in | - | B_IN | buffer of a generated constant | 2  | P3b 2_M: ideal27 +2.08e-05, mcC σ 1.67e-03 (8/8 conv) |
| 261 | homeostasis | Cross_Lobe_Coupling | γ/π | - | GAMMA,PI | LOGR+SUM+CAL (log-domain product/power) | 2a | P3b 2a_M: ideal27 +8.42e-06, mcC σ 8.81e-04 (8/8 conv) |
| 262 | soliton_stdp | STDP_Tau_Plus_ms | 20.0 | - | - | resistor ratio / literal (no computation) | 0  | planned(literal) |
| 263 | soliton_stdp | STDP_Tau_Minus_ms | 20.0 | - | - | resistor ratio / literal (no computation) | 0  | planned(literal) |
| 264 | soliton_stdp | Soliton_Width | 1/(1+Poof) | - | POOF | LOGR+SUM+CAL (log-domain product/power) + current-mode summing TIA | 2  | P3b 2_P: ideal27 +1.58e-05, mcC σ 1.34e-03 (8/8 conv) |
| 265 | soliton_stdp | STDP_Learning_Rate | K·ψ_con | - | K,PSI_CON | LOGR+SUM+CAL (log-domain product/power) | 2  | P3b 2_M: ideal27 +2.12e-05, mcC σ 1.50e-03 (8/8 conv) |
| 266 | soliton_stdp | STDP_Asymmetry | A_bleed | - | A_BLEED | buffer of a generated constant | 2  | P3b 2_M: ideal27 +2.28e-05, mcC σ 1.79e-03 (8/8 conv) |
| 267 | soliton_stdp | Promotion_Threshold | 1 − η_eff | - | ETA_EFF | current-mode summing TIA | 2  | P3b 2_P: ideal27 +6.21e-06, mcC σ 1.49e-03 (8/8 conv) |
| 268 | soliton_stdp | Hebbian_Consolidation | γ/π | - | GAMMA,PI | LOGR+SUM+CAL (log-domain product/power) | 2a | P3b 2a_M: ideal27 +8.42e-06, mcC σ 8.81e-04 (8/8 conv) |
| 269 | soliton_stdp | Oja_Learning_Rate | γ/π | - | GAMMA,PI | LOGR+SUM+CAL (log-domain product/power) | 2a | P3b 2a_M: ideal27 +8.42e-06, mcC σ 8.81e-04 (8/8 conv) |
| 270 | soliton_stdp | Attention_Error_Gain | e/π | - | E,PI | LOGR+SUM+CAL (log-domain product/power) | 2a | P3b 2a_M: ideal27 +8.36e-07, mcC σ 6.29e-04 (8/8 conv) |
| 271 | cross_species | S(Human) | S_neuro D= | - | - | resistor ratio / literal (no computation) | 0  | planned(literal) |
| 272 | cross_species | Density(Human) | N/(\|S_neuro\|·V) | - | - | resistor ratio / literal (no computation) | 0  | planned(literal) |
| 273 | cross_species | S(Octopus) | S_neuro D= | - | - | resistor ratio / literal (no computation) | 0  | planned(literal) |
| 274 | cross_species | Density(Octopus) | N/(\|S_neuro\|·V) | - | - | resistor ratio / literal (no computation) | 0  | planned(literal) |
| 275 | cross_species | S(Corvid) | S_neuro D= | - | - | resistor ratio / literal (no computation) | 0  | planned(literal) |
| 276 | cross_species | Density(Corvid) | N/(\|S_neuro\|·V) | - | - | resistor ratio / literal (no computation) | 0  | planned(literal) |
| 277 | cross_species | S(Honeybee) | S_neuro D= | - | - | resistor ratio / literal (no computation) | 0  | planned(literal) |
| 278 | cross_species | Density(Honeybee) | N/(\|S_neuro\|·V) | - | - | resistor ratio / literal (no computation) | 0  | planned(literal) |
| 279 | cross_species | S(C_elegans) | S_neuro D= | - | - | resistor ratio / literal (no computation) | 0  | planned(literal) |
| 280 | cross_species | Density(C_elegans) | N/(\|S_neuro\|·V) | - | - | resistor ratio / literal (no computation) | 0  | planned(literal) |
| 281 | trinary | Max_Trits | 3³ | - | - | resistor ratio / literal (no computation) | 0  | planned(literal) |
| 282 | trinary | Collapse_Threshold | C_eff·P_var | - | C_EFF,P_VAR | LOGR+SUM+CAL (log-domain product/power) | 2  | P3b 2_M: ideal27 +2.90e-06, mcC σ 6.05e-03 (8/8 conv) |
| 283 | predictions | CMB_tau | (1/√φ−1/φ)/(π^(1/3)+√e) | - | E,PHI,PI | LOGR+SUM+CAL (log-domain product/power) + current-mode summing TIA | 2a | P3b 2a_X: ideal27 +8.24e-06, mcC σ - (0/8 conv) |
| 284 | predictions | Dark_energy_wa | −γeφ/π | - | E,GAMMA,PHI,PI | LOGR+SUM+CAL (log-domain product/power) | 2a | P3b 2a_M: ideal27 +1.14e-05, mcC σ 6.46e-04 (8/8 conv) |
| 285 | predictions | Tau_g-2 | α√φ·π/e | - | ALPHA,E,PHI,PI | LOGR+SUM+CAL (log-domain product/power) | 2  | P3b 2_M: ideal27 -2.03e-05, mcC σ 6.95e-03 (8/8 conv) |
| 286 | predictions | Cross_Opt_QO | S_opt/S_qo | - | - | resistor ratio / literal (no computation) | 0  | planned(literal) |
| 287 | predictions | Cross_MatSci_CM | S_ms/S_cm | - | - | resistor ratio / literal (no computation) | 0  | planned(literal) |
| 288 | predictions | Cross_Astro_PS | S_ast/S_ps | - | - | resistor ratio / literal (no computation) | 0  | planned(literal) |
| 289 | chemistry_ionization | IE_H | γ⁻⁵ − G⁻⁸ | - | GAMMA,G_CAT | LOGR+SUM+CAL (log-domain product/power) + current-mode summing TIA | 2a | P3b 2a_P: ideal27 -1.97e-05, mcC σ 3.10e-03 (8/8 conv) |
| 290 | chemistry_ionization | IE_He | e²/P_new | - | E,P_NEW | LOGR+SUM+CAL (log-domain product/power) | 2  | P3b 2_M: ideal27 +1.87e-06, mcC σ 9.46e-04 (8/8 conv) |
| 291 | chemistry_ionization | IE_Li | γ⁻³ + γ³ | - | GAMMA | LOGR+SUM+CAL (log-domain product/power) + current-mode summing TIA | 2a | P3b 2a_P: ideal27 -9.23e-06, mcC σ 1.39e-03 (8/8 conv) |
| 292 | chemistry_ionization | IE_Be | π² − sin(γ) | - | GAMMA,PI | LOGR+SUM+CAL (log-domain product/power) + current-mode summing TIA + trig series (powers block) | 2a | P3b 2a_T: ideal27 +1.87e-05, mcC σ 1.60e-03 (8/8 conv) |
| 293 | chemistry_ionization | IE_B | γ⁻⁴ − G⁴ | - | GAMMA,G_CAT | LOGR+SUM+CAL (log-domain product/power) + current-mode summing TIA | 2a | P3b 2a_P: ideal27 -1.83e-05, mcC σ 2.42e-03 (8/8 conv) |
| 294 | chemistry_ionization | IE_C | e⁷/π⁴ | - | E,PI | LOGR+SUM+CAL (log-domain product/power) | 2a | P3b 2a_M: ideal27 -1.59e-05, mcC σ 2.64e-03 (8/8 conv) |
| 295 | chemistry_ionization | IE_N | γ⁻⁵ − G⁻¹ | - | GAMMA,G_CAT | LOGR+SUM+CAL (log-domain product/power) + current-mode summing TIA | 2a | P3b 2a_P: ideal27 -2.08e-05, mcC σ 2.89e-03 (8/8 conv) |
| 296 | chemistry_ionization | IE_O | sin(1)/C_cosm | - | C_COSM | LOGR+SUM+CAL (log-domain product/power) + trig series (powers block) | 2  | P3b 2_M: ideal27 +2.78e-05, mcC σ 2.14e-03 (8/8 conv) |
| 297 | chemistry_ionization | IE_F | φ⁷/A_in | - | A_IN,PHI | LOGR+SUM+CAL (log-domain product/power) | 2  | P3b 2_M: ideal27 +1.09e-05, mcC σ 4.14e-03 (8/8 conv) |
| 298 | chemistry_ionization | IE_Ne | π·φ⁴ | - | PHI,PI | LOGR+SUM+CAL (log-domain product/power) | 2a | P3b 2a_M: ideal27 +3.15e-05, mcC σ 2.86e-03 (8/8 conv) |
| 299 | chemistry_ionization | IE_Na | γ⁻³ − C_cosm | - | C_COSM,GAMMA | LOGR+SUM+CAL (log-domain product/power) + current-mode summing TIA | 2  | P3b 2_P: ideal27 -1.12e-05, mcC σ 1.95e-03 (8/8 conv) |
| 300 | chemistry_ionization | IE_Mg | φ⁴ + B_in | - | B_IN,PHI | LOGR+SUM+CAL (log-domain product/power) + current-mode summing TIA | 2  | P3b 2_P: ideal27 +2.45e-05, mcC σ 2.40e-03 (8/8 conv) |
| 301 | chemistry_ionization | IE_Al | φ⁴/ln(π) | - | PHI,PI | LOGR+SUM+CAL (log-domain product/power) + LOGR (log of a computed quantity) | 2a | P3b 2a_X: ideal27 +3.34e-05, mcC σ - (0/8 conv) |
| 302 | chemistry_ionization | IE_Si | e² + √γ | - | E,GAMMA | LOGR+SUM+CAL (log-domain product/power) + current-mode summing TIA | 2a | P3b 2a_P: ideal27 +6.57e-06, mcC σ 6.57e-04 (8/8 conv) |
| 303 | chemistry_ionization | IE_P | π² + φ⁻¹ | - | PHI,PI | LOGR+SUM+CAL (log-domain product/power) + current-mode summing TIA | 2a | P3b 2a_P: ideal27 +1.70e-05, mcC σ 1.79e-03 (8/8 conv) |
| 304 | chemistry_ionization | IE_S | φ⁶/√3 | - | PHI | LOGR+SUM+CAL (log-domain product/power) | 2a | P3b 2a_M: ideal27 +2.72e-05, mcC σ 2.82e-03 (8/8 conv) |
| 305 | chemistry_ionization | IE_Cl | φ⁷/√5 | - | PHI | LOGR+SUM+CAL (log-domain product/power) | 2a | P3b 2a_M: ideal27 +4.58e-05, mcC σ 3.16e-03 (8/8 conv) |
| 306 | chemistry_ionization | IE_Ar | γ⁻⁵ + Poof | - | GAMMA,POOF | LOGR+SUM+CAL (log-domain product/power) + current-mode summing TIA | 2  | P3b 2_P: ideal27 -1.96e-05, mcC σ 3.64e-03 (8/8 conv) |
| 307 | chemistry_ionization | IE_K | φ³ + π⁻² | - | PHI,PI | LOGR+SUM+CAL (log-domain product/power) + current-mode summing TIA | 2a | P3b 2a_P: ideal27 +1.81e-05, mcC σ 3.50e-03 (8/8 conv) |
| 308 | chemistry_ionization | IE_Ca | φ³/ln(2) | - | PHI | LOGR+SUM+CAL (log-domain product/power) + LOGR (log of a computed quantity) | 2a | P3b 2a_X: ideal27 +4.85e-05, mcC σ - (0/8 conv) |
| 309 | chemistry_electronegativity | EN_H | γ⁻¹ + η_eff | - | ETA_EFF,GAMMA | LOGR+SUM+CAL (log-domain product/power) + current-mode summing TIA | 2  | P3b 2_P: ideal27 -2.04e-06, mcC σ 7.23e-04 (8/8 conv) |
| 310 | chemistry_electronegativity | EN_Li | γ⁵ + G | - | GAMMA,G_CAT | LOGR+SUM+CAL (log-domain product/power) + current-mode summing TIA | 2a | P3b 2a_P: ideal27 +1.52e-05, mcC σ 8.56e-04 (8/8 conv) |
| 311 | chemistry_electronegativity | EN_Na | G⁵/ln(2) | - | G_CAT | LOGR+SUM+CAL (log-domain product/power) + LOGR (log of a computed quantity) | 2a | P3b 2a_X: ideal27 +6.44e-05, mcC σ - (0/8 conv) |
| 312 | chemistry_electronegativity | EN_C | G⁻⁵ + sin(φ) | - | G_CAT,PHI | LOGR+SUM+CAL (log-domain product/power) + current-mode summing TIA + trig series (powers block) | 2a | P3b 2a_T: ideal27 -1.28e-05, mcC σ 2.70e-03 (8/8 conv) |
| 313 | chemistry_electronegativity | EN_N | π − π⁻² | - | PI | LOGR+SUM+CAL (log-domain product/power) + current-mode summing TIA | 2a | P3b 2a_P: ideal27 +1.31e-05, mcC σ 1.03e-03 (8/8 conv) |
| 314 | chemistry_electronegativity | EN_O | √π + A_in | - | A_IN,PI | LOGR+SUM+CAL (log-domain product/power) + current-mode summing TIA | 2  | P3b 2_P: ideal27 +2.17e-05, mcC σ 1.34e-03 (8/8 conv) |
| 315 | chemistry_electronegativity | EN_F | ln(π)/C | - | C_FACTOR,PI | LOGR+SUM+CAL (log-domain product/power) + LOGR (log of a computed quantity) | 2  | P3b 2_X: ideal27 -1.04e-05, mcC σ 1.33e-03 (8/8 conv) |
| 316 | chemistry_electronegativity | EN_S | φ² − γ⁶ | - | GAMMA,PHI | LOGR+SUM+CAL (log-domain product/power) + current-mode summing TIA | 2a | P3b 2a_P: ideal27 +1.18e-05, mcC σ 2.56e-03 (8/8 conv) |
| 317 | chemistry_electronegativity | EN_Cl | π + e⁻⁴ | - | PI | current-mode summing TIA + CAL (antilog of a computed exponent) | 2a | P3b 2a_X: ideal27 +1.32e-05, mcC σ - (0/8 conv) |
| 318 | chemistry_electronegativity | EN_Br | γ⁻³ − √5 | - | GAMMA | LOGR+SUM+CAL (log-domain product/power) + current-mode summing TIA | 2a | P3b 2a_P: ideal27 -1.82e-05, mcC σ 3.15e-03 (8/8 conv) |
| 319 | chemistry_bond_lengths | BL_H−H | sin(1) − π⁻² | - | PI | LOGR+SUM+CAL (log-domain product/power) + current-mode summing TIA + trig series (powers block) | 2a | P3b 2a_P: ideal27 -7.59e-06, mcC σ 4.30e-04 (8/8 conv) |
| 320 | chemistry_bond_lengths | BL_C−H | A_in − γ | - | A_IN,GAMMA | current-mode summing TIA | 2  | P3b 2_P: ideal27 +3.63e-05, mcC σ 3.31e-03 (8/8 conv) |
| 321 | chemistry_bond_lengths | BL_C−C | γ⁻¹ − γ³ | - | GAMMA | LOGR+SUM+CAL (log-domain product/power) + current-mode summing TIA | 2a | P3b 2a_P: ideal27 -2.70e-06, mcC σ 6.24e-04 (8/8 conv) |
| 322 | chemistry_bond_lengths | BL_C=C | φ⁻² + P_var | - | PHI,P_VAR | LOGR+SUM+CAL (log-domain product/power) + current-mode summing TIA | 2  | P3b 2_P: ideal27 -1.65e-05, mcC σ 3.75e-03 (8/8 conv) |
| 323 | chemistry_bond_lengths | BL_C≡C | π/φ² | - | PHI,PI | LOGR+SUM+CAL (log-domain product/power) | 2a | P3b 2a_M: ideal27 +1.13e-05, mcC σ 9.79e-04 (8/8 conv) |
| 324 | chemistry_bond_lengths | BL_O−H | C_eff + e⁻⁶ | - | C_EFF | current-mode summing TIA + CAL (antilog of a computed exponent) | 2  | P3b 2_X: ideal27 +1.57e-05, mcC σ 2.08e-03 (8/8 conv) |
| 325 | chemistry_bond_lengths | BL_N−H | A_bleed − γ⁶ | - | A_BLEED,GAMMA | LOGR+SUM+CAL (log-domain product/power) + current-mode summing TIA | 2  | P3b 2_P: ideal27 +2.11e-05, mcC σ 1.26e-03 (8/8 conv) |
| 326 | chemistry_bond_lengths | BL_C−N | √2 + φ⁻⁶ | - | PHI | LOGR+SUM+CAL (log-domain product/power) + current-mode summing TIA | 2a | P3b 2a_P: ideal27 -2.04e-06, mcC σ 3.40e-04 (8/8 conv) |
| 327 | chemistry_bond_lengths | BL_C−O | ln(3) − Chaos | - | CHAOS | current-mode summing TIA + LOGR (log of a computed quantity) | 2  | P3b 2_X: ideal27 -1.56e-05, mcC σ 6.44e-04 (8/8 conv) |
| 328 | chemistry_bond_lengths | BL_C=O | G⁻⁸ − B_in | - | B_IN,G_CAT | LOGR+SUM+CAL (log-domain product/power) + current-mode summing TIA | 2  | P3b 2_P: ideal27 -6.30e-05, mcC σ 1.05e-02 (8/8 conv) |
| 329 | chemistry_bond_energies | BE_H−H | e⁸/φ⁴ | - | E,PHI | LOGR+SUM+CAL (log-domain product/power) | 2a | P3b 2a_M: ideal27 -4.11e-06, mcC σ 2.00e-03 (8/8 conv) |
| 330 | chemistry_bond_energies | BE_C−H | e⁶ + π² | - | E,PI | LOGR+SUM+CAL (log-domain product/power) + current-mode summing TIA | 2a | P3b 2a_P: ideal27 +9.18e-06, mcC σ 7.03e-04 (8/8 conv) |
| 331 | chemistry_bond_energies | BE_C−C | e⁶ − e⁴ | - | E | LOGR+SUM+CAL (log-domain product/power) + current-mode summing TIA | 2a | P3b 2a_P: ideal27 +8.10e-06, mcC σ 3.82e-04 (8/8 conv) |
| 332 | chemistry_bond_energies | BE_C=C | G⁸/α | - | ALPHA,G_CAT | LOGR+SUM+CAL (log-domain product/power) | 2  | P3b 2_M: ideal27 +1.23e-04, mcC σ 8.66e-03 (8/8 conv) |
| 333 | chemistry_bond_energies | BE_C≡C | e⁶/ln(φ) | - | E,PHI | LOGR+SUM+CAL (log-domain product/power) + LOGR (log of a computed quantity) | 2a | P3b 2a_X: ideal27 +3.38e-05, mcC σ - (0/8 conv) |
| 334 | chemistry_bond_energies | BE_O−H | π⁶·ln(φ) | - | PHI,PI | LOGR+SUM+CAL (log-domain product/power) + LOGR (log of a computed quantity) | 2a | P3b 2a_X: ideal27 -2.36e-05, mcC σ - (0/8 conv) |
| 335 | chemistry_bond_energies | BE_N−H | e⁶ − φ⁵ | - | E,PHI | LOGR+SUM+CAL (log-domain product/power) + current-mode summing TIA | 2a | P3b 2a_P: ideal27 +6.16e-06, mcC σ 7.43e-04 (8/8 conv) |
| 336 | chemistry_bond_energies | BE_N≡N | π⁶ − γ⁻⁵ | - | GAMMA,PI | LOGR+SUM+CAL (log-domain product/power) + current-mode summing TIA | 2a | P3b 2a_P: ideal27 +1.08e-05, mcC σ 4.43e-03 (8/8 conv) |
| 337 | chemistry_bond_energies | BE_O=O | π⁵·φ | - | PHI,PI | LOGR+SUM+CAL (log-domain product/power) | 2a | P3b 2a_M: ideal27 +2.84e-05, mcC σ 4.90e-03 (8/8 conv) |
| 338 | chemistry_bond_energies | BE_F−F | e⁵ + φ⁵ | - | E,PHI | LOGR+SUM+CAL (log-domain product/power) + current-mode summing TIA | 2a | P3b 2a_P: ideal27 +9.31e-06, mcC σ 9.11e-04 (8/8 conv) |
| 339 | chemistry_molecular | BP_H₂O | e⁶ − π³ | - | E,PI | LOGR+SUM+CAL (log-domain product/power) + current-mode summing TIA | 2a | P3b 2a_P: ideal27 +3.66e-06, mcC σ 4.76e-04 (8/8 conv) |
| 340 | chemistry_molecular | BP_NH₃ | e⁵·φ | - | E,PHI | LOGR+SUM+CAL (log-domain product/power) | 2a | P3b 2a_M: ideal27 +9.16e-06, mcC σ 1.30e-03 (8/8 conv) |
| 341 | chemistry_molecular | BP_CH₄ | π⁷·γ⁶ | - | GAMMA,PI | LOGR+SUM+CAL (log-domain product/power) | 2a | P3b 2a_M: ideal27 +9.60e-05, mcC σ 6.19e-03 (8/8 conv) |
| 342 | chemistry_molecular | BP_C₂H₅OH | π⁸·γ⁶ | - | GAMMA,PI | LOGR+SUM+CAL (log-domain product/power) | 2a | P3b 2a_M: ideal27 +9.64e-05, mcC σ 6.69e-03 (8/8 conv) |
| 343 | chemistry_molecular | BP_CO₂_sub | e⁶·ln(φ) | - | E,PHI | LOGR+SUM+CAL (log-domain product/power) + LOGR (log of a computed quantity) | 2a | P3b 2a_X: ideal27 -2.01e-05, mcC σ - (0/8 conv) |
| 344 | chemistry_molecular | dipole_H₂O | √π/P_var | - | PI,P_VAR | LOGR+SUM+CAL (log-domain product/power) | 2  | P3b 2_M: ideal27 +2.66e-05, mcC σ 5.07e-03 (8/8 conv) |
| 345 | chemistry_molecular | dipole_NH₃ | √2 + φ⁻⁶ | - | PHI | LOGR+SUM+CAL (log-domain product/power) + current-mode summing TIA | 2a | P3b 2a_P: ideal27 -2.04e-06, mcC σ 3.40e-04 (8/8 conv) |
| 346 | chemistry_molecular | dipole_HCl | G⁻⁷ − G³ | - | G_CAT | LOGR+SUM+CAL (log-domain product/power) + current-mode summing TIA | 2a | P3b 2a_P: ideal27 -6.90e-05, mcC σ 7.95e-03 (8/8 conv) |
| 347 | chemistry_molecular | pH_water | φ⁻⁴ + φ⁴ | - | PHI | LOGR+SUM+CAL (log-domain product/power) + current-mode summing TIA | 2a | P3b 2a_P: ideal27 +2.33e-05, mcC σ 4.28e-03 (8/8 conv) |
| 348 | chemistry_molecular | pKw_water | γ⁻⁵ − φ | - | GAMMA,PHI | LOGR+SUM+CAL (log-domain product/power) + current-mode summing TIA | 2a | P3b 2a_P: ideal27 -2.22e-05, mcC σ 2.83e-03 (8/8 conv) |
| 349 | chemistry_radii | R_H | ln(2)/√5 | - | - | resistor ratio / literal (no computation) | 0  | P3b 2a_X: ideal27 -1.90e-05, mcC σ - (0/8 conv) |
| 350 | chemistry_radii | R_C | π⁻⁴ + √γ | - | GAMMA,PI | LOGR+SUM+CAL (log-domain product/power) + current-mode summing TIA | 2a | P3b 2a_P: ideal27 +1.26e-05, mcC σ 6.57e-04 (8/8 conv) |
| 351 | chemistry_radii | R_N | G⁻⁷ − ln(3) | - | G_CAT | LOGR+SUM+CAL (log-domain product/power) + current-mode summing TIA + LOGR (log of a computed quantity) | 2a | P3b 2a_X: ideal27 -7.01e-05, mcC σ - (0/8 conv) |
| 352 | chemistry_radii | R_O | P_base/θ_S | - | P_BASE,THETA_S | LOGR+SUM+CAL (log-domain product/power) | 2  | P3b 2_M: ideal27 +3.11e-05, mcC σ 2.96e-03 (8/8 conv) |
| 353 | chemistry_radii | R_F | √γ − e⁻³ | - | GAMMA | LOGR+SUM+CAL (log-domain product/power) + current-mode summing TIA + CAL (antilog of a computed exponent) | 2a | P3b 2a_X: ideal27 +1.32e-05, mcC σ - (0/8 conv) |
| 354 | chemistry_radii | R_Na | γ⁻¹ − γ³ | - | GAMMA | LOGR+SUM+CAL (log-domain product/power) + current-mode summing TIA | 2a | P3b 2a_P: ideal27 -2.70e-06, mcC σ 6.24e-04 (8/8 conv) |
| 355 | chemistry_radii | R_Cl | C_eff + π⁻³ | - | C_EFF,PI | LOGR+SUM+CAL (log-domain product/power) + current-mode summing TIA | 2  | P3b 2_P: ideal27 +1.52e-05, mcC σ 1.42e-03 (8/8 conv) |
| 356 | chemistry_radii | R_Fe | φ⁻¹ + ψ_con | - | PHI,PSI_CON | LOGR+SUM+CAL (log-domain product/power) + current-mode summing TIA | 2  | P3b 2_P: ideal27 -1.47e-06, mcC σ 7.24e-04 (8/8 conv) |
