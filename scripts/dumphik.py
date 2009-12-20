#!/usr/bin/python
#
# A fairly naive script which dumps out the data of a HIK file to a JSON file
# for editing and reencoding with buildhik.py.

import json
import struct
import sys

def read_string(f):
  length = struct.unpack("i", f.read(0x4))[0]
  format_str = str(length) + "s"
  return struct.unpack(format_str, f.read(length))

property_formats = {
  20001: lambda f: struct.unpack("i", f.read(0x4)),
  20100: read_string,
  20101: lambda f: struct.unpack("ii", f.read(0x8)),
  21000: lambda f: struct.unpack("i", f.read(0x4)),
  21001: lambda f: struct.unpack("iiii", f.read(0x10)),
  21002: lambda f: struct.unpack("iiiii", f.read(0x14)),
  21003: lambda f: struct.unpack("i", f.read(0x4)),
  21100: lambda f: struct.unpack("i", f.read(0x4)),
  21101: lambda f: struct.unpack("iiii", f.read(0x10)),
  21200: lambda f: struct.unpack("i", f.read(0x4)),
  21201: lambda f: struct.unpack("iiii", f.read(0x10)),
  21202: lambda f: struct.unpack("ii", f.read(0x8)),
  21203: lambda f: struct.unpack("i", f.read(0x4)),
  21301: lambda f: struct.unpack("i", f.read(0x4)),
  21300: lambda f: struct.unpack("iiii", f.read(0x10)),
  30000: lambda f: struct.unpack("i", f.read(0x4)),
  30001: lambda f: struct.unpack("i", f.read(0x4)),
  30100: lambda f: struct.unpack("i", f.read(0x4)),
  30101: lambda f: struct.unpack("i", f.read(0x4)),
  30102: lambda f: struct.unpack("i", f.read(0x4)),
  40000: lambda f: struct.unpack("i", f.read(0x4)),
  40101: lambda f: struct.unpack("iiiiiiiiiiiiiiiiiiiiiiiiiiiiiii", f.read(0x7c)),
  40102: lambda f: struct.unpack("i", f.read(0x4)),
  40103: lambda f: struct.unpack("ii", f.read(0x8)),
  40100: read_string,
}

results = [ ]

if len(sys.argv) != 3:
  print "Usage: " + sys.argv[0] + " <input hik file> <output json>"
  exit(-1)

with open(sys.argv[1], "rb") as f:
  # Get the header but don't do anything interesting with it yet.
  header = f.read(0x30)

  # TODO(erg): This value may not be the number of entries! See LEAF.hik in ALMA.
  entries = struct.unpack("i", f.read(0x4))[0]
  for i in range(entries):
    record = [ ]

    property_id = struct.unpack("i", f.read(0x4))[0]
    while property_id != -1:
      if property_id in property_formats:
        record.append([property_id, property_formats[property_id](f)])
      else:
        print "huh: " + str(property_id)

      property_id = struct.unpack("i", f.read(0x4))[0]

    # Consume the weird 100.
    struct.unpack("i", f.read(0x4))[0]

    results.append(record)

with open(sys.argv[2], "w") as f:
  print >>f, json.dumps(results, sort_keys=True, indent=4)
