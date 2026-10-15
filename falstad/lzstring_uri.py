"""Pure-Python port of LZString.compressToEncodedURIComponent / decompressFromEncodedURIComponent
(pieroxy lz-string 1.4.x, MIT), the codec CircuitJS uses for ?ctz=. Verified against Falstad's own
war/lz-string.min.js by verify_falstad.py."""
KEY = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+-$"

def _compress(s, bits, char):
    if s is None: return ""
    dictionary, to_create = {}, {}
    w = ""; enlarge = 2; dict_size = 3; num_bits = 2
    out = []; val = 0; pos = 0
    def emit(value, nbits):
        nonlocal val, pos
        for _ in range(nbits):
            val = (val << 1) | (value & 1)
            if pos == bits - 1:
                pos = 0; out.append(char(val)); val = 0
            else:
                pos += 1
            value >>= 1
    def emit_w():
        nonlocal enlarge, num_bits
        if w in to_create:
            c = ord(w[0])
            if c < 256:
                emit(0, num_bits); emit(c, 8)
            else:
                emit(1, num_bits); emit(c, 16)
            enlarge -= 1
            if enlarge == 0: enlarge = 2 ** num_bits; num_bits += 1
            del to_create[w]
        else:
            emit(dictionary[w], num_bits)
        enlarge -= 1
        if enlarge == 0: enlarge = 2 ** num_bits; num_bits += 1
    # operate on UTF-16 code units like JS
    units = s.encode("utf-16-le")
    chars = [chr(units[i] | (units[i + 1] << 8)) for i in range(0, len(units), 2)]
    for c in chars:
        if c not in dictionary:
            dictionary[c] = dict_size; dict_size += 1; to_create[c] = True
        wc = w + c
        if wc in dictionary:
            w = wc
        else:
            emit_w()
            dictionary[wc] = dict_size; dict_size += 1
            w = c
    if w != "":
        emit_w()
    emit(2, num_bits)
    while True:
        val <<= 1
        if pos == bits - 1:
            out.append(char(val)); break
        pos += 1
    return "".join(out)

def compressToEncodedURIComponent(s):
    return _compress(s, 6, lambda a: KEY[a])
