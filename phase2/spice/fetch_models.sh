#!/usr/bin/env bash
# Phase 2 vendor models (not committed: vendor terms are not a redistribution grant). SHA-256 verified; refuses a mismatch.
#  - TI OPAx187 PSpice model (zero-drift, 36 V; OPA2187 dual) — SBOMAB6
#  - TI LM139 comparator macromodel (LM139/LM339 family, open collector) — SLCJ002
set -euo pipefail; cd "$(dirname "$0")"; mkdir -p models/vendor && cd models/vendor
curl -fsSL -o sbomab6.zip https://www.ti.com/lit/zip/sbomab6
echo "50c3c188a17bca04f2760feac9c5e6350561eb12a1e25d5a2c19b08dcc2b3930  sbomab6.zip" | sha256sum -c -
mkdir -p ti_sbomab6 && unzip -o -q -j sbomab6.zip OPAx187.LIB -d ti_sbomab6
echo "a1a17afb119ffc7dbcceff289ca43af137b794f447b0e190b56a289d6cfe8f4f  ti_sbomab6/OPAx187.LIB" | sha256sum -c -
curl -fsSL -o slcj002.zip https://www.ti.com/lit/zip/slcj002
echo "337a56c5c2c6fdae500195eab7ee3353cb293f110f3466391df250b8eaf5ee0f  slcj002.zip" | sha256sum -c -
mkdir -p ti_slcj002 && unzip -o -q -j slcj002.zip LM139.5_1 -d ti_slcj002
echo "eaef5de1a4eafcea6ab1ad89f54817292fcff74e767196c64d000b08f1bca1a8  ti_slcj002/LM139.5_1" | sha256sum -c -
