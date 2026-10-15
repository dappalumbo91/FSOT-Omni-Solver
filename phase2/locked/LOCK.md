# Phase 2 lock (FSOT Omni-Solver)

**Locked:** 2026-10-15T14:34:46-04:00 (EDT), before any Phase-2 simulation.
**Engine:** FSOT-2.1-Cpp @ 3466e8d4211d786fa7a430de1ee75131fb3b2e97 (`-std=c++20 -O2 -ffp-contract=off`).
**Derivation:** `phase2/cpp/derive2 phase2/locked`. It prints `DERIVE2_OK parts=177 domain_violations=0`.

What is locked:
- Every component value, as an exact value plus its E24 series realisation, in `design2_values.tsv`.
- The summer constants in nepers, with the PTAT bus each one is taken from, in `design2_consts.tsv`.
- All 35 fold targets (every factor of every term) in `folds35_targets.tsv`.
- The readout targets in `targets2.tsv`.
- The domain margins of every log and antilog block in `domain_margins.tsv`. The criterion is a cascode headroom n ≤ −8 mV at 27 °C. All rows pass.
- The derivation and netlist generator sources.

```
2441eb69e86d759c17380768e4fbecb34590d16c867b2c100c2dad725af81bf1  design2_values.tsv
a15ec3e22cd07ccb560fa0339295873850a5b3c6a8f472e02e836e0855935317  design2_consts.tsv
bd12292332c847919448e22307fbe7ee082d9ca566942d17e1125691aaecb55b  folds35_targets.tsv
448673f207f3e0c9ced5879b0af36ad661152bc5d07401380e6328a7eea7ca11  targets2.tsv
28b89461048276bf42a0c1ea125dc9f09703de4f93bb334fb565e9a1f4b5c2c5  domain_margins.tsv
24324dc8a33639fb262a2e69258170e58bd979a3f3299b83592df490e281aab8  ../cpp/design2.hpp
01d6f0a08ad37f50232a5fb7e1f0fd24eefedb38a8fba1375bafc40a28406945  ../cpp/derive2.cpp
c29f4090067650056ed092f4e4010753daf76424866f8b4e454497247c0843d1  ../cpp/netgen2.cpp
```

The design units (VR 2.5 V, I_U 5 µA, readout scales, the reference fractions of the log blocks, and the antilog multiples) are scale choices. They were fixed in `design2.hpp` before simulation.

**Every FSOT quantity is used exactly as the engine computes it:**
- quirkMod = e^{C_F·P_var}·cos(δψ+P_var), gated by `observed`.
- T3 = β·cos δψ·(NP/√D)·(1+CHAOS(D−25)/25)·valve·acoustic·phase.
- The full 24-field ScalarInput.

**Math adaptations (exact identities, no rule change):**
- cos x = −cos(x−π).
- cos(δψ+P_var) = −sin(δψ+P_var−π/2).
- cos δψ = sin(π/2−δψ).
- acoustic = 1 + A_in/φ − ((A_in−A_bleed)/φ)·sin²δθ.
- Power series truncated where the next term is below 2e-8 of the block value.
- φ⁴ = 3φ+2.
- e³ = (4·e^{1−ln4})³, i.e. three e¹ stages with a ×e/4 exponent each.

## Amendments
(Appended below, if any, with their reason. Values derived from FSOT are never re-tuned.)

### Amendment 1: 2026-10-15T14:40:02-04:00. A physics budget fix applied uniformly, not tuned to FSOT numbers.

**What the first lock showed.** The first lock (hashes above, kept in the git history) was simulated exactly once: `fold ideal_ideal Chemistry`, ideal op-amps. Two things came out of that run:
- A wiring bug in netgen2. The growth TIA node name `tg4x` collided with generation tap 4. It was renamed to `ngr`.
- After that fix, S_chem came out −8.1e-4 relative and V_NEP came out +2.9e-4.

**Diagnosis: standard Gummel-Poon high injection (IKF = 66.78 mA in the 2N3904 model).**
- Each log or antilog block has a log-conformity error of about (I_big − I_small)/IKF nepers.
- At 64 µA, the PTAT cell error was 2.3e-4 of ln 64. It multiplied every neper constant, which totals about 7.6 nepers in T1.
- The locked design did not budget this effect.

