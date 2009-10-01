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
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
// -----------------------------------------------------------------------

#include "gtest/gtest.h"

#include "MachineBase/RLMachine.hpp"
#include "Modules/Module_Grp.hpp"
#include "Systems/Base/Colour.hpp"
#include "TestSystem/MockSurface.hpp"
#include "TestSystem/TestMachine.hpp"
#include "TestSystem/TestSystem.hpp"
#include "libReallive/archive.h"

#include "testUtils.hpp"

class MediumGrpTest : public ::testing::Test {
 protected:
  MediumGrpTest()
      : arc(locateTestCase("Module_Str_SEEN/strcpy_0.TXT")),
        system(locateTestCase("Gameexe_data/Gameexe.ini")),
        rlmachine(system, arc) {
    rlmachine.attachModule(new GrpModule);
  }

  // Use any old test case; it isn't getting executed
  libReallive::Archive arc;
  TestSystem system;
  TestMachine rlmachine;
};

TEST_F(MediumGrpTest, TestWipe) {
  EXPECT_CALL(system.graphics().getMockDC(0),
              fill(RGBAColour(128, 128, 128, 255)));
  rlmachine.exe("wipe", 0, TestMachine::Arg(0, 128, 128, 128));
}

TEST_F(MediumGrpTest, TestFill_0) {
  EXPECT_CALL(system.graphics().getMockDC(0),
              fill(RGBAColour(128, 128, 128, 255)));
  rlmachine.exe("recFill", 0, TestMachine::Arg(0, 128, 128, 128));
}

TEST_F(MediumGrpTest, TestFill_1) {
  EXPECT_CALL(system.graphics().getMockDC(0),
              fill(RGBAColour(128, 128, 128, 255)));
  rlmachine.exe("recFill", 1, TestMachine::Arg(0, 128, 128, 128, 255));
}

TEST_F(MediumGrpTest, TestFill_2) {
  EXPECT_CALL(system.graphics().getMockDC(0),
              fill(RGBAColour(128, 128, 128, 255), Rect(10, 10, Size(20, 20))));
  rlmachine.exe("recFill", 2,
                TestMachine::Arg(10, 10, 20, 20, 0, 128, 128, 128));
}

TEST_F(MediumGrpTest, TestFill_3) {
  EXPECT_CALL(system.graphics().getMockDC(0),
              fill(RGBAColour(128, 128, 128, 255), Rect(10, 10, Size(20, 20))));
  rlmachine.exe("recFill", 3,
                TestMachine::Arg(10, 10, 20, 20, 0, 128, 128, 128, 255));
}

TEST_F(MediumGrpTest, TestMono_1) {
  EXPECT_CALL(system.graphics().getMockDC(0),
              mono(system.graphics().getMockDC(0).rect()));
  rlmachine.exe("recMono", 0, TestMachine::Arg(0));
}

TEST_F(MediumGrpTest, TestMono_3) {
  EXPECT_CALL(system.graphics().getMockDC(0), mono(Rect(5, 6, Size(7, 8))));
  rlmachine.exe("recMono", 2, TestMachine::Arg(5, 6, 7, 8, 0));
}

TEST_F(MediumGrpTest, TestColour_1) {
  EXPECT_CALL(system.graphics().getMockDC(0),
              applyColour(RGBColour(128, 128, 128),
                          Rect(0, 0, Size(640, 480))));
  rlmachine.exe("recColour", 0, TestMachine::Arg(0, 128, 128, 128));
}

TEST_F(MediumGrpTest, TestColour_3) {
  EXPECT_CALL(system.graphics().getMockDC(0),
              applyColour(RGBColour(128, 128, 128),
                          Rect(0, 0, Size(640, 480))));
  rlmachine.exe("recColour", 1,
                TestMachine::Arg(0, 0, 640, 480, 0, 128, 128, 128));
}

TEST_F(MediumGrpTest, TestLight_1) {
  EXPECT_CALL(system.graphics().getMockDC(0),
              applyColour(RGBColour(128, 128, 128),
                          Rect(0, 0, Size(640, 480))));
  rlmachine.exe("recLight", 0, TestMachine::Arg(0, 128));
}

TEST_F(MediumGrpTest, TestLight_3) {
  EXPECT_CALL(system.graphics().getMockDC(0),
              applyColour(RGBColour(128, 128, 128),
                          Rect(0, 0, Size(640, 480))));
  rlmachine.exe("recLight", 1, TestMachine::Arg(0, 0, 640, 480, 0, 128));
}

TEST_F(MediumGrpTest, TestInvert_1) {
  EXPECT_CALL(system.graphics().getMockDC(0),
              invert(system.graphics().getMockDC(0).rect()));
  rlmachine.exe("recInvert", 0, TestMachine::Arg(0));
}

TEST_F(MediumGrpTest, TestInvert_3) {
  EXPECT_CALL(system.graphics().getMockDC(0), invert(Rect(5, 6, Size(7, 8))));
  rlmachine.exe("recInvert", 2, TestMachine::Arg(5, 6, 7, 8, 0));
}

TEST_F(MediumGrpTest, TestFade_6) {
  EXPECT_CALL(system.graphics().getMockDC(0),
              fill(RGBAColour(128, 128, 128, 255), Rect(10, 10, Size(20, 20))));
  rlmachine.exe("recFade", 6,
                TestMachine::Arg(10, 10, 20, 20, 128, 128, 128));
}

TEST_F(MediumGrpTest, TestFade_7) {
  EXPECT_CALL(system.graphics().getMockDC(0),
              fill(RGBAColour(128, 128, 128, 255), Rect(10, 10, Size(20, 20))));
  rlmachine.exe("recFade", 7,
                TestMachine::Arg(10, 10, 20, 20, 128, 128, 128, 0));
}
