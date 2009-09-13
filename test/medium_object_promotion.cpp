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

#include <boost/assign.hpp>
#include <boost/assign/list_of.hpp>
#include <boost/tuple/tuple.hpp>

#include <string>
#include <vector>

#include "MachineBase/RLMachine.hpp"
#include "Modules/Module_Grp.hpp"
#include "Systems/Base/GraphicsObject.hpp"
#include "Systems/Base/GraphicsTextObject.hpp"
#include "TestSystem/TestMachine.hpp"
#include "TestSystem/TestSystem.hpp"
#include "libReallive/archive.h"

#include "testUtils.hpp"

using namespace boost;
using namespace boost::assign;

// A test suite that tests that certain graphical commands promote the object
// layer... and tests that others that are known to not promote objects don't.
typedef boost::tuple<
  std::string,                            // function name
  int,                                    // overload number
  TestMachine::ExeArgument,               // function arguments
  bool                                    // Should bg layer be promoted?
  > PromotionData;

class PromotionTest : public ::testing::TestWithParam<PromotionData> {
 public:
  PromotionTest()
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

enum CommandProperties {
  NONE = 0,
  SHOULD_PROMOTE_BG = 1
};

// Tests whether a certain command will promote an object from the background
// layer to the foreground.
TEST_P(PromotionTest, BgLayerPromotion) {
  PromotionData data = GetParam();
  GraphicsSystem& gs = system.graphics();

  // Build a dummy object into the background slot.
  GraphicsObject& obj = gs.getObject(OBJ_BG, 0);
  obj.setTextText("String");
  obj.setObjectData(new GraphicsTextObject(system));

  // Run the graphics command.
  rlmachine.exe(data.get<0>(), data.get<1>(), data.get<2>());

  // Whether the object was promoted to the fg layer.
  EXPECT_EQ(data.get<3>() & SHOULD_PROMOTE_BG,
            !gs.getObject(OBJ_FG, 0).isCleared());
}

std::vector<PromotionData> data =
    tuple_list_of
    // These commands shouldn't promote:
    ("grpLoad", 0, TestMachine::Arg("file", 0),
     NONE)
    ("grpLoad", 1, TestMachine::Arg("file", 0, 255),
     NONE)
    ("grpLoad", 2, TestMachine::Arg("file", 0, 0, 0, 200, 200, 0, 0),
     NONE)
    ("grpLoad", 3, TestMachine::Arg("file", 0, 0, 0, 200, 200, 0, 0, 255),
     NONE)
    ("recLoad", 0, TestMachine::Arg("file", 0),
     NONE)
    ("recLoad", 1, TestMachine::Arg("file", 0, 255),
     NONE)
    ("recLoad", 2, TestMachine::Arg("file", 0, 0, 0, 200, 200, 0, 0),
     NONE)
    ("recLoad", 3, TestMachine::Arg("file", 0, 0, 0, 200, 200, 0, 0, 255),
     NONE)
    ("grpMaskLoad", 0, TestMachine::Arg("file", 0),
     NONE)
    ("grpMaskLoad", 1, TestMachine::Arg("file", 0, 255),
     NONE)
    ("grpMaskLoad", 2, TestMachine::Arg("file", 0, 0, 0, 200, 200, 0, 0),
     NONE)
    ("grpMaskLoad", 3, TestMachine::Arg("file", 0, 0, 0, 200, 200, 0, 0, 255),
     NONE)
    ("recMaskLoad", 0, TestMachine::Arg("file", 0),
     NONE)
    ("recMaskLoad", 1, TestMachine::Arg("file", 0, 255),
     NONE)
    ("recMaskLoad", 2, TestMachine::Arg("file", 0, 0, 0, 200, 200, 0, 0),
     NONE)
    ("recMaskLoad", 3, TestMachine::Arg("file", 0, 0, 0, 200, 200, 0, 0, 255),
     NONE)

    ("grpCopy", 0, TestMachine::Arg(1, 0),
     NONE)
    ("grpCopy", 1, TestMachine::Arg(1, 0, 255),
     NONE)
    ("grpCopy", 2, TestMachine::Arg(0, 0, 200, 200, 1, 0, 0, 0),
     NONE)
    ("grpCopy", 3, TestMachine::Arg(0, 0, 200, 200, 1, 0, 0, 0, 255),
     NONE)
    ("recCopy", 0, TestMachine::Arg(1, 0),
     NONE)
    ("recCopy", 1, TestMachine::Arg(1, 0, 255),
     NONE)
    ("recCopy", 2, TestMachine::Arg(0, 0, 200, 200, 1, 0, 0, 0),
     NONE)
    ("recCopy", 3, TestMachine::Arg(0, 0, 200, 200, 1, 0, 0, 0, 255),
     NONE)

