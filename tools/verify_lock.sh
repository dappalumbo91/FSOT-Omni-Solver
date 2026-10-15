#!/usr/bin/env bash
# Verify the LOCK: (1) every file in locked/SHA256SUMS matches, (2) every listed hash is recorded in locked/LOCK.md,
# (3) re-deriving with the pinned engine reproduces the locked TSVs byte for byte. Needs cpp/derive built.
set -euo pipefail; cd "$(dirname "$0")/.."
sha256sum -c locked/SHA256SUMS
while read -r h f; do grep -q "$h" locked/LOCK.md || { echo "hash of $f not recorded in LOCK.md"; exit 1; }; done < locked/SHA256SUMS
tmp=$(mktemp -d); cpp/derive "$tmp" > /dev/null
for f in design_values.tsv fsot_targets.tsv folds.tsv ladder_thresholds.tsv; do cmp "$tmp/$f" "locked/$f"; done
rm -rf "$tmp"; echo "LOCK_OK"
