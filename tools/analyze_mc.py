"""Analysis only (no value is designed here): MC statistics, fold mis-round counts, 33-domain S check, plots."""
import os as _os
ROOT = _os.path.dirname(_os.path.dirname(_os.path.abspath(__file__)))
import csv, math, statistics as st, os, collections
import matplotlib; matplotlib.use("Agg"); import matplotlib.pyplot as plt
R = ROOT
T = {}
for l in open(f"{R}/locked/fsot_targets.tsv"):
    if l[0] in "#n" or not l.strip(): continue
    n, v, _ = l.split("\t", 2); T[n] = float(v)
Q = ["S_quant", "S_chem", "S_cosm", "f_D5", "f_D6", "f_D25", "m_H/m_W", "Omega_b_h2", "alpha_s(M_Z)", "1/alpha_em"]
def load(p): return [r for r in csv.DictReader(open(p), delimiter="\t") if r["log"] != "FSOT"]
nom = {}
for tag, p in [("e24_tl082", "out/slice_e24_tl082_s0_t27.log"), ("e24_idealop", "out/slice_e24_tl082_s0_t27_idealop.log"), ("ideal_ideal", "out/slice_ideal_ideal_s0_t27.log"), ("ideal_tl082", "out/slice_ideal_tl082_s0_t27.log")]:
    import subprocess; o = subprocess.run(["../cpp/score", p], cwd=f"{R}/spice", capture_output=True, text=True).stdout.splitlines()
    h = o[0].split("\t"); v = o[1].split("\t"); nom[tag] = dict(zip(h, v))
out = open(f"{R}/results/mc_summary.tsv", "w")
out.write("variant\tquantity\tFSOT\tN\tmean\tsd\trel_sd\tmin\tmax\tp2.5\tp97.5\n")
mc = {}
for v in ["mcA", "mcB", "mcC"]:
    rows = load(f"{R}/results/mc_{v}_idealop.tsv"); mc[v] = rows
    for q in Q:
        x = sorted(float(r[q]) for r in rows if r[q] != "nan"); m = st.mean(x); s = st.stdev(x)
        out.write(f"{v}\t{q}\t{T[q]:.12g}\t{len(x)}\t{m:.9g}\t{s:.4g}\t{abs(s/T[q]):.3g}\t{x[0]:.9g}\t{x[-1]:.9g}\t{x[int(.025*len(x))]:.9g}\t{x[int(.975*len(x))-1]:.9g}\n")
out.close()
# paired check: converged TL082 mcA seeds vs same-seed idealop
tl = {r["log"].split("_s")[1].split("_")[0]: r for r in load(f"{R}/results/mc_mcA_tl082_converged.tsv") if r["S_quant"] != "nan" and "SOLVER: FAILED" not in open(f"{R}/spice/" + r["log"]).read()}
il = {r["log"].split("_s")[1].split("_")[0]: r for r in mc["mcA"]}
pair = open(f"{R}/results/mc_mcA_paired_tl082_vs_idealop.tsv", "w"); pair.write("# converged TL082 mcA draws (SOLVER nodeset/plain) vs the same draw solved with 1e6-gain op-amps; rel diff = (TL082-idealop)/FSOT\nquantity\tN_pairs\tmean_rel_diff\tmax_abs_rel_diff\n")
for q in Q:
    d = [(float(tl[s][q]) - float(il[s][q])) / T[q] for s in tl if s in il]
    pair.write(f"{q}\t{len(d)}\t{st.mean(d):.3g}\t{max(abs(x) for x in d):.3g}\n")