**Changes.** No FSOT-derived value changed. The FSOT targets (`folds35_targets.tsv`, `targets2.tsv`) are byte-identical.
- **PTAT reference.** Now two ΔVBE cells, A (64:4 µA) and B (32:2 µA), with ratio 16. V_NEP = (10/ln16)(2ΔV_B − ΔV_A), which cancels every current-proportional error to first order.
- **Log-block references.** Moved to about 0.7 of the smallest operating value, keeping the 8 mV cascode margin. Antilog multiples were reduced (T1: 3.5 with a 2 µA unit; T2 and growth: 1.4). The ladder unit is 0.2 µA (D = 25 corresponds to 5 µA).
- **Harness.** ngspice now uses `itl1=5000 gminsteps=50 srcsteps=50`. This is solver iteration only.

**After the change.** The same netlist reads S_chem +1.6e-4 and V_NEP +7e-6. That is the only simulation run before this relock.

```
629b15e6f047e4f8ee3e57bf30d28fe61b5d72428e8d69711b0ddd3f9a9cea8a  design2_values.tsv
92091884a359fcfd36a0059b9db2296dc5e9b3761d0ad473ca13762b22a60b87  design2_consts.tsv
bd12292332c847919448e22307fbe7ee082d9ca566942d17e1125691aaecb55b  folds35_targets.tsv
448673f207f3e0c9ced5879b0af36ad661152bc5d07401380e6328a7eea7ca11  targets2.tsv
080691450ae56cb36c4b5392a1129fc1e1877bdbbb3052284b1e0b9e3613f1f1  domain_margins.tsv
b82488abdc50e3058c9453d3aab258c2c08a43a1c04c29981f65ace365e6be8b  ../cpp/design2.hpp
fb0b783c97b0796955f3062965c4c4f655d30b22c0c880e70426668b8aefff38  ../cpp/derive2.cpp
04e897b11b36cb52c169a8aa033662a44f3233b981b3074e206b2b5bea29ffde  ../cpp/netgen2.cpp
fb9c269ac71ed7e560f1c289659a87d9a10c4c621b9fa11227ae9965a7aea09c  ../cpp/score2.cpp
```

### Amendment 2: 2026-10-15T15:14-04:00. Harness only; no value, topology or target change.

`netgen2.cpp` got three solver/harness additions after Amendment 1, made while getting all 35 folds to converge:
- `FSOT_P2_NODESET`: an optional file of node voltages from a converged solve, written as `.nodeset` lines (initial guess only; ngspice then iterates freely).
- `FSOT_P2_CHPREFIX`: name prefix so per-channel seed dumps of the omb netlist can be merged.
- `#include <cctype>` for the lower-casing used by the above.

Check: with neither variable set, the regenerated `fold ideal_ideal Chemistry` and `consts ideal_ideal` netlists are byte-identical
to the ones simulated under Amendment 1. netgen2.cpp hash 04e897b11b36cb52… → `c55d37e9a8024d1e700ece49a1f0ccf361af2ad028ae942f24d4270c4b4a7980`. All other hashes unchanged.

### Amendment 3: 2026-10-15T15:21:17-04:00. New block (oven), locked before its first simulation. No existing value changed.

Phase 2 item 1 asks for the PTAT reference *plus an oven*. The oven is not an FSOT quantity; its values are standard physics and engineering choices, derived in `oven/oven.cpp` (C++20) and listed with meanings in `oven_values.tsv`: 2N3904 sensor at ~20 µA, T_set = 60 °C nominal (58.87 °C from the E24 divider), OPA187 error amp (gain −100) + PI integrator (R_i 100 k, C_i 10 µF, R_p 1.8 M), TIP31 heater (R_e 22 Ω at 9 V), block θ_sa = 40 K/W, C_th = 3 J/K, sensor-to-core gradient θ_g = 1 K/W. The netlist generator `oven/ovennet.cpp` is locked with it.

```
45a920dfe347f3c837cba0ed0dd27a167da31ba3a1bb0262d1b812ce7a7986d3  oven_values.tsv
e5df5c5f2b3e4ca1c556220bbc64f4371f46a8706d53ed139ecbe1c39f68a634  ../oven/oven.cpp
06d21bbe1b113740e7a1d6501497f1f9a094182b17f7013412605804773fcf34  ../oven/ovennet.cpp
```