    // These commands should promote:
    ("grpDisplay", 0, TestMachine::Arg(0, 5),
     SHOULD_PROMOTE_BG)
    ("grpDisplay", 1, TestMachine::Arg(0, 5, 255),
     SHOULD_PROMOTE_BG)
    ("grpDisplay", 2, TestMachine::Arg(0, 5, 0, 0, 200, 200, 0, 0),
     SHOULD_PROMOTE_BG)
    ("grpDisplay", 3, TestMachine::Arg(0, 5, 0, 0, 200, 200, 0, 0, 255),
     SHOULD_PROMOTE_BG)
    ("recDisplay", 0, TestMachine::Arg(0, 5),
     SHOULD_PROMOTE_BG)
    ("recDisplay", 1, TestMachine::Arg(0, 5, 255),
     SHOULD_PROMOTE_BG)
    ("recDisplay", 2, TestMachine::Arg(0, 5, 0, 0, 200, 200, 0, 0),
     SHOULD_PROMOTE_BG)
    ("recDisplay", 3, TestMachine::Arg(0, 5, 0, 0, 200, 200, 0, 0, 255),
     SHOULD_PROMOTE_BG)

    ("grpOpenBg", 0, TestMachine::Arg("file", 0),
     SHOULD_PROMOTE_BG)
    ("grpOpenBg", 1, TestMachine::Arg("file", 0, 255),
     SHOULD_PROMOTE_BG)
    ("grpOpenBg", 2, TestMachine::Arg("file", 0, 0, 0, 200, 200, 0, 0),
     SHOULD_PROMOTE_BG)
    ("grpOpenBg", 3, TestMachine::Arg("file", 0, 0, 0, 200, 200, 0, 0, 255),
     SHOULD_PROMOTE_BG)
    ("recOpenBg", 0, TestMachine::Arg("file", 0),
     SHOULD_PROMOTE_BG)
    ("recOpenBg", 1, TestMachine::Arg("file", 0, 255),
     SHOULD_PROMOTE_BG)
    ("recOpenBg", 2, TestMachine::Arg("file", 0, 0, 0, 200, 200, 0, 0),
     SHOULD_PROMOTE_BG)
    ("recOpenBg", 3, TestMachine::Arg("file", 0, 0, 0, 200, 200, 0, 0, 255),
     SHOULD_PROMOTE_BG)

    ("grpMaskOpen", 0, TestMachine::Arg("file", 0),
     SHOULD_PROMOTE_BG)
    ("grpMaskOpen", 1, TestMachine::Arg("file", 0, 255),
     SHOULD_PROMOTE_BG)
    ("grpMaskOpen", 2, TestMachine::Arg("file", 0, 0, 0, 200, 200, 0, 0),
     SHOULD_PROMOTE_BG)
    ("grpMaskOpen", 3, TestMachine::Arg("file", 0, 0, 0, 200, 200, 0, 0, 255),
     SHOULD_PROMOTE_BG)
    ("recMaskOpen", 0, TestMachine::Arg("file", 0),
     SHOULD_PROMOTE_BG)
    ("recMaskOpen", 1, TestMachine::Arg("file", 0, 255),
     SHOULD_PROMOTE_BG)
    ("recMaskOpen", 2, TestMachine::Arg("file", 0, 0, 0, 200, 200, 0, 0),
     SHOULD_PROMOTE_BG)
    ("recMaskOpen", 3, TestMachine::Arg("file", 0, 0, 0, 200, 200, 0, 0, 255),
     SHOULD_PROMOTE_BG)

    ("recMulti", 0, TestMachine::Arg("file", 0),
     SHOULD_PROMOTE_BG)
    ("recMulti", 1, TestMachine::Arg("file", 0, 255),
     SHOULD_PROMOTE_BG)

    ("grpOpen", 0, TestMachine::Arg("file", 0),
     SHOULD_PROMOTE_BG)
    ("grpOpen", 1, TestMachine::Arg("file", 0, 255),
     SHOULD_PROMOTE_BG)
    ("grpOpen", 2, TestMachine::Arg("file", 0, 0, 0, 200, 200, 0, 0),
     SHOULD_PROMOTE_BG)
    ("grpOpen", 3, TestMachine::Arg("file", 0, 0, 0, 200, 200, 0, 0, 255),
     SHOULD_PROMOTE_BG)
    ("recOpen", 0, TestMachine::Arg("file", 0),
     SHOULD_PROMOTE_BG)
    ("recOpen", 1, TestMachine::Arg("file", 0, 255),
     SHOULD_PROMOTE_BG)
    ("recOpen", 2, TestMachine::Arg("file", 0, 0, 0, 200, 200, 0, 0),
     SHOULD_PROMOTE_BG)
    ("recOpen", 3, TestMachine::Arg("file", 0, 0, 0, 200, 200, 0, 0, 255),
     SHOULD_PROMOTE_BG);

INSTANTIATE_TEST_CASE_P(MediumObjectPoromotion,
                        PromotionTest,
                        ::testing::ValuesIn(data));
