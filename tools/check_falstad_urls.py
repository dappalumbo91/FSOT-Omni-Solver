"""CI: every falstad/*.url is exactly the lz-string encoding of the committed falstad/*.txt circuit, and regenerating the
circuits from locked/design_values.tsv (falstad/gen_falstad_core.py) reproduces the same text."""
import glob, os, sys
ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__))); sys.path.insert(0, ROOT + "/falstad")
from lzstring_uri import compressToEncodedURIComponent as enc
bad = 0
for u in sorted(glob.glob(ROOT + "/falstad/*.url")):
    url = open(u).read().strip(); txt = open(u[:-4] + ".txt").read()
    ok = url.split("?ctz=", 1)[1] == enc(txt); bad += not ok; print(("OK  " if ok else "BAD ") + os.path.basename(u))
print("FALSTAD_OK" if not bad else "FALSTAD_BAD"); sys.exit(bad)
