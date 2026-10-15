# Third-party files (not committed)

Vendor SPICE models and papers are **not** in this repository: vendor terms are not an explicit redistribution grant.
Each is referenced by source, link, download steps and SHA-256, and fetched by a script that refuses a mismatching file.

| file | source | link | SHA-256 | fetched by |
|---|---|---|---|---|
| `sloj070.zip` (TI TL082 PSpice macromodel archive) | Texas Instruments, TL082 product page → Design & development → PSpice model | https://www.ti.com/lit/zip/sloj070 | `4f27e17a7ceabebf61edddca7b7e187830c1924af417c40811255a8f3a879937` | `spice/fetch_models.sh` |
| `ti_sloj070/TL082.301` (extracted) | same archive | — | `adad2e131f728610963e0797670ea34d2857c0b26b3f49c6ce356a3213d89011` | `spice/fetch_models.sh` |
| `sbomab6.zip` (TI OPAx187 PSpice model, Phase 2) | Texas Instruments, OPA187 product page → Design & development → PSpice model | https://www.ti.com/lit/zip/sbomab6 | `50c3c188a17bca04f2760feac9c5e6350561eb12a1e25d5a2c19b08dcc2b3930` | `phase2/spice/fetch_models.sh` |
| `ti_sbomab6/OPAx187.LIB` (extracted) | same archive | — | `a1a17afb119ffc7dbcceff289ca43af137b794f447b0e190b56a289d6cfe8f4f` | `phase2/spice/fetch_models.sh` |
| `slcj002.zip` (TI LM139 comparator macromodel, Phase 2) | Texas Instruments, LM139 product page → PSpice model | https://www.ti.com/lit/zip/slcj002 | `337a56c5c2c6fdae500195eab7ee3353cb293f110f3466391df250b8eaf5ee0f` | `phase2/spice/fetch_models.sh` |
| `ti_slcj002/LM139.5_1` (extracted) | same archive | — | `eaef5de1a4eafcea6ab1ad89f54817292fcff74e767196c64d000b08f1bca1a8` | `phase2/spice/fetch_models.sh` |
| FSOT-2.1-Cpp engine headers | github.com/dappalumbo91/FSOT-2.1-Cpp, commit `3466e8d4211d786fa7a430de1ee75131fb3b2e97` | https://github.com/dappalumbo91/FSOT-2.1-Cpp/tree/3466e8d4211d786fa7a430de1ee75131fb3b2e97 | git commit pin | `reproduce.sh` (git clone + checkout, HEAD verified) |

```bash
spice/fetch_models.sh     # downloads, verifies SHA-256, unzips into spice/models/vendor/ (git-ignored)
```

Inline models written in this repo (free to commit): 2N3904 (Fairchild/onsemi published Gummel–Poon parameters, typed into
`cpp/netgen.cpp`), 1N4148 diode parameters, LM339-class behavioural comparator.

Phase 2 inline models (free to commit): 2N7000 as an ngspice VDMOS card (published onsemi-style parameters typed into
`phase2/cpp/netgen2.cpp`), 2N3904 Gummel–Poon (same card as Phase 1), CD4053 poles as SW elements with R_on 75–175 Ω.

```bash
phase2/spice/fetch_models.sh   # OPAx187 + LM139, SHA-256 verified, into phase2/spice/models/vendor/ (git-ignored)
```
