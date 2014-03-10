// -*- Mode: C++; tab-width:2; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi:tw=80:et:ts=2:sts=2
//
// -----------------------------------------------------------------------
//
// This file is part of RLVM, a RealLive virtual machine clone.
//
// -----------------------------------------------------------------------
//
// Copyright (C) 2011 Elliot Glaysher
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

#include "gtest/gtest.h"

#include <tuple>

#include "test_utils.h"
#include "modules/module_sys_index_series.h"

using std::get;

class IndexSeriesTest : public FullSystemTest {
 public:
  void verifyIndexSeries(IndexList::type index_list,
                         int offset,
                         int init,
                         const std::vector<std::tuple<int, int>>& tests) {
    Sys_index_series fun;
    for (std::vector<std::tuple<int, int>>::const_iterator it = tests.begin();
         it != tests.end();
         ++it) {
      EXPECT_EQ(get<1>(*it),
                fun(rlmachine, get<0>(*it), offset, init, index_list));
    }
  }

  IndexList::type addValue(IndexList::type orig, int val) {
    IndexSeriesEntry::type a = {0, val};
    orig.push_back(a);
    return orig;
  }

  IndexList::type addMode0(IndexList::type orig,
                           int start,
                           int end,
                           int endval) {
    IndexSeriesEntry::type a = {1, 0,
                                std::tuple<int, int, int>(start, end, endval)};
    orig.push_back(a);
    return orig;
  }
};

TEST_F(IndexSeriesTest, SimpleMode0Test) {
  verifyIndexSeries(
      addMode0(IndexList::type(), 0, 10, 5),
      0,
      0,
      {std::make_tuple(0, 0), std::make_tuple(1, 0), std::make_tuple(2, 1),
       std::make_tuple(3, 1), std::make_tuple(4, 2), std::make_tuple(5, 2),
       std::make_tuple(6, 3), std::make_tuple(7, 3), std::make_tuple(8, 4),
       std::make_tuple(9, 4), std::make_tuple(10, 5)});
}

TEST_F(IndexSeriesTest, RldevExample) {
  // This example is listed in the rldev documentation.
  verifyIndexSeries(
      addMode0(addMode0(IndexList::type(), 0, 5, 10), 8, 10, 0),
      0,
      5,
      {std::make_tuple(0, 5), std::make_tuple(1, 6), std::make_tuple(2, 7),
       std::make_tuple(3, 8), std::make_tuple(4, 9), std::make_tuple(5, 10),
       std::make_tuple(6, 10), std::make_tuple(7, 10), std::make_tuple(8, 10),
       std::make_tuple(9, 5), std::make_tuple(10, 0)});
}

TEST_F(IndexSeriesTest, RawValueTest) {
  // NOTE: Reallive.exe thinks its (9, 19). (9, 20) is what rlvm says, probably
  // due to rounding error.
  verifyIndexSeries(
      addMode0(addValue(addMode0(IndexList::type(), 0, 5, 10), 40), 8, 10, 0),
      0,
      5,
      {std::make_tuple(0, 5), std::make_tuple(1, 6), std::make_tuple(2, 7),
       std::make_tuple(3, 8), std::make_tuple(4, 9), std::make_tuple(5, 40),
       std::make_tuple(6, 40), std::make_tuple(7, 40), std::make_tuple(8, 40),
       std::make_tuple(9, 20), std::make_tuple(10, 0)});
}
