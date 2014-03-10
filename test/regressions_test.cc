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

#include "gtest/gtest.h"

#include "utilities/string_utilities.h"

// Regression that came up when I switched my tables onto Haeleth's rlBabel
// interface.
TEST(Regressions, cp932toUTF8) {
  EXPECT_EQ(
      "\xe3\x83\x9e\xe3\x82\xb8\xef\xbc\x9f\xe3\x80\x80\xe3\x81\x84\xe3\x81\x84"
      "\xe3\x81\xae\xe3\x81\x8b\xef\xbc\x9f",  // NOLINT
      cp932toUTF8(
          "\x83\x7d\x83\x57\x81\x48\x81\x40\x82\xa2\x82\xa2\x82\xcc\x82\xa9\x81"
          "\x48",
          0))  // NOLINT
      << "Didn't convert the string 'maji? iinoka?' correctly.";
}
