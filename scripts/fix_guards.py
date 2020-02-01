#!/usr/bin/python3

import sys
import os
import re

ifndef = re.compile('^\#ifndef')
define = re.compile('^\#define')
endif  = re.compile('^\#endif')

name = sys.argv[1]
bak_name = name + ".bak"

new_guard_name = name.replace('/', '_').replace('.', '_').upper()
new_guard_name += "_"

try:
  os.rename(name, bak_name)

  input = file(bak_name, 'r')
  output = file(name, 'w')

  for line in input:
    m = ifndef.search(line)
    if m is not None:
      output.write('#ifndef ' + new_guard_name + '\n')
      continue

    m = define.search(line)
    if m is not None:
      output.write('#define ' + new_guard_name + '\n')
      continue

    m = endif.search(line)
    if m is not None:
      output.write('#endif  // ' + new_guard_name + '\n')
      continue

    output.write(line)

  input.close()
  output.close()
except IOError:
  sys.stderr.write('Cannot open "%s"\n' % arg)
