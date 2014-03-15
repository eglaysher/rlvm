// -*- Mode: C++; tab-width:2; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi:tw=80:et:ts=2:sts=2
//
// -----------------------------------------------------------------------
//
// This file is part of RLVM, a RealLive virtual machine clone.
//
// -----------------------------------------------------------------------
//
// Copyright (C) 2008 Elliot Glaysher
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
// -----------------------------------------------------------------------

#ifndef SRC_UTILITIES_DYNAMIC_BITSET_SERIALIZE_H_
#define SRC_UTILITIES_DYNAMIC_BITSET_SERIALIZE_H_

// boost::serialization doesn't come with built in support for
// serializing dynamic_bitset, so write our own version.
//
// This implementation is in no way optimal, but is meant to be
// portable. This representation isn't that bad once you think about
// how the data is going to be pumped through gzip.

#include <boost/dynamic_bitset.hpp>
#include <boost/serialization/split_free.hpp>
#include <boost/serialization/string.hpp>

#include <ostream>
#include <string>

namespace boost {
namespace serialization {

template <class Archive, class Block, class Allocator>
inline void save(Archive& ar,
                 const boost::dynamic_bitset<Block, Allocator>& bs,
                 const unsigned int /* file_version */) {
  std::string text_representation;
  boost::to_string(bs, text_representation);
  ar& text_representation;
}

template <class Archive, class Block, class Allocator>
inline void load(Archive& ar,
                 boost::dynamic_bitset<Block, Allocator>& bs,
                 const unsigned int /* file_version */) {
  std::string text_representation;
  ar& text_representation;
  bs = boost::dynamic_bitset<Block, Allocator>(text_representation);
}

// split non-intrusive serialization function member into separate
// non intrusive save/load member functions
template <class Archive, class Block, class Allocator>
inline void serialize(Archive& ar,
                      boost::dynamic_bitset<Block, Allocator>& bs,
                      const unsigned int file_version) {
  boost::serialization::split_free(ar, bs, file_version);
}

}  // namespace serialization
}  // namespace boost

#endif  // SRC_UTILITIES_DYNAMIC_BITSET_SERIALIZE_H_