### Amendment 4: 2026-10-15T15:53:46-04:00. Harness only (oven netlist generator); no value changed.

The first oven simulation (vendor OPAx187 in the PI loop) never started: ngspice found no operating point and the uic transient failed at t = 0 (the same Green-Williams-Lis convergence failure documented in docs/INSIGHTS.md). `ovennet.cpp` got:
- an `ideal` op-amp option: gain 1e5, one pole (GBW 1.6 MHz), output limited to the OPA187 swing of ±8.9 V;
- a fix to the thermal capacitor initial condition (IC = 0 K across C_th, so the block starts at ambient; it had been written as the ambient value).

`oven_values.tsv` is unchanged. ovennet.cpp 06d21bbe1b113740… → `ec641a3ce739c28dc3b24deb7de792ea9b9437649fd73864ff6b09139fc217d7`.

### Amendment 5: 2026-10-15T15:57:42-04:00. Topology (circuit-side), recorded before simulation. No FSOT-derived value moved.

**Hypothesis being tested:** the ngspice failures come from antilog outputs whose nominal current is too small for the cascode to control. These are the T3 output (about 1e-22 A) and the highest power-series taps (0.1–10 pA). Each such cascode op-amp loop is open (its output runs to the rail), and a 2N7000 leakage (Is = 5.6 pA) is larger than the signal.

**Rule.** For a CAL output whose locked nominal current is below I_DIRECT = 100 pA, the cascode (2N7000 + op-amp) is removed and the Q4 collector goes straight to the output node, which is always a virtual ground. The nominal current is computed in netgen2 from locked data only:
- for a series tap, I_U·x^{2k}/(2k+f)!, with x = y, w, v from folds35_targets.tsv and δθ = 1 rad;
- the T3 output is always direct.

**Accuracy cost.** Without the cascode, Q4 sees V_BC = v_x (−0.3 to −1 V), an Early error of |v_x|/VAF ≤ 1.3 % of a term that is itself ≤ 2e-5·I_U. That is at most 3e-7 of the unit, below every other budget line. For Chemistry, 7 of 42 cascodes are removed.

netgen2.cpp c55d37e9a8024d1e… → `1125298e866e4a806772ee013f31d61ea135ce3e38293630d714803084e1ec24`. All locked TSVs are unchanged.

**Amendment 5 outcome (2026-10-15T16:04:00-04:00): rejected and withdrawn.** With the low-current cascodes removed, the nominal fold no longer converges even from a plain start. Removing any one of them flips convergence either way (cy3, sw3, sv4: no; a3, sv3, st4, st5: yes). So the T3 / low-current cascodes are *not* the root cause. Convergence of these netlists is chaotic: plain Newton produces NaN by iteration 2 in every variant, and only gmin/source stepping sometimes lands. netgen2.cpp is restored byte-for-byte to the Amendment 2 version (`c55d37e9a8024d1e700ece49a1f0ccf361af2ad028ae942f24d4270c4b4a7980`).

### Amendment 6: 2026-10-15T16:15:17-04:00. Harness only (solver); no value, part or topology changed.

**What.** `cpp/blocksolve.cpp` (C++20) solves a netgen2 netlist block by block in ngspice. Every ideal op-amp output (E source to ground), every comparator output (B source) and every grounded supply is a *stiff node*: its voltage depends only on its own driver's inputs and not at all on what it feeds. Tearing the netlist at those nodes is therefore exact. A block is the set of elements joined galvanically between stiff nodes, plus the drivers they control. Blocks that read each other (strongly connected components) are merged. Blocks are solved in dependency order, and each one sees the stiff nodes it reads from other blocks as DC sources at their already-solved values, printed with 15 digits. Every block runs `.options sparse` (Sparse 1.3 instead of KLU), with a gmin 1e-12 and then a KLU / reltol 1e-5 fallback that is reported as "retried".

