# LOCK5 — FSOT Omni-Solver Phase 3c (headroom, exact rewrites, full-wave rectifier, base-engine tiny taps)

Locked 2026-10-15 before any LOCK5 simulation. Engine FSOT-2.1-Cpp 3466e8d, golden golden_AEB2AD.tsv, leaves_map.tsv from LOCK4 (leafmap4, unchanged;
verify_lock5.sh checks the re-derived map is byte-identical to locked4/leaves_map.tsv). No FSOT rule is new or changed. No locked file is edited:
every change is in a fork (phase3/cpp/leafc5.cpp from leafc4b.cpp, phase3/cpp/netgen4.cpp from phase2/cpp/netgen2.cpp). Solver: phase3/cpp/blocksolve4
(LOCK4 Amendments 4.3/4.4), unchanged; tolerances unchanged.

## 5.1 Cascode headroom (leafc5)
Every compiled LOGR now runs Ir = Ix e^-3 (was Ix/2), and every compiled antilog output runs at IUo = IIN e^-3 = 49.787 nA (was IIN/2 = 0.5 uA),
with IIN = 1 uA unchanged. A log-domain node (the antilog Q4 base) therefore sits at about -3 V_T = -77.6 mV nominal instead of -V_T ln 2 = -17.9 mV,
i.e. the margin before the base reaches 0 V (where the 2N7000 cascode loses its drain-source headroom and the loop reverses) rises from 0.69 to 3 nepers.
"Larger antilog input current" is realised as a larger input-to-output ratio (e^3 instead of 2) rather than raising IIN to 10 uA: raising IIN would
multiply the high-injection error (I1-I2)/IKF by 10 (to ~1.4e-4), the ratio form keeps it at ~1.4e-5. e^3 is FSOT's own e (E3M = e^-3 to 20 digits,
checked against fsot::m::exp(-3) at derivation). Consequence: summer feedback resistors Rf = mx/IUo grow by e^3/2 = 10.04; resistors above 1.8 G (previously a hard error)
are built as T-networks into the virtual-ground node, R_eq = Ra + Rc + Ra Rc/Rb with Ra = Rc = 9 M (stats key R_tee). The base engine channel
(CHAN, LOCK2 values) is unchanged.

## 5.2 Exact algebraic rewrites (leafc5 table REWRITE)
|V_cb| = S_q/C_eff - S_q is compiled as S_q (POOF sin(THETA_S) (1+g) - g)/C_eff with g = G_cat/(pi^4 pi phi), using
C_eff = (1 - POOF sin THETA_S)(1+g) (engine core.hpp:124), so 1 - C_eff = POOF sin THETA_S (1+g) - g exactly. The two forms are identical in exact
arithmetic; leafc5 evaluates the rewritten expression and requires agreement with golden to 1e-15 (it agrees: engine column unchanged,
0.0419188788072565816). leaves5_targets.tsv column rewritten_from_engine_cexpr keeps the engine's form. Condition number (sum of |terms| / |result|)
of the implemented subtraction: 46 (engine form) -> 1.09 (rewrite).
Considered and NOT rewritten (no exact rearrangement lowers the condition number, the cancellation is between independent FSOT constants):
gamma_2_Stieltjes pi^-2 - gamma^4 (21.9; factoring (pi^-1-gamma^2)(pi^-1+gamma^2) gives 43.8, worse), gamma1_Stieltjes G_cat^2 + cos(e) (24.1),
theta_star P_BASE phi - gamma^2 = gamma (phi/e - gamma) (65.0 either way, P_BASE = gamma/e), Hashing_bound A_BLEED - sqrt3 G_cat^7 (18.0).
Omega_b h^2 and Omega_DM h^2 share (1 - S_CHEM), which the base engine already forms as one current node K(1/K - T1 - T2 - T3) (LOCK2): that is the
exact rearrangement, already in place.

## 5.3 Full-wave rectifier (leafc5, recipe FWR)
fabs(x) of a linear (non-log-type) argument is a precision full-wave rectifier (100 k / 200 k, two 1N4148, two op-amps) instead of a fixed sign
chosen from the nominal value. Used once: Chain_consistency_% in 2a_X, whose argument (6.7e-5 nominal) is below the E24 / Monte-Carlo resolution
and flipped sign in 12 LOCK4 runs.

## 5.4 Base-engine tiny taps (netgen4)
A power-series tap (cy_k, sw_k, sv_k, st_k) or the T3 tap whose nominal term (in units of I_U = 5 uA: x^{2k}/(2k)! for cy with x = y,
x^{2k}/(2k+1)! for sw (x = w), sv (x = v), st (x = dt = 1 rad), |T3| for the T3 tap; all from the locked phase2/locked/folds35_targets.tsv) is below
1e-4 is built without the 2N7000 output cascode: Q4's collector goes straight to the virtual-ground node (V_cb = -V(vx) > 0). Every tap output passes
a 0 V sense source VSN_<tap>, printed as i(vsn_<tap>) in fold mode, so each analog term is read against the engine. taps5.tsv is the decision table
(560 taps over 35 folds: 247 uncascoded, 313 cascoded). netgen4 also has mode leafc5 (FSOT_P5_LOCKED, design5_/recipe5_, FWR primitive).

## Harness
tools/run_bs5.sh (leafc5 keys and fold:<name>), tools/score5.sh, tools/summ5.py, tools/verify_lock5.sh (sha256 + byte-identical re-derivation).

