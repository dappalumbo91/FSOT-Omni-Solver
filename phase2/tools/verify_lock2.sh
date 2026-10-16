#!/usr/bin/env bash
# Phase 2 LOCK check: (1) locked/SHA256SUMS matches, (2) every listed hash is recorded in locked/LOCK.md,
# (3) re-deriving with the pinned engine (cpp/derive2, built against FSOT-2.1-Cpp 3466e8d) reproduces the locked TSVs byte for byte.
set -euo pipefail; cd "$(dirname "$0")/../locked"
sha256sum -c SHA256SUMS
while read -r h f; do grep -q "$h" LOCK.md || { echo "hash of $f not recorded in LOCK.md"; exit 1; }; done < SHA256SUMS
tmp=$(mktemp -d); (cd ../cpp && ./derive2 "$tmp" | tail -1)
for f in design2_values.tsv design2_consts.tsv folds35_targets.tsv targets2.tsv domain_margins.tsv; do cmp "$tmp/$f" "$f"; done
rm -rf "$tmp"; echo "LOCK2_OK"
