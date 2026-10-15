# LOCK7 — FSOT Omni-Solver: near-zero summer constants, matched ln-256 neper reference, deeper base-engine baseline, Early correction, range-sized readout

Locked 2026-10-15 before any LOCK7 simulation with FSOT values. (Toolchain dry run only: leafc7 in its DEV mode, NON-FSOT base values pi->3, phi->1.5,
gamma->0.5, G->0.9, netlists 2_X, GEN, 2a_X, 2a_T ideal 27 C, to check that the flipped topologies converge; no FSOT value was simulated.)
Engine FSOT-2.1-Cpp 3466e8d, golden golden_AEB2AD.tsv, leaves_map.tsv from LOCK4 (leafmap4, unchanged). No FSOT rule is new or changed; no tolerance
is changed (mcB/mcC as before, except the stated matched-pair assumption 7.2); solver phase3/cpp/blocksolve4 unchanged. No locked file is edited:
every change is a fork — phase3/cpp/design7.hpp + derive7.cpp (from phase2/cpp/design2.hpp, derive2.cpp), leafc7.cpp (from leafc5.cpp),
tapc7.cpp (from tapc6.cpp), netgen6.cpp (from netgen5.cpp).

## 7.1 Near-zero summer constants by ratio choice (leafc7)
LOCK5 put every compiled signal LOGR at Ir = Ix e^-3 with the signal on the transdiode side: n = -V_T ln(Ix/Ir), weight t = -1, constant ln av - 3.
An antilog output at IUo = IIN e^-3 then needs the exponent constant c = -3 - 3W for a product with net log weight W: the mean |c| feeding the
cascoded antilogs rose to 7-12 nepers (GEN 11.9, max 47.8), and every neper of constant carries the V_NEP bus error (the LOCK5 MC spread grew 1.5-2x).
LOCK7 flips the signal LOGR: signal current on the cascoded side c2 at IIN e^-1.5 nominal, reference on the transdiode c1 at IIN e^+1.5, so
n = V_T ln(Isig/Iref) = -3 V_T nominal (the LOCK5 headroom is kept: 3 nepers before the cascode loses its drain-source voltage) but t = +1 and the
constant is ln av + 3, giving c = -3 + 3W (W = 1: 0). Each antilog then absorbs a further a nepers into its input resistor: I_in = IIN e^a,
a = clamp(c0, max(vx0 + 3, -3), 1.5) (vx0 = nominal base voltage in nepers; the cascode keeps vx <= -3 V_T and I_in stays in [IIN e^-3, IIN e^1.5],
4.48 uA max, high-injection error <= 7e-5 neper). e^+-1.5 are literals to 20 digits checked against fsot::m::exp at compile time.
Constant-generator base cells keep the LOCK5 orientation (constant -ln m per unit weight; flipping would add 3 nepers per unit weight, and the generator
raises base cells to powers up to 13) but m is now the smallest integer with m c >= e, i.e. >= 1 neper of cascode headroom (pi 1, phi 2, gamma 5,
G_cat 3; LOCK5: m c >= 1.4 = 0.34 neper minimum, gamma 0.55, G_cat 0.60 — the 2_X mcB seed 3 failure sat at the G_cat cell).
Result (summer constants feeding cascoded antilogs, mean |c| / max |c| nepers, LOCK5 -> LOCK7; tools/consts7.py):
GEN 11.86/47.8 -> 5.98/33.7; T1 7.20/23.1 -> 2.41/15.7; 2_M 9.25 -> 5.97; 2_P 8.07 -> 4.84; 2_T 10.93 -> 6.11; 2_X 7.15 -> 2.79;
2a_P 4.08 -> 3.40; 2a_T 6.25 -> 5.64; 2a_X 4.39 -> 3.56; 2a_Q 4.03 -> 4.15; 2a_M 4.57 -> 5.37 (2a_M/2a_Q: literal-dominated, slightly worse).
The largest remaining constants are (a) the BETA readout (+33.7: the 38.9-neper exponent pi^pi + e - 1 is a PTAT-scaled V_NEP voltage, so this
constant tracks the bus and cancels to first order) and (b) literal Taylor coefficients 1/k! of tiny series terms on uncascoded antilogs (below 1e-4
of their sum by construction).

## 7.2 Matched ln 256 neper reference (design7.hpp, netgen6)
V_NEP = (10/ln MPT)(2 dV_B - dV_A) with MPT = 256 (was 16): cells A 64 uA : 0.25 uA, B 32 uA : 0.125 uA, dV = V_T ln 256 = 143.4 mV (was 71.7 mV);
R_ptg_B = R_SUM ln256/20, R_ptg_A = R_SUM ln256/10. The PTAT transistors (ptA*, ptB*) are specified as a matched pair: in netgen6 their V_BE draw
is min(variant sigma, 1e-5 V) in every variant (monolithic matched NPN pair class, e.g. MAT12 / dual 2N3904 on one die). This is an ASSUMPTION about
the part (stated, optimistic for two discrete 2N3904s); with it the V_BE contribution to delta V_NEP / V_NEP falls from ~0.4% (mcB, LOCK5) to ~0.02%;
the resistor tolerances of the PTAT network (~0.1-0.15% in mcB) then dominate.

