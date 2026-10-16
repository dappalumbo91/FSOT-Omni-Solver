# LOCK8 (locked8) — leaves, part C: domain_scalar, fsot_correct, hub dynamics

Locked 2026-10-15, before any LOCK8 simulation. Forks (earlier locked files unchanged): phase3/cpp/leafc8.cpp from leafc7c.cpp (Amendment 7.2),
phase3/cpp/netgen7.cpp from netgen6.cpp (LOCK7). No FSOT rule, constant or tolerance is added or changed; every value below is computed in
C++20 long double from the pinned engine (FSOT-2.1-Cpp 3466e8d, pin AEB2AD) or, for the hub coefficients, from the formulas of
vendor/fsot_dynamics.py (sha256 837817ba3b04...) evaluated on the engine constants (cross-checked against that file in Python: all 5 agree to 1e-16).

## 8.1 domain_scalar (13 forms, netlist C_S)
`domain_scalar("X")` (engine: the full-core-law S of domain X) is now a compiled identifier: the S output of one full-core-law fold channel
for domain X (netgen7 recipe line `DCHAN <prefix> <domain>`; node vS<prefix>, V = 2 S, the same channel and scaling as S_COSM / S_QUANT /
S_CHEM in T1). Physical meaning: S is the domain's FSOT scalar (the dimensionless "state" of the domain from its D_eff, recent hits,
observation flag and look type); cross_species S(species) are the Neuroscience S, Density(species) = neuron count / (|S| x volume), and the
three Cross_* predictions are ratios S_A / S_B between sister domains. Leaves: S(Human, Octopus, Corvid, Honeybee, C_elegans), the 5 Density
forms, Cross_Opt_QO, Cross_MatSci_CM, Cross_Astro_PS (7 domains: Neuroscience, Optics, Quantum_Optics, Materials_Science, Condensed_Matter,
Astronomy, Planetary_Science).

## 8.2 fsot_correct (10 factors, netlist C_S)
engine.hpp:240 `fsot_correct(m, X) = m (1 + |S_X| ALPHA)`: the Ledger-B correction applied to a measured value m in domain X (a correction,
not a prediction). Compiled as the factor F_X = 1 + |S_X| ALPHA (m is the external measured input, a unit resistor gain here) for the 7
domains above plus Cosmology, Quantum_Mechanics, Chemistry (10 DCHAN channels in C_S). Targets: ENGF->fsot_correct(1, X).

## 8.3 hub dynamics (6 coefficients, netlist C_H)
From vendor/fsot_dynamics.py (the FSOT fluid/relaxation hub; not part of the pinned engine, so its targets are its formulas in long double):
- gamma_rel = |CHAOS| + PSI_CON POOF (line 143): relaxation rate of S toward its fold value.
- kappa = (POOF + SUCTION) A_BLEED: inter-scale coupling (bleed) coefficient.
- mu(D) = |CHAOS| |D - 25| / 25 + A_BLEED POOF (line 84): scale-dependent viscosity; leaves mu_Fluid at D_eff(Fluid_Dynamics) = 12 and mu_D25
  (D = 25, the bleed floor A_BLEED POOF).
- J_obs = C_FACTOR cos(delta_psi + THETA_S) (line 97): observer source term, evaluated observed with delta_psi = 1.
- c_s^2 = C_EFF / PHI / rho (line 102): effective sound speed squared at rho = 1.

## 8.4 Floor rows
The floor rows were already compiled in LOCK7c netlist 2a_Q (9 rows, ideal error <= 6e-6); nothing new.

## 8.5 What is unchanged
The 12 LOCK7c netlists (GEN, GENpp, T1, 2_*, 2a_*) are byte-identical in locked8 (design/recipe/generator files equal to locked7c's;
verify_lock8.sh checks), so results/leaves7b_* remain their results. leaves8_targets.tsv equals leaves7_targets.tsv except the 13
domain_scalar rows (previously "not compiled: unknown identifier domain_scalar") and 16 appended partC rows. Taps (taps7.tsv,
design7_taps_values.tsv) are copies of LOCK7's (re-derived by tapc7 in verify_lock8.sh). Run set: tools/batch8.sh (C_S, C_H; ideal 0/27/50 C,
mcC 1-8, mcB 1-4), tools/run_bs8.sh, solver blocksolve4 unchanged. CI: tools/verify_lock8.sh.

