/*
   RealLive compression.
   Copyright (C) 2006 Haeleth

   This program is free software; you can redistribute it and/or modify it under
   the terms of the GNU General Public License as published by the Free Software
   Foundation; either version 2 of the License, or (at your option) any later
   version.

   This program is distributed in the hope that it will be useful, but WITHOUT
   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
   FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
   details.

   You should have received a copy of the GNU General Public License along with
   this program; if not, write to the Free Software Foundation, Inc., 59 Temple
   Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#ifndef COMPRESSION_H
#define COMPRESSION_H

#include "defs.h"
#include "lzcomp.h"

namespace LIBRL_NAMESPACE {
namespace Compression {

typedef AVG32Comp::Compress<AVG32Comp::CInfoRealLive, AVG32Comp::Container::RLDataContainer> RealliveCompressor;

void decompress(const char* src, size_t src_len, char* dst, size_t dst_len);
string* compress(char* arr, size_t len);
void apply_mask(char* array, size_t len);
void apply_mask(string& array, size_t start = 0);

}
}

#endif
