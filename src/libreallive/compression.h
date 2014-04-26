// This file is part of libreallive, a dependency of RLVM.
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

#ifndef SRC_LIBREALLIVE_COMPRESSION_H_
#define SRC_LIBREALLIVE_COMPRESSION_H_

#include <string>

#include "libreallive/defs.h"

namespace libreallive {
namespace compression {

// An individual xor key; some games use multiple ones.
struct XorKey {
  char xor_key[16];
  int xor_offset;
  int xor_length;
};

// Per game xor keys to be passed to decompress, terminated with -1 offset
// entries.
extern const XorKey little_busters_xor_mask[];
extern const XorKey clannad_full_voice_xor_mask[];
extern const XorKey little_busters_ex_xor_mask[];
extern const XorKey snow_standard_edition_xor_mask[];
extern const XorKey kud_wafter_xor_mask[];
extern const XorKey kud_wafter_all_ages_xor_mask[];

void Decompress(const char* src, size_t src_len, char* dst, size_t dst_len,
                const XorKey* per_game_xor_key);

}  // namespace compression
}  // namespace libreallive

#endif  // SRC_LIBREALLIVE_COMPRESSION_H_
