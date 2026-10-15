# LOCK7 Amendment 7.1 (locked7b) — leaf LOGR currents and readout range

Locked 2026-10-15 after the LOCK7 run set (ideal + full MC, results/leaves7_*.tsv, kept as recorded) and before any Amendment 7.1 simulation.
Fork phase3/cpp/leafc7b.cpp from leafc7.cpp (unchanged). Base engine, netgen6, taps: LOCK7, unchanged (taps7.tsv and design7_taps_values.tsv here
are byte-identical copies, so netgen6 mode leafc7 reads everything from FSOT_P7_LOCKED=../../phase3/locked7b). No FSOT rule, tolerance or solver change.

## 7.1(a) Leaf signal LOGR currents
LOCK7 7.1 ran the flipped signal LOGRs at reference IIN e^+1.5 = 4.48 uA (transdiode) and signal IIN e^-1.5 = 0.22 uA (cascoded side). The constant
(ln av + 3) and the headroom (-3 V_T) depend only on the ratio, not on the levels, but the 2N3904 high-injection term (I_c1 - I_c2)/IKF grew from
1.4e-5 to 6.4e-5 neper per LOGR: the LOCK7 ideal 27 C errors came out 2-3x LOCK5's (GEN 1.37e-3 -> 4.56e-3, 2_P 5.7e-4 -> 1.35e-3, 2_X
1.3e-4 -> 2.7e-4). 7.1(a) runs the reference at IIN = 1 uA and the signal at IIN e^-3 = 49.8 nA (the LOCK5 levels): same ratio, constant and headroom.

## 7.1(b) Readout antilog of a range-sized node
LOCK7 7.5 sized the Chain_consistency_% readout GAIN for rho = 100 but its readout antilog still for the nominal value, so at 46x nominal (mcC) its
base reached +22 mV (3.8 nepers above nominal) and the block failed (2a_X: mcC 2/8, mcB 0/4, all at e_zt213x). The readout antilog now takes
av rho too (nominal base -3 - ln 100 = -7.6 nepers).

## Harness
tools/run_bs7b.sh (FSOT_P7_LOCKED=locked7b, logs bs7b_*), tools/batch7b.sh, tools/summ7b.py, tools/verify_lock7b.sh.

## SHA256 (recorded before simulation)
    279a8802962fd98cefc4cf844030c80813346bff0fe2ae1f13e9ffee0cd14832  design7_2_M_values.tsv
    7bc999dfc84d8a11133aeb8a226867c3efd265ff513745fd4a926ab618aaaf00  design7_2_P_values.tsv
    038526a9b1b3c089cbef84b47438e9c742464024a60098ac642594536c7359cf  design7_2_T_values.tsv
    1a306d4e93efed40b92e77a8ab061e577a6ab273df0d5036b677cbce787fef70  design7_2_X_values.tsv
    0081c3b300046bcc3f5cea6de7b31514cd6799b0547dbdda8b28dd5aaa4023a9  design7_2a_M_values.tsv
    85605ac97e0d1d0e80fbee8d1134413841dacc771e3b5a836f8d635a1c717dd4  design7_2a_P_values.tsv
    f9227814333ac11d001ca939b573a63511d17ccbe77e18900209f2ea4cce7c5b  design7_2a_Q_values.tsv
    6081d0a4b07f45877551fb5e93797f659662b0915944e21470e880b42b7cc1d9  design7_2a_T_values.tsv
    fe30c1bfcd6dade67fe834920bfb90e0a9048d3c00ad1ab4b666cfc0a51ff75a  design7_2a_X_values.tsv
    7d54cff43c213415fef11a351f304002477387744f17320f17f6f7d6f09f7dc1  design7_GEN_values.tsv
    7952ecdac1daa1147da8ea9915a28f074f8f24b7efeee0e7690e2a0929db6cfc  design7_GENpp_values.tsv
    17cf77d9aaccc5296e8b9d010c012ab243f592c6dcae179c276015dfe387f8bd  design7_T1_values.tsv
    471c79ca390528ae1feedb4290fd7b2e89a4d37335866921b049f11c0625e883  design7_taps_values.tsv
    a39e58113dfbc352016a8d8c8ee29caf7b3afa4e65c3c309371dee12d779ee8c  generator7.tsv
    aaf5e1226a929610c3da0feff71d381078c771e604ed898965d62e7077cd8d6c  leaves7_targets.tsv
    6b4ced7632fba3fb28b296c7495b0f139cdac05aa596e765d309dba7faa8afcb  netlists7.tsv
    3b97a95adfc6f7d37c6c3df53b5401fa88e04246c3af53ced43e5c39dce310f9  recipe7_2_M.txt
    045a5d2dec938eb463eb484c9ae803796113385157867ee83b05388a94ab53e7  recipe7_2_P.txt
    da0dc74b098a1673903aa6e0b1d4f65d7cae97c93882eb6f2b922d6c30dc216f  recipe7_2_T.txt
    e410132fed960df8332cd50680e42bcf2aeea009217d87c03d69853335bf2be7  recipe7_2_X.txt
    cbd822cc5785ee1ae8f0125952939690c15a250086f1330dafd1d9c6ac012bd6  recipe7_2a_M.txt
    c873df626af770bf8cb5288bedb410e9b04ea8d26faaf51705791f262c84dc61  recipe7_2a_P.txt
    717ad6da3234ea5f6e12b12a9d182a6670592aece10e1e8e94717cbf4262e91c  recipe7_2a_Q.txt
    b3f2fba06f09685d6ebe7d77315e80a21576f6bc24af61effba90bdd29540f2d  recipe7_2a_T.txt
    ae560ff0801ed08a4f2ec2dc891d313564ace2b68c7161b17624621e7dc29d42  recipe7_2a_X.txt
    243cd7cee1a48068706464137bf5ea7901beb756d12cc89628c2f66e775974b6  recipe7_GEN.txt
    94358406ebc1542bbb2366bd385689f8506ff16389af687ef857ee227dd4ecbd  recipe7_GENpp.txt
    4d37145044a086b8d41d935a5bae6ab2f589d55b3b31d354db1f19e493a3a884  recipe7_T1.txt
    0bafb0b56742cd097598c9914c60403331c4e7d354dbf139a6bfe78f215a4e8f  taps7.tsv
    74970598deeb5daed7d1b09ac140ea73897b964aa082eb967947560d419afe6e  ../cpp/leafc7b.cpp
    15e1723af9426a0c9d6bd2566f5807f6ce1e7aa14ecb9704bc5522c4ce59f9cb  ../tools/run_bs7b.sh
    68c589ec916045a0dfa1542987419eb0d3809203a71a2ca1999cf31f35142daa  ../tools/verify_lock7b.sh
