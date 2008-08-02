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

#ifndef COMPRESSION_H
#define COMPRESSION_H

#include "defs.h"
#include "lzcomp.h"

namespace libReallive {
namespace Compression {

typedef AVG32Comp::Compress<AVG32Comp::CInfoRealLive, AVG32Comp::Container::RLDataContainer> RealliveCompressor;

// Per game xor keys to be passed to decompress.
extern const char little_busters_xor_mask_2[];
extern const char clannad_full_voice_xor_mask_2[];

void decompress(const char* src, size_t src_len, char* dst, size_t dst_len,
                const char* per_game_xor_key);
string* compress(char* arr, size_t len);
void apply_mask(char* array, size_t len);
void apply_mask(string& array, size_t start = 0);

}
}

#endif
