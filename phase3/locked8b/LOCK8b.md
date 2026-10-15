# LOCK8 Amendment 8.1 (locked8b) — host-independent derivation

Locked 2026-10-15, before any 8.1 simulation. Fork phase3/cpp/leafc8b.cpp from leafc8.cpp (LOCK8, unchanged); new phase3/cpp/detmath.cpp.
No FSOT rule, constant, tolerance or leaf is added or changed.

## Cause (CI runs 38821375699, 38823220674, 38823805490, 38824108037 failed; 38822916086 passed)
The LOCK7 leaf derivation re-derived different bytes on some GitHub runners. The failing rows were design7_2_M_values.tsv line 35 (LOCK7,
an E24_rel at 1e-19 noise, Amendment 7.2) and locked7c/design7_2_M_values.tsv line 595 (P_zS131xc, a summer-constant resistor 10000/0.15 =
66666.666... ohm). That value sits exactly on a decision threshold of cpp/design.hpp realise() (LOCK1, locked): two E24 pieces 62000 + 4700
give rel = 5.000e-4 exactly, and realise() searches three pieces only when "best.rel > 5e-4". A one-ulp change of the computed 0.15 decides
2 pieces (62000 + 4700) or 3 (62000 + 4300 + 360). The derivers call glibc's long double expl/logl/powl/log10l/sinl/cosl/sincosl/acosl, which
on x86-64 use the x87 transcendental instructions (fyl2x, f2xm1, fsin/fcos, fpatan); their last bit is CPU-model dependent and the runner
pool mixes CPU models, hence the intermittency. Evidence:
- glibc / compiler version is not the cause: leafc7c built in an Ubuntu 24.04 chroot (glibc 2.39, g++ 13.3, the ubuntu-latest toolchain) on
  this box's Intel Xeon reproduces locked7c byte for byte.
- A one-ulp perturbation of expl/logl/sinl/cosl reproduces the failure class: leafc7c linked with detmath perturbed by +-1 ulp changes the
  E24 pieces of the 66666.67 ohm parts and the 18-digit columns: +1 ulp changes design7_2_M/2_P/2_X/GEN/T1, generator7 and leaves7_targets;
  -1 ulp also 2_T and GENpp. derive7's folds35_targets.tsv (18 digits) also changes under +-1 ulp; derive7 and tapc7 linked with unperturbed
  detmath reproduce locked7/base and the taps byte for byte, so the base engine is protected by detmath's determinism (no CPU dependence),
  while leafc8b is additionally robust to a 1-ulp change (check_hostindep8b.sh: identical under +-1 ulp, -O0, -O3 -march=native).
- Amendment 7.2 snapped one symptom (the antilog exponent a ~ 1e-19) but not the threshold decision.

## Fix (8.1)
(a) phase3/cpp/detmath.cpp: expl, logl, log10l, powl, pow, sinl, cosl, sincosl, tanl, atanl, atan2l, acosl, asinl implemented with IEEE
    + - * / on the 80-bit format and exact scaling only (-ffp-contract=off, -fno-builtin). Linked into a deriver, these definitions take
    precedence over libm's for the deriver and the inline engine headers alike; verify_lock8b.sh checks that the linked derive7, tapc7 and
    leafc8b import no libm transcendental. Accuracy vs glibc: <= 1.8e-19 relative (log, exp, log10, sin, cos, acos), 3.3e-18 for powl with a
    non-integer exponent (exp(y ln x)); integer exponents |y| <= 64 by repeated multiplication.
(b) leafc8b: each part's ideal value is snapped to the 12 significant digits it is printed with BEFORE realise(), so the E24 decision is made on
    the printed value (66666.6666667: rel 4.99999999997e-4, always 2 pieces, 3e-15 relative from the threshold vs ~1e-19 noise).
(c) leafc8b prints computed node values (leaves8_targets engine column, generator8 engine column = the GEN scoring target, partC targets) with
    12 significant digits instead of 18/21; golden values read from golden_AEB2AD.tsv keep 21 digits.
Checks: tools/check_hostindep8b.sh (local, before pushing): leafc8b re-derives locked8b byte-identically with detmath perturbed +1 ulp and
-1 ulp on every expl/logl/sinl/cosl result, at -O0 and at -O3 -march=native. tools/verify_lock8b.sh (CI): run locally and in the Ubuntu 24.04
chroot before each push.

