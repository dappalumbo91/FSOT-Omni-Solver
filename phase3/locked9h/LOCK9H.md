# LOCK9H (locked9h) — 0-D hub relaxation bench (transient analog circuit)

Locked 2026-10-15, before any LOCK9H simulation. Purpose (Damian, 8:21 PM, task 2): run the hub's relaxation dynamics as a transient analog
circuit driven by the compiled hub coefficients and compare with the Python and C++ hub dynamics.
Model (vendor/fsot_dynamics.py, sha256 837817ba3b04...; not part of the pinned engine; no rule added or changed), 0-D modes on the engine constants:
- S mode: dS/dt = -gamma_rel (S - S_eq), S(0) = 0, S_eq = domain_scalar("Cosmology") (scalar_transport_rhs with v = dS = d2S = 0).
- dark Fluid mode: dv/dt = -mu(D_eff(Fluid_Dynamics) = 12) v, v(0) = 1 (momentum_rhs, rho = 1, one k = 1 mode: d2v = -v, unobserved).
- observed D = 25 mode: dv/dt = -mu(25) v + J_obs, v(0) = 1 (momentum_rhs, rho = 1, k = 1, observed, delta_psi = 1); v -> J_obs/mu(25).
Circuit (phase3/cpp/hubbench.cpp, new; detmath): per mode an ideal-op-amp integrator (50 nF), a state inverter and a one-quadrant translinear
multiplier of four 2N3904 (same model card as netgen, nominal; V_BC = 0) giving I_out = I_a I_x / I_r into the integrator node. I_a = coefficient
voltage / R_a. One hub unit = 10 ms; 8 units; .tran 5 us, gear, reltol 1e-7. Stage 1 (hub_stage1.tsv): the coefficient voltages are the LOCK8/8.1
compiled readouts from the ideal 27 C C_H solve (V(zA1xo) = 10 gamma_rel, V(zA96xo) = 10 mu_Fluid, V(zT101xo) = 10 mu_D25, V(zT104xo) =
100 J_obs) and the C_S Cosmology channel V(vSdh) = 2 S_eq, held as DC sources (a sample-and-hold of the stage-1 outputs; the full C_H/C_S netlists
are not co-simulated). Variant "exact" uses the exact target voltages instead, isolating the bench's own error. The initial condition S(0) = 0 uses
the same V(vSdh); the S output is formed in the circuit as V(outS) = 2 S = V(state) + V(vSdh).
References: hubref_cpp.tsv (C++20 long double: exact solution and RK4, dt = 1e-3 unit) and tools/hubpy.py (Python RK4 on fsot_dynamics.py's own
scalar_transport_rhs / momentum_rhs). Comparison: tools/hubcmp.py; run: tools/run_hub9.sh (re-derives and cmp's the locked values first).

## SHA256
    cf14613576f1ff2effba300f1adbc639d6df49c4b2faed4f13cf77963dc947fd  ./hub_stage1.tsv
    9340953f7815feeb8e23f8cab7549aef3601db6c8e06543b1d66ef8b91104e24  ./hubbench_values.tsv
    d9f8997eda14f29d23b444d62021fe77186416e236e1f25ad6f59324c8a9f7a5  ./hubref_cpp.tsv
    b1a69908e5f432c1096dce7db81959bf682cae53781a7b99ff98e0ced2d17d10  ../cpp/hubbench.cpp
    da923b2e0c241fab576b56e1acfb49e4cc0efaa618f7fdc14679cd21382fe541  ../tools/run_hub9.sh
    6c1d37a25efe6926ceb790a6d9426e0042e022a22496e2ff83c73da2384cce38  ../tools/hubpy.py
    78f2a3bfaf8f456d46b22562498c179bb1c468d8ab21bc4db629bd7cb2bef0e9  ../tools/hubcmp.py