## SHA256
    471c79ca390528ae1feedb4290fd7b2e89a4d37335866921b049f11c0625e883  design7_taps_values.tsv
    13c402f3d8022bd9202e6db72eb02dd3a6fb55ba5a33f5aacdd9082291c19c82  design8_2_M_values.tsv
    7bc999dfc84d8a11133aeb8a226867c3efd265ff513745fd4a926ab618aaaf00  design8_2_P_values.tsv
    038526a9b1b3c089cbef84b47438e9c742464024a60098ac642594536c7359cf  design8_2_T_values.tsv
    05715f0707f55160456a53b4c7050cbdf44ed8f1de4015366ff612bfe7c9d108  design8_2_X_values.tsv
    0081c3b300046bcc3f5cea6de7b31514cd6799b0547dbdda8b28dd5aaa4023a9  design8_2a_M_values.tsv
    55e3558d43b29bc4709dbb07f3bf421bd2e50ed858810408d85ea250ad635b29  design8_2a_P_values.tsv
    f9227814333ac11d001ca939b573a63511d17ccbe77e18900209f2ea4cce7c5b  design8_2a_Q_values.tsv
    6081d0a4b07f45877551fb5e93797f659662b0915944e21470e880b42b7cc1d9  design8_2a_T_values.tsv
    a78dd229a7336a8bcb77fc670b14a66b6b7adc1e053db449834b7ffdd25ca2a8  design8_2a_X_values.tsv
    010386a98d4daf269bafab91916d16dca5d7baa034d50eb76ada93b44a1061fb  design8_C_H_values.tsv
    a0f3404d02f547ea78b98074233138d92c8d7ade5b61ca3fed62d262c47c1762  design8_C_S_values.tsv
    7d54cff43c213415fef11a351f304002477387744f17320f17f6f7d6f09f7dc1  design8_GEN_values.tsv
    4b23b68cb9ca2708215cdfa0c3ea47502851f6f4c5d4863c22d5817bc8256e59  design8_GENpp_values.tsv
    e4105dd56217a153713e0bfa0b81d15504318eff6df2f3d78436b0c71dea8990  design8_T1_values.tsv
    a39e58113dfbc352016a8d8c8ee29caf7b3afa4e65c3c309371dee12d779ee8c  generator8.tsv
    23a2000793ec6401d4ca2f898d328a99f4aac168b84428a1876ea8cec5cab771  leaves8_targets.tsv
    56faae89e5ee7b8150dfcb413fd6ac64f06cf3b90adb47a2c2a2a3572775e110  netlists8.tsv
    3b97a95adfc6f7d37c6c3df53b5401fa88e04246c3af53ced43e5c39dce310f9  recipe8_2_M.txt
    045a5d2dec938eb463eb484c9ae803796113385157867ee83b05388a94ab53e7  recipe8_2_P.txt
    da0dc74b098a1673903aa6e0b1d4f65d7cae97c93882eb6f2b922d6c30dc216f  recipe8_2_T.txt
    e410132fed960df8332cd50680e42bcf2aeea009217d87c03d69853335bf2be7  recipe8_2_X.txt
    cbd822cc5785ee1ae8f0125952939690c15a250086f1330dafd1d9c6ac012bd6  recipe8_2a_M.txt
    c873df626af770bf8cb5288bedb410e9b04ea8d26faaf51705791f262c84dc61  recipe8_2a_P.txt
    717ad6da3234ea5f6e12b12a9d182a6670592aece10e1e8e94717cbf4262e91c  recipe8_2a_Q.txt
    b3f2fba06f09685d6ebe7d77315e80a21576f6bc24af61effba90bdd29540f2d  recipe8_2a_T.txt
    ae560ff0801ed08a4f2ec2dc891d313564ace2b68c7161b17624621e7dc29d42  recipe8_2a_X.txt
    80c2121e5c267a40381bb85fed3e5fdacc8af77ae8e67c89a5122dc22ec2568b  recipe8_C_H.txt
    6c4505ce5ba218c4fcfcdd91cc76adcca46a0ccbc6e8c8e060b276bbae4ce1ba  recipe8_C_S.txt
    243cd7cee1a48068706464137bf5ea7901beb756d12cc89628c2f66e775974b6  recipe8_GEN.txt
    94358406ebc1542bbb2366bd385689f8506ff16389af687ef857ee227dd4ecbd  recipe8_GENpp.txt
    4d37145044a086b8d41d935a5bae6ab2f589d55b3b31d354db1f19e493a3a884  recipe8_T1.txt
    0bafb0b56742cd097598c9914c60403331c4e7d354dbf139a6bfe78f215a4e8f  taps7.tsv
    fb6da69e6c4659cb66669d7318fa48a3a7cd655c8b0913ddd32471f9d49a7604  ../cpp/leafc8.cpp
    588e97dabb2bc5a4961cc380cd53c769a662f17b0134f9aa35f01a0acc647606  ../cpp/netgen7.cpp
    2c2b0a0ea24a43b39826d95ca327bd2908d0bdc3b6c16f32ac263ec016891798  ../tools/run_bs8.sh
    3ad072dbef6d51b3b533697567320814e944cdf08d79c52a56bc05d2bec55308  ../tools/batch8.sh
    6e8c04bcfc820f5ac04dd99d26f635f1a6ad7f5d584dfdc19ac46c0c8242c265  ../tools/verify_lock8.sh