## 7.3 Deeper baseline for the base engine's own log stages (design7.hpp, derive7)
Every LOGR reference fraction of the base engine (N, P, cosy, a4, a5, y, w, sp, v, vp, ch, v2, ac, ph, t, tp, sc, am, oS, o1) is multiplied by
KD = e^-3/0.7: the LOCK2 references sat at ~0.7 of each stage's smallest operating value (n >= -9 mV; the LOCK6 T1 mcC failure was LOGR_vp at
-9 mV margin), they now sit at e^-3 of it: every base-engine LOGR margin is <= -76.6 mV (domain_margins.tsv, check n <= -2.9 V_T, 0 violations).
Db is unchanged (the fold current itself drives the cascoded side). The T1 antilog runs I_in = 7 uA with I_A = 7 uA/(2.5535 e^3) (vx <= -3 V_T at
|T1|max); the h/N antilog keeps I_in = I_PT/0.7 and its TIA takes the factor 0.7/r_N (R_hN_T = R_PT 0.7/r_N). T2 and growth antilogs are tied to
the S-node I_U scale and stay at -8.7 mV (noted, not changed). Summer constants follow from the same references (consts(); e.g. T1 -4.82 -> -20.72
nepers: the deeper baseline costs constant, which the V_NEP improvement 7.2 is meant to pay for). Parts above 20 M (R_yr 234 M, R_wr 39 M, R_o1r 22 M;
also R_em1, R_eiin) get T-network rows <part>_ta/_tb/_tc (Ra = Rc = 3.3 M below 60 M, else 9.1 M; Rb = Ra^2/(R - 2Ra)) into the virtual-ground node;
netgen6 builds the T when the rows exist. folds35_targets.tsv is byte-identical to phase2/locked.

## 7.4 Early-effect correction on uncascoded output stages
(a) leafc7: an uncascoded antilog (CALU, sum terms below 1e-4 of their sum) has Q4's collector on the virtual ground and its base at vx < 0, so
V_CB = -vx and its current is high by (1 + |vx|/VAF), VAF = 74.03 V (the 2N3904 model). The input resistor is raised by that factor at the nominal
vx (27 C). (b) tapc7: the log-carry divide-back Q4 (base at -n V_T) gets R_ldi_n = R_lcT (1 + n V_T0/VAF) instead of R_lcT. The correction is exact
at the nominal operating point and VAF; the VAF tolerance (+-20% in MC) leaves 20% of the uncorrected error.

## 7.5 Range sizing for resolution (leafc7)
A linear sum whose largest term exceeds its result by kappa > 1e3 is designed for rho times its nominal result, rho = 10^ceil(log10(3 sqrt2 1e-3 kappa))
(3 sigma of a two-term difference at 1e-3 relative term error); rho propagates through |x| (FWR), its LOGR (av rho) and the readout gain.
Only Chain_consistency_% qualifies: kappa ~ 1.5e4, rho = 100, readout 10 V/% (LOCK5: 1000 V/%, the circuit's ~1-5% MC spread was 30-230x the
nominal and saturated it). The readout now spans +-0.8% consistency at full scale.

## 7.6 GENpp (exploration, leafc7)
BETA = 1/exp(pi^pi + e - 1) is compiled a second time with pi^pi = pi^3 exp((pi - 3) ln pi) (exact identity; the integer part is a resistor weight of 3
on the pi base cell, only the 0.162-neper remainder goes through the multiply-in-log path). netlist GENpp, generator7 row BETA_pp. GEN keeps
exp(pi ln pi).

## Harness
tools/run_bs7.sh, tools/score7.sh (GENpp rows), tools/summ7.py, tools/batch7.sh, tools/consts7.py, tools/verify_lock7.sh.

