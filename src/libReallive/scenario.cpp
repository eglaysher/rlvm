// -*- Mode: C++; tab-width:2; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi:tw=80:et:ts=2:sts=2
//
// -----------------------------------------------------------------------
//
// This file is part of libReallive, a dependency of RLVM.
//
// -----------------------------------------------------------------------
//
// Copyright (c) 2006 Peter Jolly
//
// Permission is hereby granted, free of charge, to any person
// obtaining a copy of this software and associated documentation
// files (the "Software"), to deal in the Software without
// restriction, including without limitation the rights to use, copy,
// modify, merge, publish, distribute, sublicense, and/or sell copies
// of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be
// included in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
// NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
// BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
// ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
// CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//
// -----------------------------------------------------------------------

#include "scenario.h"
#include "compression.h"
#include <cassert>

#include <algorithm>
#include <sstream>

#include "Utilities/Exception.hpp"
#include "Utilities/StringUtilities.hpp"
#include "Utilities/gettext.h"

using namespace std;

namespace libReallive {

Metadata::Metadata() : encoding(0) {}

void
Metadata::assign(const char* input) {
  const int meta_len = read_i32(input),
            id_len   = read_i32(input + 4) + 1;
  if (meta_len < id_len + 17) return; // malformed metadata
  as_string.assign(input, meta_len);
  encoding = input[id_len + 16];
}

Header::Header(const char* data, const size_t length) {
  if (length < 0x1d0)
    throw Error("not a RealLive bytecode file");

  string compiler = string(data, 4);

  // Check the version of the compiler.
  if (read_i32(data + 4) == 10002)
    use_xor_2 = false;
  else if (read_i32(data + 4) == 110002)
    use_xor_2 = true;
  else if (read_i32(data + 4) == 1110002)
    use_xor_2 = true;
  else {
    // New xor key?
    ostringstream oss;
    oss << "Unsupported compiler version: " << read_i32(data + 4);
    throw Error(oss.str());
  }

  if (read_i32(data) != 0x1d0)
    throw Error("unsupported bytecode version");

  // Debug entrypoints
  zminusone = read_i32(data + 0x2c);
  zminustwo = read_i32(data + 0x30);

  // Misc settings
  savepoint_message = read_i32(data + 0x1c4);
  savepoint_selcom  = read_i32(data + 0x1c8);
  savepoint_seentop = read_i32(data + 0x1cc);

  // Dramatis personae
  int dplen = read_i32(data + 0x18);
  dramatis_personae.reserve(dplen);
  int offs = read_i32(data + 0x14);
  while (dplen--) {
    int elen = read_i32(data + offs);
    dramatis_personae.push_back(string(data + offs + 4, elen - 1));
    offs += elen + 4;
  }

  // If this scenario was compiled with RLdev, it may include a
  // potentially-useful metadata block.  Check for that and read it if
  // it's present.
  offs = read_i32(data + 0x14) + read_i32(data + 0x1c);
  if (offs != read_i32(data + 0x20)) {
    rldev_metadata.assign(data + offs);
  }
}

Script::Script(const Header& hdr, const char* data, const size_t length,
               const std::string& regname,
               bool use_xor_2, const Compression::XorKey* second_level_xor_key)
  : uptodate(true), strip(false) {
  // Kidoku/entrypoint table
  const int kidoku_offs = read_i32(data + 0x08);
  const size_t kidoku_length = read_i32(data + 0x0c);
  ConstructionData cdat(kidoku_length, elts.end());
  for (size_t i = 0; i < kidoku_length; ++i)
    cdat.kidoku_table[i] =  read_i32(data + kidoku_offs + i * 4);

  // Decompress data
  const size_t dlen = read_i32(data + 0x24);

  const Compression::XorKey* key = NULL;
  if (use_xor_2) {
    if (second_level_xor_key) {
      key = second_level_xor_key;
    } else {
      // Probably safe to assume that any game we don't know about has a
      // Japanese encoding.
      throw rlvm::UserPresentableError(
          str(format(_("Can not read game script for %1%")) %
              cp932toUTF8(regname, 0)),
          _("Some games require individual reverse engineering. This game can "
            "not be played until someone has figured out how the game script "
            "is encoded."));
    }
  }

  char* uncompressed = new char[dlen];
  Compression::decompress(data + read_i32(data + 0x20),
                          read_i32(data + 0x28),
                          uncompressed,
                          dlen,
                          key);
  // Read bytecode
  const char* stream = uncompressed;
  const char* end = uncompressed + dlen;
  size_t pos = 0;
  while (pos < dlen) {
    // Read element
    elts.push_back(BytecodeElement::read(stream, end, cdat));
    pointer_t it = elts.end();
    cdat.offsets[pos] = --it;

    // Keep track of the entrypoints
    if (it->type() == Entrypoint) {
      entrypointAssociations.insert(make_pair(it->entrypoint(), it));
    }

    // Advance
    size_t l = it->length();
    if (l <= 0) l = 1; // Failsafe: always advance at least one byte.
    stream += l;
    pos += l;
  }
  lencache = pos;
  // Resolve pointers
  for (pointer_t it = elts.begin(); it != elts.end(); ++it) {
    it->set_pointers(cdat);
  }

  delete [] uncompressed;
}

const pointer_t Script::getEntrypoint(int entrypoint) const {
  pointernumber::const_iterator it = entrypointAssociations.find(entrypoint);
  if (it == entrypointAssociations.end())
    throw Error("Unknown entrypoint");

  return it->second;
}

Scenario::const_iterator Scenario::findEntrypoint(int entrypoint) const {
  return script.getEntrypoint(entrypoint);
}

}
