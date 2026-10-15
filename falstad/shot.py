import sys, json, time
from playwright.sync_api import sync_playwright
url = open(sys.argv[1]).read().strip()
with sync_playwright() as p:
    b = p.chromium.launch(channel="chrome", headless=True); pg = b.new_page(viewport={"width": 1600, "height": 900}); pg.goto(url, timeout=120000)
    pg.wait_for_function("window.CircuitJS1 !== undefined && window.CircuitJS1.getTime !== undefined", timeout=120000)
    time.sleep(float(sys.argv[3]) if len(sys.argv) > 3 else 8); pg.screenshot(path=sys.argv[2]); b.close()