## SHA256 (recorded before simulation)
    942ba05bd88f08d0e2f172a818904788153eba3e19e732a8a6c6e44736c1d606  design5_2_M_values.tsv
    f26edf4c1fe6e6088c9b7ed1b790e1e5fefc189dc6d4d445828ef500fa4a9ad7  design5_2_P_values.tsv
    511293dd77f4f02f79547dd256371191dd2076bde4bcf14374ff655cb009b94a  design5_2_T_values.tsv
    f064fd799f638ad097973fe1ee9136f7a0d744dc1f325bfd668aae9d690373ac  design5_2_X_values.tsv
    e56b035da1cd2f3cde47f0df104d99e74a7b0f3cf0e228f7161a4b215c7b4b77  design5_2a_M_values.tsv
    175ab17c5e6fceada2654dd4a8cbb246c5870f3826f9ca24bd1bb7e4377adee9  design5_2a_P_values.tsv
    937052ca00ecd8e7f8759619cb2505ce71847b8e8311ad744a979acdb1a63d63  design5_2a_Q_values.tsv
    6d55f519853449f2e7ff8261cbce8fd9709a42094a4f59a9c1d6e71a8bab6d36  design5_2a_T_values.tsv
    71d34aeb9a944b0d4a081e62577431c8a1cffe19dad4db6a09b2b3d1cf3ac35e  design5_2a_X_values.tsv
    d388030476cc4b5ca2ceb456e007a7e6656a1255e97e77255bfe72b900f1e7fc  design5_GEN_values.tsv
    bf110487bb543aa283ef3fbff52c342682f808e4fccf02a0b7af9ee17379f481  design5_T1_values.tsv
    0f24071f19e460bca9b97d4aa707db5c819957a7b0c2ef109ed64883b1288b97  generator5.tsv
    6f1c443188c26f0c4f2977d98d4c1687419e13461239c537211eff89b1942f54  leaves5_targets.tsv
    0e88789a84a808317d5d99c48b70d2c097b88f356992cbbd5e39818679a279e5  netlists5.tsv
    4149f1e31c852efedd526ba2be0982d01a1b0c468f3e8e6683e009f1c04596f2  recipe5_2_M.txt
    492017a8bf2a9ee2eb0019150e38712611cc2a30e1e3ca96902b47f4b7fec1b8  recipe5_2_P.txt
    5a1030013a65fbb4449edecf1a206b009d4c5424179f9328ac98c59a6a40899b  recipe5_2_T.txt
    5fd3117293d14715820a4993226d7b166c38edf4ad209add4a70f84b81f7bdbf  recipe5_2_X.txt
    27a9bbe225bfc74b62dcd75bb8d9d50fbf12eb4462201f1a8649901804eee90f  recipe5_2a_M.txt
    ae99f6d21ef9f965c677913d566519a916313262c187712a411a4a648993982d  recipe5_2a_P.txt
    717ad6da3234ea5f6e12b12a9d182a6670592aece10e1e8e94717cbf4262e91c  recipe5_2a_Q.txt
    c4cf5766f1f335b77599c501599d7117557f8debb36efe67759deffdfbee4c85  recipe5_2a_T.txt
    973933299173877f613fa9da18b9251fc14d14583b006cd2c8367f04fef7d26a  recipe5_2a_X.txt
    a14db9772865c3e1e6f87ae83b2514a30004c4a28690237ffecf6065aa297904  recipe5_GEN.txt
    ccb2d3f75a32d8c854e5adfa38c927e828cdda5311f564bbdbf81dc6d72bb743  recipe5_T1.txt
    c0a9f45fdd679f0ee5c2a08c2fb925ccb58f1cdbe66dece878127f05b9f47f36  taps5.tsv
    5c9e1a51fbf598230e364e57d4566ed14ff50ba29d36f5bba8b9b0a0a0968eb3  ../cpp/leafc5.cpp
    238d8d31bab024a513758b93dc7928bd1694ed9804919825cb740d3933a6302b  ../cpp/netgen4.cpp
    7fb1d7b1659b89ccf5bd5049278541d9ae763cf435ebda606905731283699f93  ../tools/run_bs5.sh
    87a5ee3bf17e4aebf9031f1f939baa385f5dedd7a8d6d9bc5132a34298b47c83  ../tools/verify_lock5.sh
    57235b4c402ae9e5bd2e1deeba2cca9adaea3fb9de31a8383048f6442d5fe2cd  ../tools/score5.sh

## Amendment 5.1 (netgen4 tap sense sources; harness only, no component value changes)
The first ideal T1 run with 57f6e14 failed at the block holding VSN_sw2c: a 0 V source on the tap collector ties the downstream virtual-ground node
(the LOGR summing junction, solved in a later block) into the tap's block, where it is undetermined (verify: V(snsw2c) 1.5e-5 vs 0).
netgen4 now senses each tap at Q4's emitter and base instead (VSE_<tap>, VSB_<tap>, both inside the tap's own block); I_c = i(vse) - i(vsb).
Topology and component values are otherwise unchanged; taps5.tsv is unchanged. One diagnostic ideal T1 run was made with the amended netgen4 before
this hash was recorded (it converged); the batch below reruns it.
netgen4.cpp: 238d8d31bab024a513758b93dc7928bd1694ed9804919825cb740d3933a6302b -> 63a7842984a9b3fc6cc1fa53d4225a1784309679952540aa875a54b1f1553b2b
