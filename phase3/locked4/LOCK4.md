# Phase 3b LOCK: the constant generator, the tier-1 re-point, and leaf tiers 2a and 2 (compiled)

Recorded 2026-10-15T17:06:58-04:00, **before the first simulation of any FSOT-valued netlist in this directory**. The hashes are committed and pushed, and the push is verified with `git ls-remote`, before any of these netlists is simulated.

## What is locked
- `leaves_map.tsv`: `cpp/leafmap4` run on FSOT-2.1-Cpp 3466e8d `closed_forms_core.gen.inc`. It gives each closed form's engine C++ expression with the local temporaries inlined.
- `cpp/leafc` is the leaf compiler. It turns every expression into analog primitives:
  - cascoded LOGR and CAL
  - inverting log-domain summers (V_T per neper; constants from the PTAT bus V_NEP = 10 V_T)
  - current-summing TIAs (linear domain; constants from V_REF)
  - Taylor-series sin, cos and acos
  - comparator ladders for floor
- **Constant generator.**
  - The 20 derived constants (ALPHA … C_COSM) are compiled from their own core.hpp definitions, quoted verbatim in leafc.cpp. They are not resistor values.
  - The circuit's only inputs are four base reference ratios and e.
  - The base ratios are π, φ, γ and G_cat, as current ratios m·c (m = 1, 1, 3, 2) through LOGRs.
  - e is 1 neper = V_NEP/10.
  - Literal numbers in the engine expressions (2, 5, 100, 0.333…) are resistor ratios. ln and exp of literals come off the PTAT bus.
- `design4_<key>_values.tsv` and `recipe4_<key>.txt`: one netlist per key.
  - GEN: the 20 constants.
  - T1: the 14 tier-1 forms on the Phase-2 S channels, re-pointed onto the generator.
  - 2a_* and 2_*: the tiers, grouped by block type.
    - M: monomial (one log summer and an antilog).
    - P: polynomial (current-summing TIA).
    - X: log / exp / pow with a computed exponent (PTAT-scaled antilog).
    - T: trig series.
    - Q: comparator ladder.
- `leaves4_targets.tsv`: every leaf with its tier, group, engine value, golden value (the engine expression matches golden to ≤1e-15 for every compiled leaf), output net, gain, and series remainder.
- **Series truncation:** each Taylor series stops where the next term falls below 1e-8 of the result. The remainder is reported per leaf. Smaller terms would put their antilogs below about 1e-14 A, under ngspice's abstol.
- **Not compiled:**
  - 13 literal rows (tier 0, nothing to compute).
  - 13 rows that need other Phase-2 fold channels (cross_species S(…)/Density(…), predictions Cross_*). These are planned as tier 1b.

## Disclosure: dry runs before this lock
The toolchain (leafc → netgen2 leafc → blocksolve) was debugged on **DEV dry runs** (`FSOT_LEAFC_DEV=1`).
- These used the same expressions and compiler with **non-FSOT base values**: π→3, φ→1.5, γ→0.5, G→0.9. S stayed at the already-simulated Phase-2 channel values. They ran at 0/27/50 °C, with mcC and mcB.
- No netlist with FSOT base values was simulated before this lock.
- The dry runs led to these changes, all made before this lock:
  - antilog currents of 1 µA in / 0.5 µA out (the 2N3904 IKF term adds (Ia−Ib)/IKF nepers)
  - base multipliers m with m·c ≥ 1.4
  - unity-scale ln summers (finite op-amp gain)
  - resistor strings above 10 MΩ
  - a sum-term accumulator for negative terms
  - uncascoded antilogs for terms below 1e-4
  - series truncation at 1e-8
  - two blocksolve verification fixes (Phase-2 LOCK Amendment 14)

