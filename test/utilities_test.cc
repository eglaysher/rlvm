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

#include "libreallive/gameexe.h"
#include "systems/base/rect.h"
#include "utilities/graphics.h"

TEST(UtilitiesTest, ClipDestination_Superset) {
  Rect clip(Point(5, 5), Size(5, 5));
  Rect src(Point(0, 0), Size(10, 10));
  Rect dest(Point(0, 0), Size(10, 10));

  ClipDestination(clip, src, dest);
  EXPECT_EQ(clip, dest);
}

TEST(UtilitiesTest, ClipDestination_Overlap) {
  Rect clip(Point(5, 5), Size(5, 5));
  Rect src(Point(0, 0), Size(15, 5));
  Rect dest(Point(0, 5), Size(15, 5));

  ClipDestination(clip, src, dest);
  EXPECT_EQ(clip, dest);
}

TEST(UtilitiesTest, ScreenSizeModKey) {
  // Old games
  Gameexe old;
  old.parseLine("#SCREENSIZE_MOD=0");
  EXPECT_EQ(Size(640, 480), GetScreenSize(old));

  // Newer games
  Gameexe newer;
  newer.parseLine("#SCREENSIZE_MOD=1");
  EXPECT_EQ(Size(800, 600), GetScreenSize(newer));

  // Used in Memorial Edition games
  Gameexe me;
  me.parseLine("#SCREENSIZE_MOD=999,800,600");
  EXPECT_EQ(Size(800, 600), GetScreenSize(me));
}
