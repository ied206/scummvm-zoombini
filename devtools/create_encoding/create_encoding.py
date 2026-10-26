#!/usr/bin/env python3
# encoding: utf-8

import struct
from hanzi_charmap import simplified_charmap, traditional_charmap

def merge_ranges(*ranges):
    # Use a set for efficient lookup
    result = set()
    for rng in ranges:
        result.update(rng)
    return result

def processtable(inputfilename, outfile, highrange, lowrange):
    with open(inputfilename) as f:
        res = dict(((x << 8) | y, 0) for x in highrange for y in lowrange)

        for line in f:
            if line.startswith('#'):
                continue
            splits = line.split()
            if len(splits[0]) <= 5:
                continue
            key = int(splits[0], 16)
            val = int(splits[1], 16)
            high = (key >> 8) & 0xff
            low = key & 0xff
            if high not in highrange or low not in lowrange:
                continue
            res[key] = val

    for (k, v) in sorted(res.items()):
        outfile.write(struct.pack("<H", v))

def process_hanzi_t2s(outfile):
    l = min(len(traditional_charmap), len(simplified_charmap))
    outfile.write(struct.pack("<i", l))
    for i in range(l):
        outfile.write(struct.pack("<HH", ord(traditional_charmap[i]), ord(simplified_charmap[i])))
    
def process_mac_table(inputfilename, highrange, lowrange, private_use_mapping=None):
    # Apple tables also map single-byte characters and Unicode sequences.
    # Follow ICU's base/extension split: keep exact 1:N mappings separate from the 1:1 table.
    with open(inputfilename, encoding="ascii") as f:
        mappings = {}

        for line in f:
            if line.startswith('#'):
                continue
            splits = line.split()
            if len(splits) < 2:
                continue
            key = int(splits[0], 16)
            val = []
            for point in splits[1].split('+'):
                val.append(int(point, 16))
            assert 0 <= key < 0x10000 and key not in mappings
            mappings[key] = tuple(val)
    for code in [*range(0x20), 0x7f]:
        mappings.setdefault(code, (code,))
    if private_use_mapping:
        encoded_highrange, encoded_lowrange, pua_start = private_use_mapping
        offset = 0
        for high in sorted(encoded_highrange):
            for low in sorted(encoded_lowrange):
                code = (high << 8) | low
                assert code not in mappings
                mappings[code] = (pua_start + offset,)
                offset += 1

    for value in mappings.values():
        assert 1 <= len(value) <= 5
        for point in value:
            assert 0 <= point < 0x10000 and not 0xd800 <= point <= 0xdfff

    # Like ICU, reject duplicate roundtrip Unicode inputs instead of choosing an encoded form by order.
    if len(set(mappings.values())) < len(mappings):
        raise ValueError(f"{inputfilename}: duplicate roundtrip Unicode mapping")

    # We reserve five Unicode slots for each sequence record.
    # The observed longest map is MacJapanese 0x85AB mapping to 0xF862+0x0058+0x0049+0x0049+0x0049.
    # Store multi-code-point mappings into separate tables.
    unicode_sequences = []
    for code, value in mappings.items():
        if 1 < len(value):
            unicode_sequences.append((value, code))
    unicode_sequences.sort()
    sequence_indices = {}
    for index, (_, code) in enumerate(unicode_sequences):
        sequence_indices[code] = index
    codes = list(range(256))
    for high in sorted(highrange):
        for low in sorted(lowrange):
            codes.append((high << 8) | low)
    values = []
    for code in codes:
        value = mappings.get(code, (0,))
        if len(value) == 1:
            values.append(value[0])
        else:
            # Mappings absent from the primary table are looked up in the sequence table.
            values.append(0)

    # A uint32 count precedes records containing the encoded code and five zero-padded Unicode values, all LE.
    sequence_data = bytearray(struct.pack("<I", len(unicode_sequences)))
    for value, code in unicode_sequences:
        sequence_data.extend(struct.pack("<6H", code, *value, *([0] * (5 - len(value)))))
    # Record indices sorted by encoded character allow binary search when decoding.
    for code, index in sorted(sequence_indices.items()):
        sequence_data.extend(struct.pack("<H", index))
    return struct.pack(f"<{len(values)}H", *values), sequence_data

encdat = open("encoding.dat", "wb")
encdat.write(b'SCVMENCD')
# version
encdat.write(struct.pack("<i", 0))

