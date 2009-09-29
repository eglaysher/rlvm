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
#include "gmock/gmock.h"

#include "MachineBase/RLMachine.hpp"
#include "TestSystem/MockSurface.hpp"
#include "TestSystem/TestSystem.hpp"
#include "TestSystem/TestTextWindow.hpp"
#include "libReallive/archive.h"
#include "libReallive/expression.h"
#include "libReallive/intmemref.h"

#include "testUtils.hpp"

#include <boost/algorithm/string.hpp>

#include <string>
#include <vector>

using namespace boost;

class TextWindowTest : public ::testing::Test {
 protected:
  TextWindowTest()
      : arc(locateTestCase("Module_Str_SEEN/strcpy_0.TXT")),
        system(),
        rlmachine(system, arc) {
  }

  void setGameexeData(const std::string& data) {
    std::vector<std::string> lines;
    boost::split(lines, data, is_any_of("\n"));

    for (std::vector<std::string>::const_iterator it = lines.begin();
         it != lines.end(); ++it) {
      system.gameexe().parseLine(*it);
    }
  }

  // Use any old test case; it isn't getting executed
  libReallive::Archive arc;
  TestSystem system;
  RLMachine rlmachine;
};

// Tests that a text box like the one in Kanon (origin 0, left aligned, etc)
// has its windowRect() calculated correctly from the gameexe data.
TEST_F(TextWindowTest, KanonLikePositioning) {
  const std::string gameexe_data =
      "#SCREENSIZE_MOD=0\n"
      "#WAKU.000.TYPE=5\n"
      "#WAKU.000.000.NAME=\"name_image\"\n"
      "#WAKU.000.000.BACK=\"\"\n"
      "#WAKU.000.000.AREA=0,0,0,0\n"
      "#WAKU.000.000.REP_MOJI_POS=0,0\n"
      "#WAKU.000.000.MOVE_BOX=0:0,0,0,0\n"
      "#WAKU.000.000.CLEAR_BOX=0:0,0,0,0\n"
      "#WAKU.000.000.READJUMP_BOX=0:0,0,0,0\n"
      "#WAKU.000.000.AUTOMODE_BOX=0:0,0,0,0\n"
      "#WAKU.000.000.MSGBK_BOX=0:0,0,0,0\n"
      "#WAKU.000.000.MSGBKLEFT_BOX=0:0,0,0,0\n"
      "#WAKU.000.000.MSGBKRIGHT_BOX=0:0,0,0,0\n"
      "#WINDOW.000.MOJI_CNT=22,3\n"
      "#WINDOW.000.MOJI_REP=0,3\n"
      "#WINDOW.000.MOJI_POS=29,0,53,0\n"
      "#WINDOW.000.POS=0:0,344\n"
      "#WINDOW.000.KEYCUR_MOD=0:0,0\n"
      "#WINDOW.000.WAKU_SETNO=000\n"
      "#WINDOW.000.WAKU_MOD=0\n"
      "#WINDOW.000.WAKU_NO=000\n"
      "#WINDOW.000.ATTR_MOD=1\n"
      "#WINDOW.000.ATTR=010,10,10,255,0\n"
      "#COLOR_TABLE.000=255,255,255\n";

  setGameexeData(gameexe_data);

  // Inject an image for the textbox.
  system.graphics().injectSurface(
      "name_image",
      boost::shared_ptr<Surface>(MockSurface::Create("name_image",
                                                     Size(640, 122))));

  TestTextWindow window(system, 0);

  EXPECT_EQ(Rect(0, 344, Size(640, 122)), window.windowRect());
}

// Tests that a text box like the one in Princess Brave (origin 2, positioned
// significantly offscreen) has its windowRect() calculated correctly from the
// gameexe data.
TEST_F(TextWindowTest, PrincessBraveLikePositioning) {
  const std::string gameexe_data =
      "#SCREENSIZE_MOD=1\n"
      "#WAKU.002.000.NAME=\"background\"\n"
      "#WAKU.002.000.BTN=\"\"\n"
      "#WAKU.002.000.AREA=4,4,4,4\n"
      "#WAKU.002.000.MOVE_BOX=0:0,0,0,0\n"
      "#WAKU.002.000.CLEAR_BOX=0:0,0,0,0\n"
      "#WAKU.002.000.READJUMP_BOX=0:0,0,0,0\n"
      "#WAKU.002.000.AUTOMODE_BOX=0:0,0,0,0\n"
      "#WAKU.002.000.MSGBK_BOX=0:0,0,0,0\n"
      "#WAKU.002.000.MSGBKLEFT_BOX=0:0,0,0,0\n"
      "#WAKU.002.000.MSGBKRIGHT_BOX=0:0,0,0,0\n"
      "#WINDOW.002.MOJI_SIZE=24\n"
      "#WINDOW.002.MOJI_CNT=24,3\n"
      "#WINDOW.002.MOJI_REP=-1,2\n"
      "#WINDOW.002.MOJI_POS=115,0,115,000\n"
      "#WINDOW.002.POS=2:150,8\n"
      "#WINDOW.002.KEYCUR_MOD=0:0,0\n"
      "#WINDOW.002.WAKU_SETNO=002\n"
      "#WINDOW.002.WAKU_MOD=0\n"
      "#WINDOW.002.WAKU_NO=000\n"
      "#WINDOW.002.ATTR_MOD=1\n"
      "#WINDOW.002.ATTR=010,10,10,255,0\n"
      "#COLOR_TABLE.000=255,255,255\n";

  setGameexeData(gameexe_data);

  // Inject an image for the textbox.
  system.graphics().injectSurface(
      "background",
      boost::shared_ptr<Surface>(MockSurface::Create("background",
                                                     Size(800, 300))));

  TestTextWindow window(system, 2);

  EXPECT_EQ(Rect(0, 292, Size(800, 300)), window.windowRect());
}