**Why.** The 900–1000-node netlists failed in ngspice for one reason: KLU on the whole ill-conditioned matrix (pA transdiodes under gain-1e6 loops) produced NaN by Newton iteration 2. The per-block matrices (at most about 110 elements) solve directly. `run_bs.sh` is the driver, and `mc2.sh` now uses it.

**Exactness check.** Particle_Physics fold, ideal: the block solution was used as the .nodeset of the full netlist (sparse). The full-netlist .op converged on the same point: S = 1.900685518 (block) and 1.900685518 (full), |ΔS|/S = 4.5e-10. With 6-digit interface printing the error was 4e-5, which is why 15 digits are used. The block result does not depend on gmin (1e-12 / 1e-15 / 1e-18 give the same S to 1e-9).

**Withdrawn alternative, not used.** A pseudo-transient continuation (an E24-nominal state with the draw ramped in over 1 ms) failed at the first time point even with a zero draw. It was removed.

blocksolve.cpp → `5373a4950343b4f4e3e43f7fa16bf2853b811efed2f3873f58e2c5ca91aca312`

### Amendment 7: 2026-10-15T16:22:42-04:00. Harness only (dynamic ideal op-amp for start-up, loop-gain and noise runs); recorded before those simulations.

With `FSOT_P2_OPDYN=1`, netgen2 writes each ideal op-amp as an OPA187-class large-signal stage instead of the DC gain-1e6 E source:
- a transconductance gm = 1 S, limited to ±58 mA, into R = 1 MΩ ∥ C = 0.2894 µF. That gives DC gain 1e6, a GBW of 0.55 MHz and a slew rate of 0.2 V/µs (the OPA187 datasheet values).
- an integrator node clamped at ±8.9 V (the OPA187 swing), so there is no wind-up;
- a unity buffer to the output.

Inside the rails the DC solution is identical: S(Chemistry) is 1.910885427 in both. The default output (no env) is byte-identical to the Amendment 2 netlists apart from the Amendment 8 lines below. Two earlier forms were tried and rejected. One was E→RC→output clamp: its pole capacitor wound up to ±1e5 V. The other was a clamp before the pole: its slew was 30 V/s.

blocksolve.cpp (same amendment) gained:
- a transient mode (`FSOT_BS_TRAN`): each block runs `tran … uic`, and the stiff nodes it reads are PWL copies of the upstream waveforms. This is exact for the same reason as the DC tearing.
- controlled current sources are kept in the block of the nodes they read;
- every DC block solution is verified by a plain Newton re-solve from it. This rejects ngspice's pseudo-transient fallback when it ends on a non-solution, which happened for the latched LOGR state.
- `spice/loopgain.sh` (voltage injection at each op-amp output).

### Amendment 8: 2026-10-15T16:22:42-04:00. Topology (compensation only), recorded before simulation. No FSOT-derived value moved; no DC value changed.

**Finding (Amendment 7 loop gains, Chemistry fold).** All 308 op-amp loops were measured. Every LOGR reference-side loop (op-amp B: c2 → base of Q2 → cascode → c2) is unstable, with phase margin between −75° and −158° at 80–170 kHz. Two poles sit together at crossover: the op-amp integrator and C_B·R_r. Every other loop has 59–96°. In the transient, the PTAT and every log block oscillate rail to rail.

**Change.** In every LOGR, C_B (c2 to ground) goes from 1 nF to 100 nF in series with R_Z = 1 kΩ, so the loop has a zero at 1.6 kHz. The crossover becomes GBW·g_m2·R_Z: about 83 kHz at 4 µA and about 8 kHz at 0.4 µA. Parts: a 100 nF X7R and a 1 kΩ 1 % resistor per LOGR (accuracy-neutral at DC).

netgen2.cpp c55d37e9a8024d1e… → `2cf2ffb3a7d3b85eef70878d14f9c70d155f7c1d32cce717f504340e4eece668`
blocksolve.cpp (Amendment 7) → `b7fc4b6127645a43caacbc7b93dd53283cc551f3b359edcacf25f8e3ab5b1102`

**Amendment 8 outcome (2026-10-15T16:23:11-04:00).** Insufficient. The B loops improved from −158…−75° to +2…+26° but still ring, because the cascode servo D crosses over at the same frequency (about 130 kHz, phase margin about 10°), right on the k-node pole of the 2N7000 running at µA.