HIGH = 'high'
LOW = 'low'
FILE = 'file'
MAC_FILE = 'mac_file'
MAC_SEQUENCES = 'mac_sequences'
PUA = 'pua'

# Table order must match loadCJKTables().
tables = [
    {
        FILE: "CP932.TXT",
        HIGH: merge_ranges(range(0x81, 0x85), range(0x87, 0xa0), range(0xe0, 0xef), range(0xfa, 0xfd)),
        LOW: range(0x40, 0x100)
    },
    {
        FILE: "CP949.TXT",
        HIGH: range(0x81, 0xFF),
        LOW: merge_ranges(range(0x41, 0x5b), range(0x61, 0x7b), range(0x81, 0xFF))
    },
    {
        FILE: "CP950.TXT",
        HIGH: range(0xA1, 0xFA),
        LOW: merge_ranges(range(0x40, 0x7f), range(0xa1, 0xff))
    },
    {
        FILE: "JOHAB.TXT",
        HIGH: range(0x84, 0xD4),
        LOW: merge_ranges(range(0x41, 0x7f), range(0x81, 0xff))
    },
    {
        FILE: "CP936.TXT",
        HIGH: range(0x81, 0xFF),
        LOW: merge_ranges(range(0x40, 0x7f), range(0x80, 0xff))
    },
    {
        FILE: "hanzi_charmap.py"
    },
    {
        # https://www.unicode.org/Public/MAPPINGS/VENDORS/APPLE/KOREAN.TXT
        MAC_FILE: "MACKOREAN.TXT",
        HIGH: range(0xa1, 0xff),
        LOW: range(0x41, 0xff)
    },
    {
        MAC_SEQUENCES: "MACKOREAN.TXT"
    },
    {
        # https://www.unicode.org/Public/MAPPINGS/VENDORS/APPLE/JAPANESE.TXT
        MAC_FILE: "MACJAPANESE.TXT",
        HIGH: merge_ranges(range(0x81, 0xa0), range(0xe0, 0xfd)),
        LOW: range(0x40, 0x100),
        PUA: (range(0xf0, 0xfd), merge_ranges(range(0x40, 0x7f), range(0x80, 0xfd)), 0xe000)
    },
    {
        MAC_SEQUENCES: "MACJAPANESE.TXT"
    },
    {
        # https://www.unicode.org/Public/MAPPINGS/VENDORS/APPLE/CHINTRAD.TXT
        MAC_FILE: "MACCHINESETRAD.TXT",
        HIGH: range(0xa1, 0xfd),
        LOW: merge_ranges(range(0x40, 0x7f), range(0xa1, 0xff))
    },
    {
        MAC_SEQUENCES: "MACCHINESETRAD.TXT"
    },
    {
        # https://www.unicode.org/Public/MAPPINGS/VENDORS/APPLE/CHINSIMP.TXT
        MAC_FILE: "MACCHINESESIMP.TXT",
        HIGH: range(0xa1, 0xfd),
        LOW: range(0xa1, 0xff)
    },
    {
        MAC_SEQUENCES: "MACCHINESESIMP.TXT"
    },
]

# Generate each Mac file once for both its primary and sequence tables.
mac_tables = {}
mac_sequence_tables = {}
for v in tables:
    if MAC_FILE in v:
        filename = v[MAC_FILE]
        primary, sequences = process_mac_table(filename, v[HIGH], v[LOW], v.get(PUA))
        mac_tables[filename] = primary
        mac_sequence_tables[filename] = sequences

# number of tables
encdat.write(struct.pack("<i", len(tables)))

curofs = 16 + 4 * len(tables)

for v in tables:
    encdat.write(struct.pack("<i", curofs))
    if MAC_FILE in v:
        curofs += len(mac_tables[v[MAC_FILE]])
    elif MAC_SEQUENCES in v:
        curofs += len(mac_sequence_tables[v[MAC_SEQUENCES]])
    elif v[FILE] == "hanzi_charmap.py":
        curofs += min(len(traditional_charmap), len(simplified_charmap)) * 4 + 4
    else:
        curofs += len(v[HIGH]) * len(v[LOW]) * 2

for v in tables:
    if MAC_FILE in v:
        encdat.write(mac_tables[v[MAC_FILE]])
    elif MAC_SEQUENCES in v:
        encdat.write(mac_sequence_tables[v[MAC_SEQUENCES]])
    elif v[FILE] == "hanzi_charmap.py":
        process_hanzi_t2s(encdat)
    else:
        processtable(v[FILE], encdat, v[HIGH], v[LOW])
encdat.close()
