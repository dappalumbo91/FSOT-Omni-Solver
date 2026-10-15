# FSOT Omni-Solver: Phase 1 report

Repository: [dappalumbo91/FSOT-Omni-Solver](https://github.com/dappalumbo91/FSOT-Omni-Solver). Reproduce with `./reproduce.sh` (add `--mc` for the Monte Carlo).
FSOT authority (read-only): FSOT-2.1-Cpp @ `3466e8d4211d…` ([github](https://github.com/dappalumbo91/FSOT-2.1-Cpp/tree/3466e8d4211d786fa7a430de1ee75131fb3b2e97), fetched by `reproduce.sh`).
Circuit conventions follow [FSOT-Chua-Circuit](https://github.com/dappalumbo91/FSOT-Chua-Circuit): ngspice `.op` and `.tran`, TI TL082 vendor macromodel (sloj070, sha256-pinned), and Falstad CircuitJS for demos.

**The circuit is analog only.** It is built from op-amps, resistors, capacitors, BJTs (log/antilog and translinear), diodes, comparators and analog switches. No microcontroller, DAC/ADC or digital calibration appears anywhere. The analog switches only steer currents.

Every component value comes from `cpp/derive.cpp` (C++20, built on the engine headers). Values were sha256-locked in `locked/LOCK.md` at 2026-10-15 11:53 EDT, before any scored simulation. Python is used only for the cross-check and the plots.

## 1. Inventory and dependency graph

| layer | content | source |
|---|---|---|
| seeds | π, e, φ, γ, G (Catalan) | engine.hpp:110-115 |
| layer 1 | ALPHA = ln π/(e φ¹³), ψ_con = 1−e⁻¹, η_eff, β, γ_c, ω, θ_s, poof | engine.hpp:116-124 |
| layer 2 | C_eff, A_bleed, P_var, B_in, A_in, suction, chaos, P_base, P_new = (γ/e)√2, C_F, K, C_cosm | engine.hpp:125-138 |
| nest | 20 generations; D_eff(g) = round(5·5^(g/19)); 12 medium orifices (look=1); Atomic looks = e/π, HEP looks = 1−poof/π with hits=1 | engine.hpp:60-90, 151-155 |
| core law | S = K(T1+T2+T3), with T2 = 1 and T3 ∝ β ≈ 2.6e-17 | engine.hpp:201-221 |
| folds | S_cosm (D25, medium), S_quant (D5, obs), S_chem (D6, obs), domain_scalar | engine.hpp:146-148, 223-237 |
| closed forms | 346 rows: 196 rational/integer-power seed algebra, 127 with exp/log/sqrt/trig, 14 use folds directly, 9 use floor | closed_forms.gen.inc (`results/inventory.tsv`, file:line per row) |
| slice observables | α_s = 1/(eπ) (:7-8); Ω_b h² = \|S_cosm\|(1−S_chem) (:15-16); 1/α_em = e³φ⁴ − ψ_con (:70-71); m_H/m_W = S_quant(1+ψ_con) (:121-122) | closed_forms.gen.inc |

Correction to the task brief: there is no α₀ = 1/(e³φ⁴) in the engine. The engine has ALPHA = ln π/(eφ¹³) (engine.hpp:117) and 1/α_em = e³φ⁴ − ψ_con. The seed leaf additionally subtracts yy·C_F²/P_base (seed_leaves.hpp:46-48).

```
seeds (π,e,φ,γ,G) ─► layer-1 ─► layer-2 (K, P_new, C_F, P_var, …)
                                   │
nest: g ∈ 0..19 ─► D_eff = round(5·5^(g/19)) ─► fold (D, look, hits, observed)
                                   │
              core law  S = K·(T1 + 1 + T3)      [T3/S ≤ 1.3e-17 → 0]
              look=1, hits=0 (33 of 35 folds):  S = K + c·f(D)
                 f(D) = (D/25)^(-1/2)·(1 + P_new·ln(D/25)),  c_med = K·A/5 = −0.92249,  c_obs = c_med·O = +0.45882
                                   │
     S_cosm, S_quant, S_chem ─► closed forms (Ω_b h², m_H/m_W, H0, n_s, …)
     seed-only rows (α_s, 1/α_em, …) come straight from the constant generator
```

The engine has no iteration or ODE. It is a feed-forward algebraic network, so the circuit is a DC analog computer: each block settles to a fixed point.

**Independent check.** `tools/crosscheck_mpmath.py` (50-digit mpmath) re-implements the seeds, the layers and `compute_scalar`. It matches every locked C++ target to ≤ 2.2e-16 (`results/crosscheck_mpmath.txt`). It also confirms φ⁴ = 3φ+2 and that dropping T3 costs 9.8e-18.

## 2. Architecture (all analog)

| block | function | realisation | parts |
|---|---|---|---|
| Reference | VR = 2.5 V, −VR | trimmed shunt reference + unity inverter | LM4040-2.5 class, 1 op-amp, trimmer |
| g-string | generation input, 0.125 V/gen | precision divider; taps g = 0..19 | 200 kΩ E24 string |
| Fold ladder (quantizer) | D_eff = round(5·5^(g/19)) | 20 comparators at thresholds g_k = 19·log5((k+½)/5), k = 5..24. Each switches one 0.4 µA unit (R_step = VR/I_step − R_on) into node c2 via an SPDT. **Math adaptation:** rounding is done in the g-domain. Smallest margin is 0.067 gen = 8.35 mV (k=19). | 5 × LM339 + 7 × CD4053 per channel |
| LOGR1 | u = V_T ln(D/25) | log-ratio pair. Q1 is emitter-driven at I_25 = 10 µA; Q2 is base-driven with (+) at its collector, so the result is offset-insensitive at junction scale. | 2 op-amps, matched NPN pair |
| Antilog (translinear) | I = I0(1 + P_new·u/V_T)·e^(−u/2V_T) = I0·f(D) | Q3 current is I0 + u/R_x with R_x = V_T/(P_new·I0); base at u/2 (20 Ω/20 Ω divider); Q4 output | 1 op-amp, matched pair |
| TIA | vf = 1 V × f(D) | R_Tf = 100 kΩ | 1 op-amp |
| S summer | vS = 2 V × S = 2(K + c·f) | inverting sum. K comes from −VR via R_S0; c is set by the R_Sx/R_Sf ratio; observed channels add a ×(−1) inverter. | 1–2 op-amps |
| \|S_cosm\| | precision full-wave rectifier | 2 op-amps + 2 × 1N4148 | |
| 40(1−S_chem) | difference amp from −VR | 1 op-amp | |
| Ω_b h² | \|S_cosm\|·(1−S_chem) | log-ratio (LOGR5) → antilog whose Q current is (1−S_chem)·I0, i.e. a translinear multiplier | 4 op-amps, 4 NPN |
| m_H/m_W | S_quant(1+ψ_con) | non-inverting gain 1+ψ_con | 1 op-amp |
| α_s | e⁻¹/π | antilog with exponent −1 from a V_T/VR divider; 1/π as a resistor ratio; Early effect cancelled (V_CB = 0) | 2 op-amps, pair |
| 1/α_em | e³(3φ+2) − 1 + e⁻¹ | e³ antilog (−3V_T divider) and e⁻¹ antilog. 3φ+2 is set as a resistor ratio (exact identity φ⁴ = 3φ+2). Currents summed in a TIA. | 3 op-amps, 2 pairs |

**Voltage-to-FSOT scales** (each a locked design constant):
- S: 2 V/unit
- f: 1 V/unit
- g: 0.125 V/generation
- Ω_b h²: 100 V/unit
- α_s: 10 V/unit
- 1/α: 0.04 V/unit
- m_H/m_W: 2 V/unit

Currents are I0 = 10 µA and I_U = 0.2 µA (kept below high injection). V_T = kT/q = 25.865 mV at 27 °C (SI 2019).

**Vertical-slice BOM.** Three fold channels plus four observables: 31 op-amps, 22 NPN, 60 comparators, 60 SPDT, 352 resistor pieces, 21 capacitors and 2 diodes. Prices are approximate single-quantity distributor prices and were not re-verified today.

| build class (matches MC class) | key parts | ≈ cost |
|---|---|---|
| A, bench-cheap | 16 × TL082, 15 × LM339, 20 × CD4053, 22 × 2N3904 (unmatched), 1% E24 resistors, LM4040 | ≈ $40 |
| B, precision analog | zero-drift dual op-amps (MCP6V02/OPA2333 class, ≤ 25 µV) in the log/antilog/TIA/summer sockets; matched NPN pairs (SSM2212/MAT12 class, or hand-matched 2N3904 to 0.1 mV); 0.1% resistors in the ratio-setting positions; 0.1% reference | ≈ $150–200 |
| C, B + identity trims | B plus cermet trimmers nulled at analog identity points (for example D = 25 ⇒ u = 0 ⇒ trim vf to 1.0000 V; exponent dividers trimmed against a ratio bridge) instead of 0.02% resistors | ≈ B + $15 |

## 3. Vertical slice: results

Simulation setup:
- ngspice .op with the TL082 vendor macromodel and ±9 V rails.
- 2N3904 Fairchild model.
- E24 values realised as series strings (worst realisation error 4.75e-4).
- LM339-class behavioural comparators.
- Monte Carlo: 200 draws per class (`results/mc_summary.tsv`).

**MC tolerance classes:**

| class | resistors | Vos σ (op-amp and comparator) | BJT ΔVbe σ | VR | R_on |
|---|---|---|---|---|---|
| A | 1% (uniform) | 3 mV | 1 mV | ±0.5% | 75–175 Ω |
| B | 0.1% | 25 µV | 0.1 mV | ±0.1% | 75–175 Ω |
| C | 0.02% | 2 µV | 10 µV | ±0.05% | 75–175 Ω |

**How the MC was solved** (LOCK.md amendment 3). Full-TL082 .op failed to converge on ~60% of class-A draws, so each draw was solved with 1e6-gain op-amps carrying that draw's Vos. The TL082 effect was checked on the 33 cleanly converged draws, paired seed-for-seed: ≤ 3.5e-4 on every quantity, and it is already inside the nominal column. The fold MC uses the full TL082 model.

| quantity | C++ engine | ngspice nominal (E24+TL082) | nominal rel err | MC-A sd (cheap) | MC-B sd (precision) | MC-C sd (precision+matched) | MC-C bias | bar (rel) | sig figs held A/B/C | bar needs | meets bar? |
|---|---|---|---|---|---|---|---|---|---|---|---|
| S_quant | 0.9501974702 | 0.9497125 | -5.10e-04 | 5.27e-02 | 3.20e-03 | 5.67e-04 | -3.02e-04 | 9.0e-04 (via m_H/m_W) | 1/2/3 | 4 | yes (1 sd) |
| S_chem | 0.9552893401 | 0.9548605 | -4.49e-04 | 5.10e-02 | 3.55e-03 | 5.60e-04 | -2.71e-04 | 3.1e-04 (via Omega_b h2 (x21 gain)) | 1/2/3 | 4 | no |
| S_cosm | -0.5023773182 | -0.502275 | +2.04e-04 | 1.24e-01 | 9.97e-03 | 1.07e-03 | -1.42e-04 | 6.7e-03 (via Omega_b h2 (|S_cosm| enters x1)) | 0/2/2 | 3 | yes (1 sd) |
| f_D5 | 1.155336367 | 1.154482 | -7.39e-04 | 9.29e-02 | 5.51e-03 | 7.98e-04 | -3.63e-04 | 1.6e-03 (via S_quant) | 1/2/3 | 3 | yes (1 sd) |
| f_D6 | 1.166434175 | 1.165706 | -6.24e-04 | 8.71e-02 | 5.93e-03 | 7.81e-04 | -3.48e-04 | 5.6e-04 (via S_chem->Omega_b h2) | 1/2/3 | 4 | no |
| f_D25 | 1 | 0.9999318 | -6.82e-05 | 6.69e-02 | 5.42e-03 | 6.19e-04 | +1.21e-04 | 3.7e-03 (via S_cosm) | 1/2/3 | 3 | yes (1 sd) |
| m_H/m_W | 1.550836826 | 1.549886 | -6.13e-04 | 5.28e-02 | 3.21e-03 | 5.64e-04 | -3.78e-04 | 9.0e-04 (PDG24 ~1.5578(14)) | 1/2/3 | 4 | yes (1 sd) |
| Omega_b_h2 | 0.02246162142 | 0.02266238 | +8.94e-03 | 9.71e-01 | 7.60e-02 | 9.09e-03 | +5.82e-03 | 6.7e-03 (Planck18 0.02237(15)) | 0/1/1 | 3 | no |
| alpha_s(M_Z) | 0.117099663 | 0.1171457 | +3.93e-04 | 5.68e-02 | 5.67e-03 | 5.86e-04 | +2.87e-04 | 7.6e-03 (PDG 0.1180(9)) | 1/2/3 | 3 | yes (1 sd) |
| 1/alpha_em | 137.0361976 | 136.9456 | -6.61e-04 | 6.36e-02 | 6.50e-03 | 1.33e-03 | -5.77e-04 | 1.5e-10 (CODATA 137.035999177(21)) | 1/2/2 | 10 | no |

*Sig figs held* = floor(−log10 √(bias² + σ²)) per class. *Bar needs* = digits needed to resolve the measurement bar. The S and f rows have no direct measurement, so their bar is the requirement propagated from the observable they feed.

**Nominal (E24 + TL082) vs C++:** every row except Ω_b h² is within 2e-4 to 7.4e-4. Ω_b h² is +0.89% because 1−S_chem = 0.0447 amplifies S_chem errors 21×.

**Fold ladder (quantizer):**
- Nominal staircase is exact for g = 0..19 (`png/fold_staircase.png`).
- 33 look=1 domains on the physical g-string: S matches the engine with median 3.6e-4 and max 8.3e-4 (`results/fold33_nominal.tsv`).
- Fold MC with full TL082, 100 draws × 20 generations × 3 classes: **0 mis-rounds in 6000 decisions** (`results/fold_mc_summary.tsv`). D current accuracy σ is 0.12% (A) at D = 25.

**Falstad (independent simulator) core block, settled vS vs C++:**

| channel | Falstad vS | C++ vS | rel. diff |
|---|---|---|---|
| D6 Chemistry | 1.91027 V | 1.91058 V | −1.6e-4 |
| D25 Cosmology | −1.00462 V | −1.00475 V | −1.3e-4 |
| D5 QM | 1.90008 V | 1.90039 V | −1.7e-4 |

Source: `falstad/falstad_check.json`.

## 4. Error budget and precision limits

First-order budget for S_chem (`tools/error_budget.py`, `results/error_budget_S_chem.tsv`). The analytic RSS agrees with the MC sd:

| class | RSS (analytic) | MC sd | dominant terms |
|---|---|---|---|
| A | 4.8e-2 | 5.1e-2 | Vos at the antilog node (Vos/R_x, R_x = 8.6 kΩ): 3.4e-2; Q3/Q4 ΔVbe: 3.1e-2 |
| B | 3.3e-3 | 3.6e-3 | Q3/Q4 ΔVbe (0.1 mV ⇒ 0.39% in exp): 3.1e-3; summer ratios: 8e-4 |
| C | 4.3e-4 | 5.6e-4 | ΔVbe: 3e-4; VR: 2e-4; ratios: 2e-4 |

The LOGR1 pair mismatch nearly cancels at D = 6, because d ln f/du ∝ −½ + P_new/(1+P_new ln 0.24) = 0.026.

### Honest limits

1. **Bench-cheap build (A) does not reproduce FSOT.** S is good to about 5%, so it holds only about 1 significant figure. Ω_b h² has a ~100% spread and sometimes goes negative.
2. **Systematic floor of about 2e-4 to 6e-4** at nominal: Early effect in the LOGR, IKF in the e³ antilog, finite β, and TL082 finite gain (~1.7e-4). The MC-C bias column shows this floor too.
3. **Ω_b h² cannot meet the Planck bar** (6.7e-3) without class C plus floor removal. Its 21× amplification of S_chem turns 3e-4 into 6e-3.
4. **1/α_em is far beyond analog reach.** CODATA's bar is 1.5e-10. The best analog realisation is about 1e-4, which holds "137.0", i.e. 3–4 digits. FSOT itself differs from CODATA by 1.5e-6, which no analog circuit here can resolve.
5. **Temperature.** All exponents are V_T-referenced through fixed dividers, so the outputs move strongly with temperature:

   | quantity | 0 °C | 27 °C | 50 °C |
   |---|---|---|---|
   | 1/α | 184.3 | 136.9 | 110.5 |
   | S_quant | 0.9943 | 0.9497 | 0.9116 |
   | Ω_b h² | 0.0045 | 0.0227 | 0.0382 |

   (`results/nominal_temp.tsv`). Only S_cosm (u = 0 at D = 25) is immune.

### Analog-only fixes

| limit | fix | expected effect |
|---|---|---|
| ΔVbe mismatch | monolithic matched pairs (SSM2212/MAT12 class) or hand-matched pairs on one isothermal block; emitter-area-ratio trim with a base-divider trimmer at the identity point u = 0 | 3e-2 → 3e-3 → 3e-4 |
| Vos at junction-scale nodes | cheap zero-drift/chopper op-amps (MCP6V02/OPA2333 class, 2–10 µV) in the A/B/C/T/S sockets; raise R_x by running the P_new·u term at a scaled current (scale splitting) | 3.4e-2 → < 1e-4 |
| Ratio errors | 0.1% thin-film resistors in ratio positions (R_Sx/R_Sf/R_S0/R_Tf/R_x/R_I0); matched resistor networks for pairs; trimmers at identity points | 8e-3 → 2e-4 |
| VR | trimmed 0.1% reference; ratiometric design (I_D and I_25 from the same VR cancel) | 2e-4 |
| Temperature | PTAT exponent reference: take the V_T dividers from a ΔVBE PTAT cell, so the exponent ratio tracks kT/q. Mount all pairs on one isothermal block, with an analog proportional oven (thermistor bridge + op-amp + transistor heater). | removes the 0–50 °C swing above to first order |
| Ω_b h² amplification | form 1−S_chem directly as (1−K) − c·f in the current domain (range splitting) rather than subtracting near-equal voltages; run that channel at higher current scale | removes the summer/ratio share of the 21× gain |
| Early/IKF floor | cascode the log transistors (fixed V_CB), lower current density with larger-area devices, β-compensating base-current return | 4e-4 → ~1e-4 |

## 5. Falstad demos (core-law block, verified headless)

Each link is the full log-ratio, antilog, TIA and S summer, with locked E24 values. D is fixed by one resistor standing in for the ladder output. The 1 kΩ isolation resistors on the log/antilog op-amp outputs are a Falstad convergence aid only (LOCK.md).

- D6 Chemistry (specimen), expected vS = 1.91058 V: [link](https://www.falstad.com/circuit/circuitjs.html?ctz=CQAgjCCsCmC0Ac4AMA6ATEpaDs2DMS8aAbAJynZRJRVRxhgBQASiACyIfuJjEjXU21NCkj9xqSIwyUucxMTbsQANwBOLcGDTctO3hPbVYIsQP6jpSSgx1dbIRcvUA7RmvDY+efV5Ck+ajAkNl5iYhRwqOjsKxs-H08+AyV1OKT-ZL8HJQBjNHcQNCEQROLqAPE0SExMFCR08tKdJpTVDRkiksrW7WVcpgAXUrw+NGLS4kRxpSDDYOoAE2gAMwBDAFcAG0HGAHcRsYmCJRn+RgBzUpCi45umgX3J6cJnjIbO0Z4EqfBiQRAAyeXy61BBOB0H2sbwhMPgnBA0EYa2uSiEYN+6JAiGwohAsFxYgWtXMQRJqAaBxOpXgp2wOjwtPOnVhjNOJTZ-SYVJuoTG9PYBgaV1hfKKArYiUeHhBWMldkw4mJ9XS8qMgiccsRjFyqOaesqQTgSFIEnqSCYnxuiWpbSRVqUlVtfTyTF1IJtv0N4GNpoE5st0I9DN+bXylzeTtDfUewzYPQmkDQ0wmc3MCxAy3W212B3jRzRTjOUNkRYmarOSg2TyTL0Qtfe6QbYD8zf+-QKBwbTW7ApLUGT4p03fhyiRKIbWMgpAV1BxeIJeOJtXm5JVXcHnPzNNmEe3W8qWMeG84KUPQtV574bE1YYKHmns8ckGvirJdX7xBf6sco611Y8Yhjh0IDHUCIoalqFVOlA-VYLaNIYImSp4JdTMmEA0dEimRBvXgYgwDwaDoRwuDRzaRZA0oUiUPItCAJAbAk31JidG9DBiMoViWOYtpq06bjKm4nIQAAIww7EDESeADHYil0hk7wdEUv5lHEiMVMqFSHFjZ8C1-FNZnAVcllWTYdieL99Ng4t0hsxMZ1uNExO5AyileUiW0COzyL8Tz2yUSjLNHJpSNhT8QoFMK-0RNZGGGFSzlpQylRMzMzJzJ5EombAmVsq5ctOHK8pKY9sS4DBEGSxtOmqrzyp4ALM07BrQVa8KFIqgVquTBFoDilFuKxFThsYhdCWQckTKgylGOYzl6TBJl+0Wnc3K3dCniGgxVrFYVGIwQU+F2qVzg8EbFXhQQ30m+Tavga6KhdRUlH6nVsQe-UrsyJVfTNTAqI+sFlM+u04vuio+G+kSgpRChXzJEpRomxciXJUlbo-J54bW0g2CUTl+zxgmmVIPoNsWFqcbFYmjvOK5aZp-H2FO2bgiRxVgglN90i5p8GF5F7VBWQpacSWnvWXO7oTFtjmYQkXOglvhaZhgpdQWRBEk1n6jVgYJ-otXnMC1-QTdU1JFehHXKh1tXkXADBqBfIISCUF3sTGsRUcxm73xmp5tEVSB5d4etmf7BhfhDpQGFGKBmaUFwVgAS0Dp2oGyMPM+8q4o-rLPo8csrtCcD3tEqD3I7QSv20IwcPcttwPD5-VW8lklOOQAVtesfRr2F42BVtvu9GUZO05bwilK0eOO4Dzo45npeLZACfeenn6V5ElQVmbx3FW1jOO9ILHF4zo-OYH3e3HPxVbYzkSXAAZ0KUuCf0N3daKChIDALv35ty-ghNQt9rZfwfk4J+r8W4+FNuAOB39lwAMQdrRBCEADKvNEG23QWhF+Dsg6PUdpqS6Xt8QTSlhjKW64D5LVjtnQmG9fichXhtAhBwiF03znTfaPCxT8NZunUhrtDw83PmIoIiCtQqCwQADwyIoDEChJTKDAEoAAYhggA8gAFQADrP1yAAezUHALYaw9iGNElsYxuQADWAAuQxABhAAFtAAAtinZ+gw1AAE9DEAAoAAiAB9VYKwAC8xAAA0hjn4AAdoC5BTp46ALgACUjAFH1WID3X4+SxCx1SBgwxUTDFoEMQANUMXIwxGCADchitF6MMRki4KcXDQGCS4wxiStgbCSYM5+mTnHPzKc-Cpz9UCkEgLWUghjYAAD5DGyPKYY-+ZMkCQHgDUxgQA)
- D25 Cosmology (medium), expected vS = −1.00475 V: [link](https://www.falstad.com/circuit/circuitjs.html?ctz=CQAgjCCsCmC0Ac4AMA6ATEpaDs2DMS8aAbAJynZRJRVRxhgBQASiACyIfuJjEjXU21NCkj9xqSIwyUucxMTbsQANwBOLcGDTctO3hPbVYIsQP6jpSSgx1dbIRcvUA7RmvDY+efV5Ck+YUhMTBQQ8JC8Kxs-H08+AyV1aPj-BL8HJQBjNHcQNCEQOILqAPE0YJCwlJKinVrE1Q0ZfMKyhu1lLKYAFyK8PjQCouJEIaVqCHMwTBAAE2gAMwBDAFcAGx7GAHd+weGCJXH+RgBzIqQjg8vW8yQdkbHCR9T7loGeWNHwYkEQboeH1uLxwOje1hB2B0QLQ8E4IGgjGWFyUQmoQLRIEQ2FEIFgOLEMwihiJVXuu0ORXgRyhVImNVpeGpwKZqP+TApNzYBlB7AM93OvO5g1pbDiAjyGNmYrss0mZJSMqMgicmKUiKyKLqWrKkzgSFIEjCSCY7xucUpjURZqUZUtnWyTE1QIt3114H1hoExtNEJd0O+jRyZxedsDnQlfTY7WGkFh+WGkxJswWKw2W120f2qKcx3BslzwyVxyUqwecaeiArrxS1bAfjrvy6uV21dqbdp+ag8d5bbhykRyOrmMgpFl1GxuPxuNJIWT4WqrfjrPYduZAtX3mZWeVEnL8eFm75gUVZUPbFVQdyHlH48ckD4mPloS7xAfu9GnGlIDLHmIBx0f9bUCfJKhfFIgO1SDGmSFpILKaCHXmJg-37OJPzScR4GIMA8GqOC0MA-tGjmX1KAwhDiKQ38QGwONtTonR3QwfCIUYhj6MaMsWnYsp2MyEAACMUKxAw4ngAxmNQLsJO8HRZJ+ZRhJDBSygUhxI3vbNHH7PNwHneYljWTYHjfbTILzCDhmOW8EzZZTdgwjAFGIvxX1cvgMN4P5SNM3TCiczsIN02knP7dVlkYPoFOOakxkTfTphTIz0weGLhmwZlLPOTKriUXLgQlXY4vyZ4SvrE8WnKvxyqbJQ5hbLEuFqEreRk5raVa8KEUi5F2MxBSBtoqcCWQYkkoXclaPolcoXRdcUjmukdMQFd6o5aa7AMJbDw3HbtowdhxRODxBtmOFBDlMbpJSC7d1IB1v2gSLNTu8T4FKECwE9I1MDIrEPu1O6rUiqrAbUwGBN85EKEfWYZkKIbRunQkIjua7wN2WHltINglBXLtcfx5kHuhbd5ka7HDyJ48TnOGnqbxo6wROXYEcu+VRTlFIZi5yYEd3JJFjyGm4hp91ZxulpRaYpmYOF6WmbKGmodyTUiVW-RMEQCX6DuH0ee17UNcUoXDeeMoTdVvJtFmOJbc+8RIDYFiuwd42MEmR9VAViF3ctz29GUFwAGcbZIfH9AjzDhAoSAwFYmxo-t6OYLUNwWm0JwA6cATQ5tnxNfAQuY4xxPi+XfQS5ggBlHmS8t6ukPz5F3cxLPUXO4axBRjGrufBU2cDlcGG+Amed4VbmQYAZlqUfOh+lAxR84fkQxX2mN6VIrwGj9u0DPbnM4PuHJhLtVVDrgAPVJFHRb5FDwZQwCUAAxGuAHkABUAB0Q6yAA9moOA6xljbD-oJdYACsgAGsABcf8ADCACQ4AFsAFQNOAATz-gACgACIAH0liLAALwVAADR-1QdAOYABLVYqCACUjAb4VUcIyB+dFn5JBrn-Uhf80B-wAGp-yvn-GuABuP+79v5-2gC4U4dCXDQDwYgv+AAHdYqwQ5aJ0UwhBIc+EhwESHWAkgsB4FwH-WAAA+P+KhjGmNgAnTAbA6IiMYEAA)
- D5 Quantum_Mechanics (specimen), expected vS = 1.90039 V: [link](https://www.falstad.com/circuit/circuitjs.html?ctz=CQAgjCCsCmC0Ac4AMA6ATEpaDs2DMS8aAbAJynZRJRVRxhgBQASiACyIfuJjEjXU21NCkj9xqSIwyUucxMTbsQANwBOLcGDTctO3hPbVYIsQP6jpSSgx1dbIRcvUA7RmvDY+efV5Ck+am1ITEwUJCsbPx9PPgMldUjY-zi-ByUAYzR3EDQhEBi86gDxNBDQ8KSigp1q+NUNGVz8krrtZQymABcCvD40PILiRAGlIMMwTBAAE2gAMwBDAFcAGy7GAHde-sGCJVH+RgBzAqR93bPm8witvpHCIZ4-CKa75LfeQRBOzcerv5wOhe1gB2B0bzQ8E4IGgjAWpyUQmobyRIEQ2FEIFgGLEk1CU3MeIqNwRBXg+zBZLGVUpeHJ-zpiO+TFulzYBkB7AMEROnPZ-UpbBiAhyKKmQrsBOQYWBshiqLYTgVMMYGVJMT2KXEYDgSFIEnCSCYr0uGsu9VhJqUJU16WZqr+GuGWqCuv1AkNxpBbydPD4mWyJzeNudDhFPTYrUGkEhuUG40JU1mi1W6y2kZ2iKcB1l7GzgwlcaZS1+MfuiDLyVzlbAfhrxC+WVLseqlc51ZblLbUOUsPhldRkFIkuo6Mx2MxRNCE3xqBJlcZ7Bt9J5S+89IzRmuzc48RK-MOTU3B8ViPqTY8Q5Hjkg-qlRMqTWIt63jh7ypLHmIux03+tgVycoZSSP8akcQZ6kSJ9BhKUC7WmJgvx7GJhkQEpR2IMA8EfEFULAvD6gQkCe1gns7U-EBsBjMCqJ0dDcjnJJaJo6j6hLJpmJKZi7QAI0QtEDBieADHojAcMoYTvB0STwH9EA+OOAS+BKGSw0OHpn0zN8RnjcAZ2oZNljWX5NKLcDzmpaCLKgYczKUBStjwjAFDI55iKePgCIbZQiMcntqicylc0C39-PfGEFkYHoZIOckdLGPTEwM+YjLTJSzOwekc0UzLrNy-4RS2OKGMQYra0CJIyr8MrvKUaZsiKrhqmK9tKqaykWvC6BIvhZjURk-rKPHHFpXxfTiV+ZjFzBZEVyYjAqW0xa6pZSjqIPGauQqk5No2hbCxFDwBqmKFBHvCamlO19SHaZVuodK6hPgYoAJ1WA9QNTAvQk56wKui1Isu36VN++CmHhCg7yCM4ztHIaxAnXFZ2uUbgK2SHFtINglEXXMsZx+kbvBDcZga-w-APfGtsOE4qcp7H2GFQ4tkmfJUUmQUCSSDnrwYNlxVUOYcipmIqfoqdGKaEW6IZyChalhmSip+DsjVPFEBidWXXAehrk9bnMA1-RDdk5x5ZBLWSi1lW4XADBqFvIISCUR20XhrERollGJcqFn7agWXeArBncwYZ1IEDvoA6ZFw5gAS1+bQpkgNIg6gNyTjDitU-D2zCrtpxXe0EpXdDtAS+8rDY1dhI5jcDwebAxvxfxcTkFpY3KTlg3KSt6x9FukBY4ThusKkrQo5bi6LbHpvZ-qYfudnq358HlQ65yJPkX0f2W9IYCmi3pv-bltxD93uIT8HlwAGdN+d4+nFEihIDANvtCcTWH8gtQz4th+rbf2vnfBuPgjbgDAdrKc79IGa0gZBAAytzSBVt4HANtkfdmD9BojURqjKU0NW4kiPouLOi1Q5p1IbPRcShb6J39geMhB5VxMIMEwpmxDsFTGLlDQ8-99zcMgcqFQSCAAeyRFDImdIoPAygwBKAAGIIIAPIABUAA6N8MgAHs1BwBWAsDYmieIrG0RkAA1gALk0QARSWAsFwXQlgAFsAD6ABZaAGQAAWDi44ZBvpogAFAAEVcfMOYABeSAAAaTRN8AAOXi47OOgC4AAlIwcR5VHC0mkVRORCQEGaMiZotAmiABqmjRGaIQQAbk0UotRmi0lHDji4aAwSADCmiEkrCWIk-pN90nWJvsUm+pSb6oFICEMAFBNGwAAHyaJESUzRb995IDwKQSpjAgA)

## 6. Files

- `locked/`: design_values.tsv, fsot_targets.tsv, folds.tsv, ladder_thresholds.tsv, LOCK.md (sha256 hashes + 3 harness amendments)
- `cpp/`: design.hpp, derive.cpp, netgen.cpp, score.cpp
- `spice/`: run1.sh, mc_idealop.sh, fold_mc.sh, netlists/, out/, models/vendor/
- `results/`: results_table.md, mc_summary.tsv, mc_*_idealop.tsv, mc_mcA_paired_tl082_vs_idealop.tsv, fold_mc_summary.tsv, fold33_nominal.tsv, nominal_temp.tsv, error_budget_S_chem.tsv, crosscheck_mpmath.txt, inventory.tsv, SHA256SUMS
- `png/`: core_block_schematic.png, falstad_core_chem_D6.png, fold_staircase.png, mc_histograms.png
- `falstad/`: *.url, *.txt, gen_falstad_core.py, falstad_check.py/json
- `tools/`: inventory.py, crosscheck_mpmath.py, analyze_mc.py, table.py, error_budget.py, schematic_core.py

## 7. Proposed Phase 2 (still analog only)

1. **PTAT neper reference and isothermal block.** Re-derive the exponent dividers as PTAT ratios, add an analog oven, and re-lock. Rerun 0/27/50 °C with the goal |Δ| < 1e-3 across the range.
2. **Floor removal.** Cascoded log pairs, β compensation and a lower current density, re-locked and re-simulated. Target nominal ≤ 1e-4.
3. **Class-B and class-C netlists as first-class variants.** Real zero-drift op-amp macromodels in place of the TL082 in the junction-scale sockets, the real LM339 vendor model in the ladder, and a matched-pair model. This also replaces the ideal-gain MC proxy with converging real models.
4. **Remaining fold types.** Analog cos/exp for looks ≠ 1 (Atomic e/π, HEP 1−poof/π with hits = 1): a translinear sine shaper or a cos-from-exp identity block, completing all 35 folds.
5. **Seed constant generator board.** Analog generation of the layer-1 and layer-2 constants (sin/cos via translinear shapers, ln via log-ratio) instead of locked resistor ratios, so the circuit derives K, P_new, A and O itself.
6. **More closed forms.** The 14 fold-using rows (H0, n_s, σ_8, V_us, V_cb, Ω_Λ, …) on the S buses, and the 9 floor rows via further comparator quantizers.
7. **Transient and stability check.** Full .tran start-up and phase margin per loop, plus noise (1/f of the choppers vs the junction-scale nodes).
8. **Range splitting for 1/α_em.** e³ built as a cascade of three e¹ stages, each trimmed at its identity, to reach the ~1e-4 analog ceiling.
