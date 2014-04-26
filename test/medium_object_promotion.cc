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

#include <string>
#include <tuple>
#include <vector>

#include "libreallive/archive.h"
#include "machine/rlmachine.h"
#include "modules/module_grp.h"
#include "systems/base/graphics_object.h"
#include "systems/base/graphics_text_object.h"
#include "test_system/test_machine.h"
#include "test_system/test_system.h"

#include "test_utils.h"

using namespace boost;

using std::get;

// A test suite that tests that certain graphical commands promote the object
// layer... and tests that others that are known to not promote objects don't.
typedef std::tuple<std::string,               // function name
                   int,                       // overload number
                   TestMachine::ExeArgument,  // function arguments
                   bool                       // Should bg layer be promoted?
                   > PromotionData;

class PromotionTest : public ::testing::TestWithParam<PromotionData> {
 public:
  PromotionTest()
      : arc(locateTestCase("Module_Str_SEEN/strcpy_0.TXT")),
        system(locateTestCase("Gameexe_data/Gameexe.ini")),
        rlmachine(system, arc) {
    rlmachine.AttachModule(new GrpModule);
  }

  // Use any old test case; it isn't getting executed
  libreallive::Archive arc;
  TestSystem system;
  TestMachine rlmachine;
};

enum CommandProperties { NONE = 0, SHOULD_PROMOTE_BG = 1 };

// Tests whether a certain command will promote an object from the background
// layer to the foreground.
TEST_P(PromotionTest, BgLayerPromotion) {
  PromotionData data = GetParam();
  GraphicsSystem& gs = system.graphics();

  // Build a dummy object into the background slot.
  GraphicsObject& obj = gs.GetObject(OBJ_BG, 0);
  obj.SetTextText("String");
  obj.SetObjectData(new GraphicsTextObject(system));

  // Run the graphics command.
  rlmachine.Exe(get<0>(data), get<1>(data), get<2>(data));

  // Whether the object was promoted to the fg layer.
  EXPECT_EQ(get<3>(data) & SHOULD_PROMOTE_BG,
            !gs.GetObject(OBJ_FG, 0).is_cleared());
}

