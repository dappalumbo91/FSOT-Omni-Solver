#!/usr/bin/env bash
# Fetch the TI TL082 PSpice macro-model (SLOJ070; used as the TL07x/TL08x JFET proxy). Not committed (vendor licence); SHA-256 verified.
set -euo pipefail; cd "$(dirname "$0")"; mkdir -p models/vendor && cd models/vendor
curl -fsSL -o sloj070.zip https://www.ti.com/lit/zip/sloj070
echo "4f27e17a7ceabebf61edddca7b7e187830c1924af417c40811255a8f3a879937  sloj070.zip" | sha256sum -c -
mkdir -p ti_sloj070 && unzip -o -q sloj070.zip -d ti_sloj070
echo "adad2e131f728610963e0797670ea34d2857c0b26b3f49c6ce356a3213d89011  ti_sloj070/TL082.301" | sha256sum -c -
