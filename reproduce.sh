#!/usr/bin/env bash
# One-command reproduction of Phase 1. Usage: ./reproduce.sh [--mc]   (--mc also reruns the 600-draw Monte Carlo + fold MC, ~10 min on 8 cores)
# Needs: g++ (C++20), ngspice (>= 42), curl, unzip, git, python3 (+ mpmath, matplotlib for the cross-check/plots).
set -euo pipefail
cd "$(dirname "$0")"; ROOT=$PWD
PIN=3466e8d4211d786fa7a430de1ee75131fb3b2e97   # FSOT-2.1-Cpp authority commit (engine pin AEB2AD)
FSOT_CPP_DIR=${FSOT_CPP_DIR:-$ROOT/third_party/FSOT-2.1-Cpp}
if [ ! -d "$FSOT_CPP_DIR/include/fsot" ]; then
  git clone -q https://github.com/dappalumbo91/FSOT-2.1-Cpp.git "$FSOT_CPP_DIR"; git -C "$FSOT_CPP_DIR" checkout -q $PIN
fi
[ "$(git -C "$FSOT_CPP_DIR" rev-parse HEAD)" = "$PIN" ] || { echo "FSOT-2.1-Cpp is not at the pinned commit $PIN"; exit 1; }
export FSOT_CPP_DIR
echo "== 1. build C++20 derivation / netlist generator / scorer"
CXX=${CXX:-g++}; F="-std=c++20 -O2 -ffp-contract=off"
$CXX $F -I"$FSOT_CPP_DIR/include" -o cpp/derive cpp/derive.cpp
$CXX $F -o cpp/netgen cpp/netgen.cpp
$CXX $F -o cpp/score cpp/score.cpp
echo "== 2. LOCK: hashes + byte-identical re-derivation of every locked value"
tools/verify_lock.sh
echo "== 3. vendor models (not committed; sha256-checked)"
[ -f spice/models/vendor/ti_sloj070/TL082.301 ] || spice/fetch_models.sh
echo "== 4. ngspice nominal slice (ideal/ideal, ideal/TL082, E24/TL082 at 0, 27, 50 C) + fold staircase"
cd spice; mkdir -p netlists out
./run1.sh ideal_ideal 0 27; ./run1.sh ideal_tl082 0 27; for t in 27 0 50; do ./run1.sh e24_tl082 0 $t; done
../cpp/netgen slice e24_tl082 0 netlists/slice_e24_tl082_s0_t27_idealop.cir 27 idealop && ngspice -b netlists/slice_e24_tl082_s0_t27_idealop.cir > out/slice_e24_tl082_s0_t27_idealop.log 2>&1
for v in ideal_ideal e24_tl082; do ../cpp/netgen foldsteps $v 0 netlists/fold_steps_$v.cir 27 && ngspice -b netlists/fold_steps_$v.cir > out/fold_steps_$v.log 2>&1; done
../cpp/netgen foldsweep e24_tl082 0 netlists/fold_sweep_e24_tl082.cir 27 && ngspice -b netlists/fold_sweep_e24_tl082.cir > out/fold_sweep_e24_tl082.log 2>&1
./fold_mc.sh e24_tl082 0 > ../results/fold_g_e24_tl082.txt
../cpp/score out/slice_ideal_ideal_s0_t27.log out/slice_ideal_tl082_s0_t27.log out/slice_e24_tl082_s0_t27.log out/slice_e24_tl082_s0_t0.log out/slice_e24_tl082_s0_t50.log > ../results/regen_nominal_temp.tsv
cd "$ROOT"
python3 tools/compare_nominal.py results/nominal_temp.tsv results/regen_nominal_temp.tsv
if [ "${1:-}" = "--mc" ]; then
  echo "== 5. Monte Carlo (classes A/B/C, 200 draws each) + fold MC"
  spice/mc_idealop.sh
  cd spice; for v in mcA mcB mcC; do ../cpp/score out/slice_${v}_s*_t27_idealop.log | grep -v '^FSOT' > ../results/mc_${v}_idealop.tsv; done; cd "$ROOT"
  python3 tools/analyze_mc.py && python3 tools/table.py > results/results_table.md && python3 tools/error_budget.py
fi
echo "== 6. independent Python (mpmath) cross-check of the C++ targets"
python3 tools/crosscheck_mpmath.py | tee results/crosscheck_mpmath.txt
echo "REPRODUCE_OK"