pair.close()
# folds
Dg = {g: round(5 * 5 ** (g / 19)) for g in range(20)}
folds = list(csv.DictReader(open(f"{R}/locked/folds.tsv"), delimiter="\t"))
for f in folds: assert Dg[int(f["gen"])] == int(f["D_eff"]), f
fo = open(f"{R}/results/fold_mc_summary.tsv", "w"); fo.write("variant\tg\tD_locked\tN\tmisrounds\tD_mean\tD_sd\n")
fmis = {}
for v in ["mcA", "mcB", "mcC"]:
    by = collections.defaultdict(list)
    for l in open(f"{R}/results/fold_g_{v}.txt"):
        p = l.split(); by[int(p[2])].append(25 * float(p[3]) / float(p[4]))
    tot = 0
    for g in range(20):
        x = by[g]; mis = sum(abs(d - Dg[g]) > 0.5 for d in x); tot += mis
        fo.write(f"{v}\t{g}\t{Dg[g]}\t{len(x)}\t{mis}\t{st.mean(x):.5f}\t{st.stdev(x):.4f}\n")
    fmis[v] = (tot, sum(len(by[g]) for g in range(20)))
fo.close()
# 33-domain nominal check (TL082 + E24, physical g-string)
nomg = {}
for l in open(f"{R}/results/fold_g_e24_tl082.txt"):
    p = l.split(); nomg[int(p[2])] = (25 * float(p[3]) / float(p[4]), float(p[6]) / 2, float(p[7]) / 2)
d33 = open(f"{R}/results/fold33_nominal.tsv", "w"); d33.write("domain\tgen\tD_eff\tobserved\tS_engine\tD_circuit\tS_circuit\trel_err\n")
errs = []
for f in folds:
    if abs(float(f["look"]) - 1) > 1e-12 or int(f["hits"]) != 0: continue
    g = int(f["gen"]); D, so, sd = nomg[g]; s = so if f["observed"] == "1" else sd; se = float(f["S_engine"]); e = (s - se) / abs(se); errs.append(abs(e))
    d33.write(f"{f['domain']}\t{g}\t{f['D_eff']}\t{f['observed']}\t{se:.9f}\t{D:.4f}\t{s:.7f}\t{e:.3g}\n")
d33.close()
print("33-domain: n", len(errs), "max rel err", max(errs), "median", st.median(errs))
print("fold misrounds", fmis)
# plots
fig, ax = plt.subplots(2, 5, figsize=(20, 7))
for a, q in zip(ax.flat, Q):
    for v, c in [("mcA", "tab:red"), ("mcB", "tab:blue"), ("mcC", "tab:green")]:
        a.hist([float(r[q]) for r in mc[v]], bins=40, alpha=.5, color=c, label=v, density=True)
    a.axvline(T[q], color="k", lw=1.5, label="C++ engine"); a.set_title(q); a.ticklabel_format(useOffset=False)
ax[0, 0].legend(fontsize=8); fig.suptitle("FSOT analog slice, 200-draw Monte Carlo per tolerance class (27 C)"); fig.tight_layout(); fig.savefig(f"{R}/png/mc_histograms.png", dpi=110)
fig, ax = plt.subplots(1, 2, figsize=(13, 4.5))
sw = [l.split() for l in open(f"{R}/spice/out/fold_sweep_e24_tl082.cir.dat") if l.strip() and l.split()[0][0] in "-0123456789"]
try:
    gs = [float(r[0]) / 0.125 for r in sw]; Dd = [25 * float(r[3]) / float(r[5]) for r in sw]
    ax[0].plot(gs, Dd, lw=1, label="ngspice TL082+E24 ladder"); 
except Exception as ex: print("sweep parse", ex)
ax[0].step(range(20), [Dg[g] for g in range(20)], where="post", color="k", ls="--", lw=.8, label="round(5*5^(g/19)) (engine)")
ax[0].set_xlabel("generation g (0.125 V/gen)"); ax[0].set_ylabel("D_eff = 25 I_D/I_25"); ax[0].legend(); ax[0].set_title("Comparator-ladder quantizer")
ax[1].semilogy([float(f["gen"]) for f in folds if abs(float(f["look"]) - 1) < 1e-12 and int(f["hits"]) == 0], errs, "o"); ax[1].set_xlabel("generation"); ax[1].set_ylabel("|S_circuit-S_engine|/|S|"); ax[1].set_title("33 look=1 folds, nominal TL082+E24")
fig.tight_layout(); fig.savefig(f"{R}/png/fold_staircase.png", dpi=110)