## Hashes
- `design4_2_M_values.tsv` `165f378f2436e1493341bfccf7a42baf94ce876bfab94c25f1d9d3d87cf99f22`
- `design4_2_P_values.tsv` `7c392253e8f01ee3a6c6f1ca2227519d3575a61401d9f71a789735f0a97e92e6`
- `design4_2_T_values.tsv` `a755f8957f9735c2848c55ae5ac996c70855911d8c3dfa1f173c319be201ff3f`
- `design4_2_X_values.tsv` `2b198aab1fb74e22aeab84fcb60927e4f885af85e6cc215439d02e457d24e0a0`
- `design4_2a_M_values.tsv` `78ec39cbf587ee086818450f3e8dd10ae3353c475865f5405c7b7080e6983650`
- `design4_2a_P_values.tsv` `1b2a5671547cf36ab7c5b0057eadd2614e97eae8a930a360f41c34c9ef85e2cb`
- `design4_2a_Q_values.tsv` `a05d6b91340c6cb18c63fd96567fc9fd8ffdad5713cbc7bd63cf68a7c83f8ad8`
- `design4_2a_T_values.tsv` `fcbc60563c8925c6bf74730328d35a81c79330905b3a9e209acb31459eb09eb3`
- `design4_2a_X_values.tsv` `90e1df6e1dbb043d0549630d931c5d529fe9a1e2dcbd047f290ca219af22f011`
- `design4_GEN_values.tsv` `13c2409bf662d79ce8edc54a71c541d1e74a5ba525de35da9ca20359d47943d8`
- `design4_T1_values.tsv` `0e805aaec961be120caa7190d3c546adbebf596960536d1f18ca5507a36483cf`
- `generator4.tsv` `2f1f74387f4c1744d1dc919a901e3033a3abd568bb50e8b80d409fe1973bb531`
- `leaves4_targets.tsv` `84f2abdbdefd265ef16ce0ebab5bf5d5305ae2bdd808b1a9824806d73e57f7c1`
- `leaves_map.tsv` `3d6916edd79941f56b8d903f44d4189a6444206f0be647a12d09ac824ad532ca`
- `netlists4.tsv` `b01c9e558a8949dbdf6bc40a29c8c0d6f51c912012b95d2e8bb067933a706053`
- `recipe4_2_M.txt` `4456460337f35c15542faadb61651aa174fae549b1b839f67fa0453c1b8c997f`
- `recipe4_2_P.txt` `1014419e6158b4c93176acf3e33b638e171a86ce3ef11f7052fd66317d706ac9`
- `recipe4_2_T.txt` `38ea11f392cc71979566077850726172dd05adc447a55f0d2c5b874fd560f248`
- `recipe4_2_X.txt` `fa007ea5c10c40acbe738cbbd065cc840b3a913b2f456896377820689b70fe86`
- `recipe4_2a_M.txt` `4251b6c3f89b87e0f13c2e3a58a2317b281c63e9993ec730b44b0d6265e8d4b3`
- `recipe4_2a_P.txt` `42374f4b364cccfeae257fbacc67da92acd19cfddec44b067ad238fc6cee6298`
- `recipe4_2a_Q.txt` `6b1788e95627df06d922af851c018e3773488b2cf3b79e90a909d72ccc7be4c3`
- `recipe4_2a_T.txt` `ac2b2a0332fbf8439a4f390e58cf9aad479926ccadf921111a82ddd2ec1e78f3`
- `recipe4_2a_X.txt` `8577ecb102f56a3a6d377724db163cfd95bbfb3e54980f957dec0f21d6028e2d`
- `recipe4_GEN.txt` `45b2736e21f2d82a7d4ece1dd358c34e8cb0c8a632dc06766220f225eeaf403e`
- `recipe4_T1.txt` `3cd884851dc73c3de1de8ba775910a100f76a1469ebb9979e66bf995ac729c64`
- `../cpp/leafc.cpp` `b29099fe65b147f4fb1d1e22c8d6a624480b3c544d044e234cc3ce01804794e1`
- `../cpp/leafmap4.cpp` `77875db0c3a2f9ad9f1ad93708de93be9988b92fb0f6c5f21af4758e404f118a`
- `../tools/score4.sh` `9902b9b3fae3c73a712385535af5026b8f77de913f2995bcd67cbed7005e71f9`

## Amendment 4.1: 2026-10-15T17:13:54-04:00. Harness only: a path change; no hash changed.
Commit a1d5620 edited `phase3/cpp/leafmap.cpp` in place, adding the `cexpr` column and stripping leading braces after loop blocks. That file is covered by LOCK3, at hash ea0e55dc…. This broke LOCK3 in CI run 38807418734.
- `leafmap.cpp` is restored byte for byte from 996ea08 (ea0e55dc…), so LOCK3 is untouched.
- The extended version is now `phase3/cpp/leafmap4.cpp`, with identical content and the same hash 77875db0…, so `leaves_map.tsv` is unchanged.
- SHA256SUMS, the hash list above and verify_lock4.sh point at leafmap4.cpp.
- Rule from now on: a file covered by an earlier lock is never edited; it is forked.
