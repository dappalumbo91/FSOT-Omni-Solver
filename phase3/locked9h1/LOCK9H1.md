# LOCK9H Amendment 9H.1 (locked9h1) — hub bench fixes

Locked 2026-10-15, after the first LOCK9H run and before any 9H.1 simulation. The first run (phase2/spice/out/hub9_*.{cir,dat,log},
results/hub9_run1_summary.tsv / hub9_run1_compare.tsv, kept as is) was wrong by construction; two defects:
1. R_a was sized as 10 coef / I_r, so I_a = I_r at the nominal coefficient: every mode then relaxes at rate 1/T0 whatever its coefficient
   (the observed mode settled at J_obs instead of J_obs/mu_D25). Fix: R_a = (readout scale)/I_r = 1 Mohm for all three, so I_a = coef I_r and
   the coefficient enters only through the compiled voltage.
2. .ic cannot set a node driven by an ideal VCVS (the integrator outputs), so the transient started at the operating point = steady state.
   Fix: the initial state is set as the integrator capacitor IC= and the transient uses uic.
New fork phase3/cpp/hubbench2.cpp (hubbench.cpp unchanged), tools/run_hub9b.sh. hubpy.py and hubcmp.py (LOCK9H) are reused unchanged;
hubref_cpp.tsv and hub_stage1.tsv are identical to LOCK9H's. hubbench_values.tsv differs only in the three R_a rows.

## SHA256
    cf14613576f1ff2effba300f1adbc639d6df49c4b2faed4f13cf77963dc947fd  ./hub_stage1.tsv
    78635be2301b8e0b76df4f6ef4a8d8fbb86f9b52a8cd8b4966b059fe8ec598a5  ./hubbench_values.tsv
    d9f8997eda14f29d23b444d62021fe77186416e236e1f25ad6f59324c8a9f7a5  ./hubref_cpp.tsv
    114b7b1910b396a85954220e5dcf73c26ae86d272bdf81a120f5704188704206  ../cpp/hubbench2.cpp
    528f024e68c40e2561c3a2ea5549cdd8dacb377219568e56c254e1c40db0031d  ../tools/run_hub9b.sh
