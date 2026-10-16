#!/usr/bin/env python3
"""hubpy.py (LOCK9H): Python reference for the 0-D hub bench. Integrates the hub's own right-hand sides from vendor/fsot_dynamics.py
(sha256 837817ba3b04..., directory from env FSOT_VENDOR) with classical RK4, dt = 1e-3 unit, t = 0..8:
  S mode:   scalar_transport_rhs(S, v=0, dS=0, d2S=0, D_eff, S_eq=equilibrium_scalar("Cosmology")), S(0) = 0
  Fluid:    momentum_rhs(rho=1, v, dv=0, d2v=-v (k = 1 mode), dP=0, D_eff=derived D_eff(Fluid_Dynamics), observed=False), v(0) = 1
  D = 25:   momentum_rhs(rho=1, v, 0, -v, 0, 25, observed=True), v(0) = 1
Usage: hubpy.py <out.tsv>"""
import os, sys, hashlib
VD = os.environ.get('FSOT_VENDOR', '/tmp/lean_ro/vendor'); sys.path.insert(0, VD)
h = hashlib.sha256(open(os.path.join(VD, 'fsot_dynamics.py'), 'rb').read()).hexdigest()
assert h.startswith('837817ba3b04'), 'unexpected fsot_dynamics.py ' + h
import fsot_dynamics as d, fsot_compute as fc
Seq = d.equilibrium_scalar("Cosmology"); DF = float(fc.derived_D_eff("Fluid_Dynamics"))   # = 12 (LOCK8.md)
fS = lambda s: d.scalar_transport_rhs(s, 0.0, 0.0, 0.0, DF, Seq)
fF = lambda v: d.momentum_rhs(1.0, v, 0.0, -v, 0.0, DF, False)
f25 = lambda v: d.momentum_rhs(1.0, v, 0.0, -v, 0.0, 25.0, True)
def rk(f, x, h):
    k1 = f(x); k2 = f(x + h / 2 * k1); k3 = f(x + h / 2 * k2); k4 = f(x + h * k3); return x + h / 6 * (k1 + 2 * k2 + 2 * k3 + k4)
N = 1000; h = 1.0 / N; S, vF, v25 = 0.0, 1.0, 1.0
with open(sys.argv[1], 'w') as o:
    o.write('t\tS_py\tvF_py\tv25_py\n')
    for i in range(8 * N + 1):
        if i % (N // 4) == 0: o.write('%.4f\t%.17e\t%.17e\t%.17e\n' % (i / N, S, vF, v25))
        S, vF, v25 = rk(fS, S, h), rk(fF, vF, h), rk(f25, v25, h)
print('HUBPY_OK S_eq=%.17g' % Seq)