## Netlist changes vs locked8 / locked7c
derive7 (locked7/base) and tapc7 (taps) re-derive byte-identically with detmath, so the base engine and taps are unchanged. The 14 recipes are
identical. Design files are identical in the ideal and E24 columns except 7 parts whose E24 realisation is now the 2-piece one (rel 5.0e-4,
was 3 pieces at 1.0e-4; part tolerance is 1 %): 2_M P_zS159xc, 2_P P_zS152xc, 2a_X P_zS13xi1 (6200 + 470), GEN P_zS123xc, GENpp P_zS13xc,
T1 P_zS96xc and P_zS113xc. Ideal-variant netlists use the ideal values, which are unchanged; E24 (Monte Carlo) netlists of those 6 keys change,
so tools/batch8b.sh reruns them (results/leaves8b_*). E24_rel of exact values now prints 0 (was 7e-20 noise). C_S, C_H and the other 6
netlists are byte-identical to LOCK8 / LOCK7c and keep their results.

## CI verification retired (disclosed)
The byte re-derivation of the locked7 / locked7b / locked7c leaf files (verify_lock7.sh, verify_lock7b.sh, verify_lock7c.sh) and of locked8
(verify_lock8.sh) is removed from CI: those derivers use libm's x87 transcendentals and cannot re-derive on every runner CPU. Those scripts and
files stay unchanged; they still re-derive byte-identically on Intel hosts (this box, and the Ubuntu 24.04 chroot), and their sha256 records are
still checked in CI by verify_lock8b.sh step 1. LOCK4/5/6 verification steps are unchanged (they have passed on every runner so far).

## SHA256
    471c79ca390528ae1feedb4290fd7b2e89a4d37335866921b049f11c0625e883  design7_taps_values.tsv
    56862133b785e82802f89b709532046ea494f954eefa9c03d40f64a0a97e80f0  design8_2_M_values.tsv
    ab04feac8fdbb2cdc6d8596351776f5aec918f0728e27af48dd60686ed24db8e  design8_2_P_values.tsv
    038526a9b1b3c089cbef84b47438e9c742464024a60098ac642594536c7359cf  design8_2_T_values.tsv
    05715f0707f55160456a53b4c7050cbdf44ed8f1de4015366ff612bfe7c9d108  design8_2_X_values.tsv
    0081c3b300046bcc3f5cea6de7b31514cd6799b0547dbdda8b28dd5aaa4023a9  design8_2a_M_values.tsv
    55e3558d43b29bc4709dbb07f3bf421bd2e50ed858810408d85ea250ad635b29  design8_2a_P_values.tsv
    f1a210ae2d715b79836420e6a0676e46d869945610712e84d9c6b435eb05ff91  design8_2a_Q_values.tsv
    6081d0a4b07f45877551fb5e93797f659662b0915944e21470e880b42b7cc1d9  design8_2a_T_values.tsv
    0551c27c44f5fff00296667098a040618be31d867466af969c146b987466dcee  design8_2a_X_values.tsv
    010386a98d4daf269bafab91916d16dca5d7baa034d50eb76ada93b44a1061fb  design8_C_H_values.tsv
    a0f3404d02f547ea78b98074233138d92c8d7ade5b61ca3fed62d262c47c1762  design8_C_S_values.tsv
    5cffbbdafdec8eaa41ed8ccf5842c96e2475aa2979f317b26aeb2dc685c8d0d2  design8_GEN_values.tsv
    7b2da1c3e1a21df7b4f8709998f75de66a66cd13740db0c9b277cd469caf0ecf  design8_GENpp_values.tsv
    1c2871e502f1f484bafa613913b2fd8af52bfe42b36528bfb56cc16957cc36f7  design8_T1_values.tsv
    a9fd145306c84956fbbe0e143a8ffcf1da44e868cc54df2442f5dfbdeb282cde  generator8.tsv
    2ce83a138f992c5867df068108894d0f665c2ae39bfb5f92fe185bdb631e72b3  leaves8_targets.tsv
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
    a965b955f54d1a2201f25c01989f321ce74d8f9d46e199c44a736603e6fbb75a  ../cpp/leafc8b.cpp
    e742e3ba195072877b3f7e7d46fe4462733fef442fcfe2518dd96f735fd6632e  ../cpp/detmath.cpp
    f3e29549e1a676577af09fe08a401c5393c2813edd6f9f07f43efc14551f57b4  ../tools/verify_lock8b.sh
    f9f7c2379656ffbe23e97ae728966c0e23341d260229bb1170b7558db36b605d  ../tools/check_hostindep8b.sh
    852335690a5efcb02c535b57ce892aec0e9a3be770fdc3c971f067f31b709369  ../tools/run_bs8b.sh
    4e02cb3fffe361a7fcb26f7d43d2a67ea04b993991f6a864b3649bcc90146900  ../tools/batch8b.sh
