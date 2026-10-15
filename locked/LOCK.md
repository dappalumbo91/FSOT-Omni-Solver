# LOCK — FSOT analog solver Phase 1

Locked: 2026-10-15 11:53:17 EDT (America/Indianapolis), BEFORE any scored simulation.
FSOT authority: FSOT-2.1-Cpp 3466e8d4211d786fa7a430de1ee75131fb3b2e97 (/workspace/repro_3466e8d, read-only), engine pin AEB2AD.

Pre-lock development runs (nominal only, used to debug topology against device physics, never against FSOT targets) were deleted. Fixes made in that phase, each from device physics: (1) base-current error of the u/2 divider -> 20 ohm arms; (2) high-injection (IKF) -> unit currents 10 uA / 0.2 uA; (3) CD4066 off-leakage into the sum node -> CD4053 SPDT current steering; (4) Early effect in fixed-exponent antilogs -> collector held at base (V_CB = 0); (5) a scale bug in R_as_T (KAS/(I0/pi) -> KAS/I0); (6) TL082 open-loop comparators did not converge in .op -> LM339-class behavioral comparator with the same offset draw.

```
a2b55f606110b8598afc118581fbe4182b7bbc6c308e416bf91ccc364cc457a8  locked/design_values.tsv
6c91e443b2b4e326614a8668e003089d19a7e0471593574909599e624f516a10  locked/fsot_targets.tsv
4adaca0a8a61efd35b26648145c5b860398df42aed166d0cdfb90d78b28a72f7  locked/folds.tsv
d89c3c6da43cf5e17a906ee61599596598199afc722c7f9248c26c9c7f73a200  locked/ladder_thresholds.tsv
c121a65967e39b054a9867b0819237b75ef47b9896af0f558bd65081a57ebc70  cpp/design.hpp
425522f7782c0d720b5c131dd4859b8c8193352d657aa1f3855cb70ab1e5b11e  cpp/derive.cpp
3bdf875ee40aff46baa41d38609d57160a74339d5d8803922665610ff0dd12de  cpp/netgen.cpp
38d0edea672697fa271dd55b89af2c8ae121e00aae6d28f6e69e6c2b418fec2c  cpp/score.cpp
182b4b45cfaaba64cb6c2610d50a2333d3b243d68a7662ffad0038a7ac0e03ed  spice/run1.sh
adad2e131f728610963e0797670ea34d2857c0b26b3f49c6ce356a3213d89011  spice/models/vendor/ti_sloj070/TL082.301
4f27e17a7ceabebf61edddca7b7e187830c1924af417c40811255a8f3a879937  spice/models/vendor/sloj070.zip
```

Post-lock harness amendment (2026-10-15 11:55 EDT): spice/run1.sh gained a 60 s timeout per ngspice call and one retry without nodeset (the 50 C nominal .op stalled). No component value, topology or target changed. New sha256: `7cf7fc728b6e4a686cbe9833264cb3ddbad0073896c96d7bb1ce35ad3e8b7812`

Post-lock harness amendment (2026-10-15 11:57 EDT): cpp/netgen.cpp gained mode foldg<g> (fold channel fed from the physical g-string tap, for the per-generation Monte Carlo); spice/fold_mc.sh added. Existing modes unchanged. New netgen sha256: `4063a97b71d693309256991d21388e42a0f58b94ed0475e74f8a0cd336d04e1c`

Post-lock harness amendment 3 (2026-10-15 12:24 EDT): the full-slice TL082 .op Monte Carlo (run1.sh) failed to converge for 49 of 80 completed mcA draws (gmin/source stepping exhausted, 60 s timeout); 4 more "converged" to non-physical roots (logged SOLVER: FAILED, excluded). Scoring only survivors would bias the MC, so it was stopped at 88/200 mcA draws. The scored MC now uses spice/mc_idealop.sh: each draw is solved with 1e6-gain op-amps that carry that draw's TL082-distributed Vos (netgen opt "idealop", already present at lock; every resistor piece, BJT IS, reference and switch Ron draw is identical to the TL082 netlist of the same seed). TL082 finite-gain/input-stage effects are carried as a systematic from (a) nominal e24_tl082 vs e24 idealop and (b) the 33 cleanly converged TL082 mcA draws paired with the same-seed idealop solution (results/mc_mcA_paired_tl082_vs_idealop.tsv: |diff| <= 3.5e-4 on every quantity except Omega_b h2 near zero). The fold MC (fold_mc.sh) keeps the full TL082 macromodel. No component value, topology or target changed; netgen.cpp unchanged since amendment 2.
New file sha256: `9b465337c9797e4336f07397dca509aeda9c6f3cbec0cbf06f716b35cd155bfb`  spice/mc_idealop.sh ; fold_mc.sh `65ffea1bf0047beaf65fe852ada9bab3b1bdbbdb3785da6554ad37fe8c3b67d2`

Presentation-only change (not a solver input): falstad/gen_falstad_core.py adds a 1 kOhm isolation resistor in series with each log/antilog op-amp output (inside the loop, no DC effect). Falstad's op-amp is a static gain block, and without the resistor its Newton solve diverges at t = 0.
