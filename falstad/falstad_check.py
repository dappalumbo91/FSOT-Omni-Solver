"""Run each Falstad link headless (system Chrome, Playwright) and read the settled labelled-node voltages."""
import sys, json, time, os
from playwright.sync_api import sync_playwright
HERE = os.path.dirname(os.path.abspath(__file__)); urls = json.load(open(os.path.join(HERE, "urls.json"))); res = {}
with sync_playwright() as p:
    b = p.chromium.launch(channel="chrome", headless=True)
    for name, d in urls.items():
        pg = b.new_page(); pg.goto(d["url"], timeout=120000)
        pg.wait_for_function("window.CircuitJS1 !== undefined && window.CircuitJS1.getTime !== undefined", timeout=120000)
        t0 = time.time(); trace = []
        while time.time() - t0 < (float(sys.argv[1]) if len(sys.argv) > 1 else 20):
            time.sleep(2); c = "window.CircuitJS1"
            trace.append(pg.evaluate(f"[{c}.getTime(), {c}.getNodeVoltage('vS'), {c}.getNodeVoltage('vf'), {c}.getNodeVoltage('u')]"))
        res[name] = {"expected_vS": d["expected_vS"], "final": trace[-1], "trace": trace}
        print(name, json.dumps(trace[-1]), "expected vS", d["expected_vS"], flush=True); pg.close()
    b.close()
json.dump(res, open(os.path.join(HERE, "falstad_check.json"), "w"), indent=1)