## SHA256 (recorded before simulation)
    8cffd462a56eda575b360304de56e7b5920672f96b17713a953f2059829662e4  design7_2_M_values.tsv
    377623178b3e1a2d4720362ce1ab8fdcd78007b4101ad1861d689bf86e2a17bd  design7_2_P_values.tsv
    5716b72b6e99b6bd431615119dd2404d11c806355d46743a4dbce417616eb090  design7_2_T_values.tsv
    326334d8611a1b9f6094931f0bc853adf786c434e02b3dea8e2949219845fd09  design7_2_X_values.tsv
    0081c3b300046bcc3f5cea6de7b31514cd6799b0547dbdda8b28dd5aaa4023a9  design7_2a_M_values.tsv
    1f36ffc048323334e14679705a30dfe3ea3cab7c272fb000b4f3552eb6e649ee  design7_2a_P_values.tsv
    f9227814333ac11d001ca939b573a63511d17ccbe77e18900209f2ea4cce7c5b  design7_2a_Q_values.tsv
    d0a7435a413de0e0eb405d0f92739b6354707865d0c0f3933230597807c224f4  design7_2a_T_values.tsv
    2d1ca42d269bdf908e8bf9838536c6dc7480492f9b3c7abfdcdaedf16c16d6ff  design7_2a_X_values.tsv
    44d929065d0bbe0bc0847dfad5332a671f4ee4385ef132fe1484c8868b0fc8e8  design7_GEN_values.tsv
    be202e033104b156eef7c4fea84986655c4f4ea738b3edd4ccba1df95c185e70  design7_GENpp_values.tsv
    53a550ca170f5acfdd541bdc9fd2b06801482a132457c001b9be31232e27040c  design7_T1_values.tsv
    471c79ca390528ae1feedb4290fd7b2e89a4d37335866921b049f11c0625e883  design7_taps_values.tsv
    a39e58113dfbc352016a8d8c8ee29caf7b3afa4e65c3c309371dee12d779ee8c  generator7.tsv
    aaf5e1226a929610c3da0feff71d381078c771e604ed898965d62e7077cd8d6c  leaves7_targets.tsv
    ebef8aea3d579f851b2d6e11ca7790e67b31cd49141deb0ea590d2f7469ce977  netlists7.tsv
    52230c1465e9ee699c99a1737742a9720411f2ba23c2f85321e4c43dd8b5e0a8  recipe7_2_M.txt
    a45b739c5f7749c27276e2ddec8994ab26bef8f59beb37e933fe224ec0e94ef0  recipe7_2_P.txt
    72a0d46f49749fa44560b9adb0f3f037c6af1d23c5401d2bdfc976597c31eb4b  recipe7_2_T.txt
    4cbb0a1143820943b1c1b3dbc300ddea576dc98b7c4224a8718457a27b64c855  recipe7_2_X.txt
    cbd822cc5785ee1ae8f0125952939690c15a250086f1330dafd1d9c6ac012bd6  recipe7_2a_M.txt
    ef3fe5ae7f6c14a7f58a26ede1db2ff43f1db4e953a1bf9f0c27cc8d953092dc  recipe7_2a_P.txt
    717ad6da3234ea5f6e12b12a9d182a6670592aece10e1e8e94717cbf4262e91c  recipe7_2a_Q.txt
    18a63a925b1f4829f4d25a1aad30f8e88c027eeebd7baa61dd8896e0bb42743c  recipe7_2a_T.txt
    a993120d084168f0b8a30b654cf7a3afef52d07905ce81a773a470fcb380f6bd  recipe7_2a_X.txt
    0fcdc8280a4e1caa14be0075684b52c25365c67d8953a1b859332476e055acd0  recipe7_GEN.txt
    0f0647dee5ca88b09cb40ae37b4d6871eaa80c58e394104e74de68da7ea66f21  recipe7_GENpp.txt
    ae215182e28bf3ae3e95492b5673ae0bb48367915862bc4520d0d1dd3335af95  recipe7_T1.txt
    0bafb0b56742cd097598c9914c60403331c4e7d354dbf139a6bfe78f215a4e8f  taps7.tsv
    159015208b2a905c714e70cea5ef651148b09b81d876d0a27ac8f0e7ed2de9d7  base/design2_consts.tsv
    c04a2367a10298d41b67d93beeaf30818fa0ed3a0b9720062b0bed65fe079dc2  base/design2_values.tsv
    b567cd2fcf129838c5fefaf6216f419439c3a7c514ac07d154c7236333e01f19  base/domain_margins.tsv
    bd12292332c847919448e22307fbe7ee082d9ca566942d17e1125691aaecb55b  base/folds35_targets.tsv
    448673f207f3e0c9ced5879b0af36ad661152bc5d07401380e6328a7eea7ca11  base/targets2.tsv
    2f69a7e18e477e4b52e8349e4529174e6385110a4f2294083125f44339a2b4d9  ../cpp/design7.hpp
    c2fb2fa94282342deb63052c47bbfe42566fdf89a9f5a35da938e38671e55f56  ../cpp/derive7.cpp
    6ee31b89c4c81354e9e975034aac5f3fb5453c3fa3a8b2c16764b50c90a5de7e  ../cpp/leafc7.cpp
    e8996c0617a0e82490da2e57451d1dce48dccea8f72fed0de890171809a311a5  ../cpp/tapc7.cpp
    ecb5a76ab36df7993d28c66114bec40f2f2779accfa677f3856bea21893f701e  ../cpp/netgen6.cpp
    5fee5f70115c5ec80389738d04b8a299755431dcd410b635f651b85aec202045  ../tools/run_bs7.sh
    f4b6475441ceabb9c60b80c3431b218c7e5b79ab2b85fccb658652225013abee  ../tools/score7.sh
    a5cae3a206c32152897c545b5d4ef0e4cbb553fc9ac74a8a918def4ebafcbe36  ../tools/verify_lock7.sh
