// -*- Mode: C++; tab-width:2; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi:tw=80:et:ts=2:sts=2
//
// -----------------------------------------------------------------------
//
// This file is part of RLVM, a RealLive virtual machine clone.
//
// -----------------------------------------------------------------------
//
// Copyright (C) 2009 Elliot Glaysher
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

#ifndef SRC_UTILITIES_ALGOPLUS_HPP_
#define SRC_UTILITIES_ALGOPLUS_HPP_

// Performs f() on any element elem in [first, last) where pred(elem) is true
template<typename InputIterator, typename Predicate, typename UnaryFunction>
UnaryFunction for_each_if(InputIterator first, InputIterator last,
                          Predicate pred, UnaryFunction f) {
  for (;first != last; ++first) {
    if (pred(*first)) {
      f(*first);
    }
  }
  return f;
}

// -----------------------------------------------------------------------

template <typename InputIterator, typename OutputIterator, typename Predicate>
OutputIterator copy_if(InputIterator first, InputIterator last,
                       OutputIterator dest, Predicate p) {
  for (; first != last; ++first) {
    if (p(*first)) {
      *dest = *first;
      ++dest;
    }
  }
  return dest;
}

// -----------------------------------------------------------------------

// Copies until the predicate is true
template<typename InIt, typename OutIt, typename Pred>
OutIt copy_until(InIt first, InIt end, OutIt x, Pred pred) {
  for (; first != end; ++first, ++x) {
    if (pred(*first)) {
      return x;
    }
    *x = *first;
  }
  return x;
}

#endif  // SRC_UTILITIES_ALGOPLUS_HPP_
