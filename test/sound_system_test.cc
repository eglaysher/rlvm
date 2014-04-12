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

#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>

#include "test_utils.h"

#include "test_system/test_system.h"
#include "test_system/test_sound_system.h"
#include "libreallive/gameexe.h"

#include <string>
#include <sstream>

using std::stringstream;

// Makes sure we can parse the bizarre Gameexe.ini keys for KOEONOFF
TEST(SoundSystem, CanParseKOEONOFFKeys) {
  TestSystem top(locateTestCase("Gameexe_data/Gameexe_koeonoff.ini"));
  SoundSystem& sys = top.sound();

  // Test the UseKoe side of things
  EXPECT_EQ(1, sys.ShouldUseKoeForCharacter(0));
  EXPECT_EQ(0, sys.ShouldUseKoeForCharacter(7));
  EXPECT_EQ(1, sys.ShouldUseKoeForCharacter(8));

  // Test the koePlay side of things
  EXPECT_EQ(5, sys.globals().character_koe_enabled.size());
  EXPECT_EQ(1, sys.globals().character_koe_enabled[0]);
  EXPECT_EQ(0, sys.globals().character_koe_enabled[3]);
  EXPECT_EQ(1, sys.globals().character_koe_enabled[2]);
  EXPECT_EQ(1, sys.globals().character_koe_enabled[20]);
  EXPECT_EQ(1, sys.globals().character_koe_enabled[105]);
}

// Tests that SetUseKoe sets values correctly
TEST(SoundSystem, SetUseKoeCorrectly) {
  TestSystem top(locateTestCase("Gameexe_data/Gameexe_koeonoff.ini"));
  SoundSystem& sys = top.sound();

  sys.SetUseKoeForCharacter(0, 0);
  sys.SetUseKoeForCharacter(7, 1);
  sys.SetUseKoeForCharacter(8, 0);

  // Make sure all values are flipped from the previous test.

  // Test the UseKoe side of things
  EXPECT_EQ(0, sys.ShouldUseKoeForCharacter(0));
  EXPECT_EQ(1, sys.ShouldUseKoeForCharacter(7));
  EXPECT_EQ(0, sys.ShouldUseKoeForCharacter(8));

  // Test the koePlay side of things
  EXPECT_EQ(5, sys.globals().character_koe_enabled.size());
  EXPECT_EQ(0, sys.globals().character_koe_enabled[0]);
  EXPECT_EQ(1, sys.globals().character_koe_enabled[3]);
  EXPECT_EQ(0, sys.globals().character_koe_enabled[2]);
  EXPECT_EQ(0, sys.globals().character_koe_enabled[20]);
  EXPECT_EQ(0, sys.globals().character_koe_enabled[105]);
}

// Make sure we thaw previously serialized character_koe_enabled data correctly.
TEST(SoundSystem, SetUseKoeSerialization) {
  std::string gexe = locateTestCase("Gameexe_data/Gameexe_koeonoff.ini");
  stringstream ss;
  {
    TestSystem top(gexe);
    SoundSystem& sys = top.sound();

    // Reverse the values as in <2>.
    sys.SetUseKoeForCharacter(0, 0);
    sys.SetUseKoeForCharacter(7, 1);
    sys.SetUseKoeForCharacter(8, 0);

    boost::archive::text_oarchive oa(ss);
    oa << const_cast<const SoundSystemGlobals&>(sys.globals());
  }
  {
    TestSystem top(gexe);
    SoundSystem& sys = top.sound();

    boost::archive::text_iarchive ia(ss);
    ia >> sys.globals();

    // Do the flip tests as in <2>

    // Test the UseKoe side of things
    EXPECT_EQ(0, sys.ShouldUseKoeForCharacter(0));
    EXPECT_EQ(1, sys.ShouldUseKoeForCharacter(7));
    EXPECT_EQ(0, sys.ShouldUseKoeForCharacter(8));

    // Test the koePlay side of things
    EXPECT_EQ(5, sys.globals().character_koe_enabled.size());
    EXPECT_EQ(0, sys.globals().character_koe_enabled[0]);
    EXPECT_EQ(1, sys.globals().character_koe_enabled[3]);
    EXPECT_EQ(0, sys.globals().character_koe_enabled[2]);
    EXPECT_EQ(0, sys.globals().character_koe_enabled[20]);
    EXPECT_EQ(0, sys.globals().character_koe_enabled[105]);
  }
}
