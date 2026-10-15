# Phase 3 LOCK — leaf tier 1 (closed forms fed directly by S_cosm, S_quant, S_chem)

Values derived by `cpp/derive3.cpp` (C++20, FSOT-2.1-Cpp 3466e8d / engine pin AEB2AD; uses phase2/cpp/design2.hpp and cpp/design.hpp).
Files written 2026-10-15 16:37:03 -04:00, before any simulation of the leaf circuit. derive3 refuses to write a passing run unless every leaf
value equals the golden `sec.*` value to 1e-15 (it does: max 1.8e-18).

**Disclosure.** The sha256 below were recorded after the first leaf1 solves (within a minute of the files being written), not before them, because this LOCK file was
written afterwards. The locked TSVs were not edited: their mtime is 2026-10-15 16:37:03, before the first solve, and `tools/verify_lock3.sh` regenerates
them byte for byte from the pinned engine. The only thing changed between the first (failed) and the passing solve was element naming in
netgen2 (Phase-2 LOCK Amendment 13).

## Design
- Every tier-1 form is `b + k·x` with x ∈ {|S_cosm| (vabs, +2 V/unit), S_quant (vSq, +2 V/unit), 1−S_chem (v1sc, −40 V/unit),
  1−S_quant (v1sq, −40 V/unit), |S_cosm|·S_quant (vprod, +10 V/unit)} and b, k FSOT constants straight from the engine.
- One inverting summer per leaf, R_f = 100 kΩ, output V = G·value with |G| a power of ten (|V| in 0.8…8 V); sign(G) = −sign(s_x·k).
  R_x = R_f·|s_x|/(|G|·|k|); the constant is a resistor R_c = R_f·2.5 V/|G·b| from +V_REF (vr) or −V_REF (vrn).
- σ_8 needs |S_cosm|·S_quant: same LOGR–SUM–CAL–TIA topology as the Phase-2 Ω_b h² product (references 0.36 and 0.6, log-domain constant
  ln 0.36 + ln 0.6 from the neper bus, TIA 10 V/unit).
- No new FSOT rule; no fitted value; pure analog (op-amps, BJTs, resistors, capacitors).

## Hashes
- `design3_values.tsv` `43e57fe913f20be04d6e33fc97dea7ef369cf0d18ad309d2ac9a0bfa4c681630`
- `design3_consts.tsv` `5937962415cd21058045fbf4f7548f2fa2a012fa9f522eae16816e3729803c39`
- `leaves3_targets.tsv` `df5360c45bbbdc3bcea77d1f6ef6caa8559484050df81bd75873745db7f72c74`
- `../cpp/derive3.cpp` `1163ffe81d200f5dad247e7e8f06bae0dcbebf9422bcba4c2ef35fe78920c3f0`
- `../cpp/leafmap.cpp` `ea0e55dc68b34b2518b82071d8d4e8fc5661e825bdc5329a731498a0a785c97b`
- `../tools/score3.sh` `a21624100db94b935229633738664f676df751d4fa95ec94f0eb703a87107e9e`
