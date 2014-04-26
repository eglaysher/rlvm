// -*- Mode: C++; tab-width:2; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi:tw=80:et:ts=2:sts=2
//
// -----------------------------------------------------------------------
//
// This file is part of RLVM, a RealLive virtual machine clone.
//
// -----------------------------------------------------------------------
//
// Copyright (C) 2013 Elliot Glaysher
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
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
// -----------------------------------------------------------------------

#include "gtest/gtest.h"

#include "systems/base/rect.h"

TEST(RectTest, EmptyIntersection) {
  Rect empty;
  Rect empty2;
  Rect t = empty.Intersection(empty2);
  EXPECT_TRUE(t.is_empty());
}

TEST(RectTest, ZeroIntersection) {
  Rect one = Rect::REC(0, 0, 1, 1);
  Rect two = Rect::REC(5, 5, 1, 1);
  Rect t = one.Intersection(two);
  EXPECT_TRUE(t.is_empty());
}

TEST(RectTest, NormalIntersection) {
  Rect one = Rect::REC(0, 0, 6, 6);
  Rect two = Rect::REC(4, 4, 6, 6);
  EXPECT_EQ(Rect::REC(4, 4, 2, 2), one.Intersection(two));
}

TEST(RectTest, IdentityInsetRectangle) {
  Rect one = Rect::GRP(0, 0, 10, 10);
  Rect two = Rect::GRP(4, 4, 6, 6);
  EXPECT_EQ(two, one.GetInsetRectangle(two));
}

TEST(RectTest, BottomLeftInsetRectangle) {
  Rect one = Rect::GRP(1, 1, 9, 9);
  Rect two = Rect::GRP(1, 5, 5, 9);
  EXPECT_EQ(Rect::REC(0, 4, 4, 4), one.GetInsetRectangle(two));
}

// TODO: Write tests for applyInset.
