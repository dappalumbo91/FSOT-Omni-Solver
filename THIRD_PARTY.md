# Third-party files (not committed)

Vendor SPICE models and papers are **not** in this repository: vendor terms are not an explicit redistribution grant.
Each is referenced by source, link, download steps and SHA-256, and fetched by a script that refuses a mismatching file.

| file | source | link | SHA-256 | fetched by |
|---|---|---|---|---|
| `sloj070.zip` (TI TL082 PSpice macromodel archive) | Texas Instruments, TL082 product page → Design & development → PSpice model | https://www.ti.com/lit/zip/sloj070 | `4f27e17a7ceabebf61edddca7b7e187830c1924af417c40811255a8f3a879937` | `spice/fetch_models.sh` |
| `ti_sloj070/TL082.301` (extracted) | same archive | — | `adad2e131f728610963e0797670ea34d2857c0b26b3f49c6ce356a3213d89011` | `spice/fetch_models.sh` |
| FSOT-2.1-Cpp engine headers | github.com/dappalumbo91/FSOT-2.1-Cpp, commit `3466e8d4211d786fa7a430de1ee75131fb3b2e97` | https://github.com/dappalumbo91/FSOT-2.1-Cpp/tree/3466e8d4211d786fa7a430de1ee75131fb3b2e97 | git commit pin | `reproduce.sh` (git clone + checkout, HEAD verified) |

```bash
spice/fetch_models.sh     # downloads, verifies SHA-256, unzips into spice/models/vendor/ (git-ignored)
```

Inline models written in this repo (free to commit): 2N3904 (Fairchild/onsemi published Gummel–Poon parameters, typed into
`cpp/netgen.cpp`), 1N4148 diode parameters, LM339-class behavioural comparator.