### Amendment 9: 2026-10-15T16:23:11-04:00. Topology (compensation only), recorded before simulation. No FSOT-derived value moved; no DC value changed.

In every LOGR:
1. The cascode servo D becomes an integrator: R_D = 100 kΩ from k to its inverting input, C_D = 1 nF from the gate to that input. Crossover is about 1.6 kHz, well below the k-node pole (g_m/C_k, 16–160 kHz). No DC current flows in R_D with an ideal input. With an OPA187, I_B·R_D ≈ 10 µV of cascode offset, which only enters through the Early effect.
2. The B integrating network becomes C_B = 1 µF in series with R_Z = 330 Ω. The zero is at 480 Hz; crossover GBW·g_m2·R_Z is about 27 kHz at 4 µA and about 2.7 kHz at 0.4 µA, below the cascode's common-gate pole.

Parts per LOGR: 1 µF X7R, 330 Ω, 100 kΩ, 1 nF C0G.

netgen2.cpp 2cf2ffb3a7d3b85e… → `548d9aa42b8e5042e44849933dfd58a724e8a76ff1b124fd54388dff41308583`

**Amendment 9 outcome (2026-10-15T16:23:33-04:00).** 153 of 154 measurable loops have a phase margin of at least 68°. The exception is LOGR_y, whose reference current is only 0.15 µA (R_yr = 16.7 MΩ): it crosses over at 690 Hz, close to the 480 Hz zero, with a margin of 5.5°.

### Amendment 10: 2026-10-15T16:23:33-04:00. Compensation only, recorded before simulation. No FSOT-derived value moved; no DC value changed.
C_B goes from 1 µF to 10 µF (X5R/X7R), with R_Z = 330 Ω unchanged. The zero moves to 48 Hz, a decade below the lowest-current crossover (about 1 kHz).

netgen2.cpp 548d9aa42b8e5042… → `94ba01a08edfe7a47b8e9c343b9ce06a63a9b8493a446dee99f7f328194bab1e`

**Amendment 10 outcome (2026-10-15T16:24:11-04:00).** The minimum phase margin is 37.9°, still at LOGR_y (604 Hz), which now interacts with the 1.6 kHz cascode integrator. As an *exploratory, unlocked* check, the loop-gain run was repeated with C_D = 10 nF (min 52.6°) and C_D = 100 pF (min 57.6°). This is disclosed here because it was done before the value was fixed.

### Amendment 11: 2026-10-15T16:24:11-04:00. Compensation only. No FSOT-derived value moved; no DC value changed.
C_D goes from 1 nF to 100 pF (C0G). The cascode integrator then sits at about 16 kHz, above the LOGR_y crossover. All the other Amendment 9/10 values are kept.

netgen2.cpp 94ba01a08edfe7a4… → `5f9ed1d6512416489ff109df33b96f32c129ca7809b17635f37351cb0bc54c9c`

### Amendment 12: 2026-10-15T16:33:16-04:00. Harness only (noise and sensitivity runs); no value, part or topology changed.
- `FSOT_P2_OPNOISE=1` models the OPA187 input voltage noise (20 nV/√Hz) as the thermal noise of a 24.15 kΩ resistor in series with each non-inverting input. No DC current flows in it. Current noise is not modelled. In this mode VREF also carries AC 1 as the noise input reference.
- `spice/swfix.sh` resolves every switch to its own RON/ROFF at the operating point before `.noise`. ngspice's SW noise reports both poles of an SPDT as open, putting the 1e12 Ω thermal noise on the common node: 5 mV of spurious output noise.
- variant `mcX` with `FSOT_P2_TOL` drives one draw class at a time, for `spice/sens.sh` (per-part error coefficients).
- The start-up / loop-gain / noise numbers in docs/INSIGHTS.md §11 were produced after Amendments 7–12.

netgen2.cpp 5f9ed1d651241648… → `43149f14c07425b14048e4edd814c6c62ef4d7d3283f9c910f33d0c5d5d46fdc`; blocksolve.cpp b7fc4b6127645a43… → `b7fc4b6127645a43caacbc7b93dd53283cc551f3b359edcacf25f8e3ab5b1102`
