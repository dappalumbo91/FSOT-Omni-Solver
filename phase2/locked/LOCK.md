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