std::vector<PromotionData> data = {
    // These commands shouldn't promote:
    std::make_tuple("grpLoad", 0, TestMachine::Arg("file", 0), NONE),
    std::make_tuple("grpLoad", 1, TestMachine::Arg("file", 0, 255), NONE),
    std::make_tuple("grpLoad",
                    2,
                    TestMachine::Arg("file", 0, 0, 0, 200, 200, 0, 0),
                    NONE),
    std::make_tuple("grpLoad",
                    3,
                    TestMachine::Arg("file", 0, 0, 0, 200, 200, 0, 0, 255),
                    NONE),
    std::make_tuple("recLoad", 0, TestMachine::Arg("file", 0), NONE),
    std::make_tuple("recLoad", 1, TestMachine::Arg("file", 0, 255), NONE),
    std::make_tuple("recLoad",
                    2,
                    TestMachine::Arg("file", 0, 0, 0, 200, 200, 0, 0),
                    NONE),
    std::make_tuple("recLoad",
                    3,
                    TestMachine::Arg("file", 0, 0, 0, 200, 200, 0, 0, 255),
                    NONE),
    std::make_tuple("grpMaskLoad", 0, TestMachine::Arg("file", 0), NONE),
    std::make_tuple("grpMaskLoad", 1, TestMachine::Arg("file", 0, 255), NONE),
    std::make_tuple("grpMaskLoad",
                    2,
                    TestMachine::Arg("file", 0, 0, 0, 200, 200, 0, 0),
                    NONE),
    std::make_tuple("grpMaskLoad",
                    3,
                    TestMachine::Arg("file", 0, 0, 0, 200, 200, 0, 0, 255),
                    NONE),
    std::make_tuple("recMaskLoad", 0, TestMachine::Arg("file", 0), NONE),
    std::make_tuple("recMaskLoad", 1, TestMachine::Arg("file", 0, 255), NONE),
    std::make_tuple("recMaskLoad",
                    2,
                    TestMachine::Arg("file", 0, 0, 0, 200, 200, 0, 0),
                    NONE),
    std::make_tuple("recMaskLoad",
                    3,
                    TestMachine::Arg("file", 0, 0, 0, 200, 200, 0, 0, 255),
                    NONE),
    std::make_tuple("grpCopy", 0, TestMachine::Arg(1, 0), NONE),
    std::make_tuple("grpCopy", 1, TestMachine::Arg(1, 0, 255), NONE),
    std::make_tuple("grpCopy",
                    2,
                    TestMachine::Arg(0, 0, 200, 200, 1, 0, 0, 0),
                    NONE),
    std::make_tuple("grpCopy",
                    3,
                    TestMachine::Arg(0, 0, 200, 200, 1, 0, 0, 0, 255),
                    NONE),
    std::make_tuple("recCopy", 0, TestMachine::Arg(1, 0), NONE),
    std::make_tuple("recCopy", 1, TestMachine::Arg(1, 0, 255), NONE),
    std::make_tuple("recCopy",
                    2,
                    TestMachine::Arg(0, 0, 200, 200, 1, 0, 0, 0),
                    NONE),
    std::make_tuple("recCopy",
                    3,
                    TestMachine::Arg(0, 0, 200, 200, 1, 0, 0, 0, 255),
                    NONE),

    // These commands should promote:
    std::make_tuple("grpDisplay", 0, TestMachine::Arg(0, 5), SHOULD_PROMOTE_BG),
    std::make_tuple("grpDisplay",
                    1,
                    TestMachine::Arg(0, 5, 255),
                    SHOULD_PROMOTE_BG),
    std::make_tuple("grpDisplay",
                    2,
                    TestMachine::Arg(0, 5, 0, 0, 200, 200, 0, 0),
                    SHOULD_PROMOTE_BG),
    std::make_tuple("grpDisplay",
                    3,
                    TestMachine::Arg(0, 5, 0, 0, 200, 200, 0, 0, 255),
                    SHOULD_PROMOTE_BG),
    std::make_tuple("recDisplay", 0, TestMachine::Arg(0, 5), SHOULD_PROMOTE_BG),
    std::make_tuple("recDisplay",
                    1,
                    TestMachine::Arg(0, 5, 255),
                    SHOULD_PROMOTE_BG),
    std::make_tuple("recDisplay",
                    2,
                    TestMachine::Arg(0, 5, 0, 0, 200, 200, 0, 0),
                    SHOULD_PROMOTE_BG),
    std::make_tuple("recDisplay",
                    3,
                    TestMachine::Arg(0, 5, 0, 0, 200, 200, 0, 0, 255),
                    SHOULD_PROMOTE_BG),
    std::make_tuple("grpOpenBg",
                    0,
                    TestMachine::Arg("file", 0),
                    SHOULD_PROMOTE_BG),
    std::make_tuple("grpOpenBg",
                    1,
                    TestMachine::Arg("file", 0, 255),
                    SHOULD_PROMOTE_BG),
    std::make_tuple("grpOpenBg",
                    2,
                    TestMachine::Arg("file", 0, 0, 0, 200, 200, 0, 0),
                    SHOULD_PROMOTE_BG),
    std::make_tuple("grpOpenBg",
                    3,
                    TestMachine::Arg("file", 0, 0, 0, 200, 200, 0, 0, 255),
                    SHOULD_PROMOTE_BG),
    std::make_tuple("recOpenBg",
                    0,
                    TestMachine::Arg("file", 0),
                    SHOULD_PROMOTE_BG),
    std::make_tuple("recOpenBg",
                    1,
                    TestMachine::Arg("file", 0, 255),
                    SHOULD_PROMOTE_BG),
    std::make_tuple("recOpenBg",
                    2,
                    TestMachine::Arg("file", 0, 0, 0, 200, 200, 0, 0),
                    SHOULD_PROMOTE_BG),
    std::make_tuple("recOpenBg",
                    3,
                    TestMachine::Arg("file", 0, 0, 0, 200, 200, 0, 0, 255),
                    SHOULD_PROMOTE_BG),
    std::make_tuple("grpMaskOpen",
                    0,
                    TestMachine::Arg("file", 0),
                    SHOULD_PROMOTE_BG),
    std::make_tuple("grpMaskOpen",
                    1,
                    TestMachine::Arg("file", 0, 255),
                    SHOULD_PROMOTE_BG),
    std::make_tuple("grpMaskOpen",
                    2,
                    TestMachine::Arg("file", 0, 0, 0, 200, 200, 0, 0),
                    SHOULD_PROMOTE_BG),
    std::make_tuple("grpMaskOpen",
                    3,
                    TestMachine::Arg("file", 0, 0, 0, 200, 200, 0, 0, 255),
                    SHOULD_PROMOTE_BG),
    std::make_tuple("recMaskOpen",
                    0,
                    TestMachine::Arg("file", 0),
                    SHOULD_PROMOTE_BG),
    std::make_tuple("recMaskOpen",
                    1,
                    TestMachine::Arg("file", 0, 255),
                    SHOULD_PROMOTE_BG),
    std::make_tuple("recMaskOpen",
                    2,
                    TestMachine::Arg("file", 0, 0, 0, 200, 200, 0, 0),
                    SHOULD_PROMOTE_BG),
    std::make_tuple("recMaskOpen",
                    3,
                    TestMachine::Arg("file", 0, 0, 0, 200, 200, 0, 0, 255),
                    SHOULD_PROMOTE_BG),
    std::make_tuple("recMulti",
                    0,
                    TestMachine::Arg("file", 0),
                    SHOULD_PROMOTE_BG),
    std::make_tuple("recMulti",
                    1,
                    TestMachine::Arg("file", 0, 255),
                    SHOULD_PROMOTE_BG),
    std::make_tuple("grpOpen",
                    0,
                    TestMachine::Arg("file", 0),
                    SHOULD_PROMOTE_BG),
    std::make_tuple("grpOpen",
                    1,
                    TestMachine::Arg("file", 0, 255),
                    SHOULD_PROMOTE_BG),
    std::make_tuple("grpOpen",
                    2,
                    TestMachine::Arg("file", 0, 0, 0, 200, 200, 0, 0),
                    SHOULD_PROMOTE_BG),
    std::make_tuple("grpOpen",
                    3,
                    TestMachine::Arg("file", 0, 0, 0, 200, 200, 0, 0, 255),
                    SHOULD_PROMOTE_BG),
    std::make_tuple("recOpen",
                    0,
                    TestMachine::Arg("file", 0),
                    SHOULD_PROMOTE_BG),
    std::make_tuple("recOpen",
                    1,
                    TestMachine::Arg("file", 0, 255),
                    SHOULD_PROMOTE_BG),
    std::make_tuple("recOpen",
                    2,
                    TestMachine::Arg("file", 0, 0, 0, 200, 200, 0, 0),
                    SHOULD_PROMOTE_BG),
    std::make_tuple("recOpen",
                    3,
                    TestMachine::Arg("file", 0, 0, 0, 200, 200, 0, 0, 255),
                    SHOULD_PROMOTE_BG)};

INSTANTIATE_TEST_CASE_P(MediumObjectPoromotion,
                        PromotionTest,
                        ::testing::ValuesIn(data));
