#!/usr/bin/env bash
# sens.sh <nseeds>: per-part error coefficients. One draw class at a time (netgen2 variant mcX, FSOT_P2_TOL), E24 values, ideal
# op-amps, block-sequential solve; sd of the relative output error over the seeds, divided by the draw magnitude.
set -e; cd "$(dirname "$0")"; n=$1; out=../results/sensitivities.tsv
printf "draw\tmagnitude\tunit\tsd_rel_S_Atomic\tsd_rel_S_Fluid\tsd_rel_Omega_b_h2\tsd_rel_1_minus_Schem\tcoef_S_Atomic_per_unit\tcoef_Omega_per_unit\n" > $out
base() { ../cpp/score2 fold Atomic_Physics 27 out/bs_fold_e24_ideal_s0_t27_Atomic_Physics.log | cut -f5; }
./run_bs.sh fold e24_ideal 0 27 Atomic_Physics; ./run_bs.sh fold e24_ideal 0 27 Fluid_Dynamics; ./run_bs.sh omb e24_ideal 0 27
bA=$(../cpp/score2 fold Atomic_Physics 27 out/bs_fold_e24_ideal_s0_t27_Atomic_Physics.log | cut -f5); bF=$(../cpp/score2 fold Fluid_Dynamics 27 out/bs_fold_e24_ideal_s0_t27_Fluid_Dynamics.log | cut -f5)
bo=$(../cpp/score2 omb 27 out/bs_omb_e24_ideal_s0_t27.log); bO=$(echo "$bo" | cut -f9); b1=$(echo "$bo" | cut -f7)
for d in "tolR 0.001 fraction(uniform)" "tc 25e-6 per_K(uniform,at_27C=0)" "vos 10e-6 V(sigma)" "vbe 1e-4 V(sigma)" "vref 0.001 fraction(uniform)" "beta 0.3 fraction(uniform)" "vaf 0.2 fraction(uniform)" "ron 50 ohm(uniform)"; do
  set -- $d; k=$1; m=$2; u=$3
  for s in $(seq 1 $n); do echo $s; done | FSOT_P2_TOL="$k=$m" xargs -P 8 -I{} sh -c "FSOT_P2_TOL=$k=$m ./run_bs.sh fold mcX {} 27 Atomic_Physics; FSOT_P2_TOL=$k=$m ./run_bs.sh fold mcX {} 27 Fluid_Dynamics; FSOT_P2_TOL=$k=$m ./run_bs.sh omb mcX {} 27"
  sA=$(for s in $(seq 1 $n); do ../cpp/score2 fold Atomic_Physics 27 out/bs_fold_mcX_s${s}_t27_Atomic_Physics.log | cut -f5; done | awk -v b=$bA '$1!="nan"{d=$1-b; s+=d*d; c++} END{printf "%.3e", sqrt(s/c)}')
  sF=$(for s in $(seq 1 $n); do ../cpp/score2 fold Fluid_Dynamics 27 out/bs_fold_mcX_s${s}_t27_Fluid_Dynamics.log | cut -f5; done | awk -v b=$bF '$1!="nan"{d=$1-b; s+=d*d; c++} END{printf "%.3e", sqrt(s/c)}')
  sO=$(for s in $(seq 1 $n); do ../cpp/score2 omb 27 out/bs_omb_mcX_s${s}_t27.log | cut -f9; done | awk -v b=$bO '$1!="nan"{d=$1-b; s+=d*d; c++} END{printf "%.3e", sqrt(s/c)}')
  s1=$(for s in $(seq 1 $n); do ../cpp/score2 omb 27 out/bs_omb_mcX_s${s}_t27.log | cut -f7; done | awk -v b=$b1 '$1!="nan"{d=$1-b; s+=d*d; c++} END{printf "%.3e", sqrt(s/c)}')
  printf "%s\t%s\t%s\t%s\t%s\t%s\t%s\t%.3g\t%.3g\n" $k $m $u $sA $sF $sO $s1 $(awk -v a=$sA -v m=$m 'BEGIN{print a/m}') $(awk -v a=$sO -v m=$m 'BEGIN{print a/m}') >> $out
done
cat $out
