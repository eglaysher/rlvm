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
#include "gmock/gmock.h"

#include "libreallive/archive.h"
#include "libreallive/expression.h"
#include "libreallive/intmemref.h"
#include "machine/rlmachine.h"
#include "test_system/mock_surface.h"
#include "test_system/test_system.h"
#include "test_system/test_text_window.h"
#include "utilities/string_utilities.h"

#include "test_utils.h"

#include <boost/algorithm/string.hpp>

#include <string>
#include <vector>

using namespace std;
using namespace std::placeholders;
using namespace boost;

namespace {

// "彼女"
const std::string kGirl = "\xe5\xbd\xbc\xe5\xa5\xb3";

// "「"
const std::string kOpenQuote = "\xe3\x80\x8c";

// "あ" (Hiragana a)
const std::string kHiraganaA = "\xe3\x81\x82";

// "」"
const std::string kCloseQuote = "\xe3\x80\x8d";

// "。"
const std::string kPeriod = "\xe3\x80\x82";

}  // namespace

class TextWindowTest : public ::testing::Test {
 protected:
  TextWindowTest()
      : arc(locateTestCase("Module_Str_SEEN/strcpy_0.TXT")),
        system(),
        rlmachine(system, arc) {}

  void kanonLikeTextbox() {
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
    system.graphics().InjectSurface(
        "name_image",
        std::shared_ptr<Surface>(
            MockSurface::Create("name_image", Size(640, 122))));
  }

  void setGameexeData(const std::string& data) {
    std::vector<std::string> lines;
    boost::split(lines, data, is_any_of("\n"));

    for (std::vector<std::string>::const_iterator it = lines.begin();
         it != lines.end();
         ++it) {
      system.gameexe().parseLine(*it);
    }
  }

  // Use any old test case; it isn't getting executed
  libreallive::Archive arc;
  TestSystem system;
  RLMachine rlmachine;
};

// Tests that a text box like the one in Kanon (origin 0, left aligned, etc)
// has its GetWindowRect() calculated correctly from the gameexe data.
TEST_F(TextWindowTest, KanonLikePositioning) {
  kanonLikeTextbox();

  TestTextWindow window(system, 0);

  EXPECT_EQ(Rect(0, 344, Size(640, 122)), window.GetWindowRect());
}

// Tests that a text box like the one in Princess Brave (origin 2, positioned
// significantly offscreen) has its GetWindowRect() calculated correctly from the
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
  system.graphics().InjectSurface(
      "background",
      std::shared_ptr<Surface>(
          MockSurface::Create("background", Size(800, 300))));

  TestTextWindow window(system, 2);

  EXPECT_EQ(Rect(0, 292, Size(800, 300)), window.GetWindowRect());
}

// Tests normal line breaking by trying to fit a line of "<two kanji name>
// <quote open> 20x<hiragana a> <quote close>" in a Kanon-like
// textbox. There should be a newline before the final 'a'.
TEST_F(TextWindowTest, NormalLineBreaking) {
  kanonLikeTextbox();

  TestTextWindow window(system, 0);

  // "彼女", "「"
  window.SetName(kGirl, kOpenQuote);

  // "「", 20x"あ", "」"
  std::string str = kOpenQuote;
  for (int i = 0; i < 20; ++i)
    str += kHiraganaA;
  str += kCloseQuote;

  PrintTextToFunction(
      bind(&TextWindow::DisplayCharacter, std::ref(window), _1, _2), str, "");

  EXPECT_EQ(window.current_contents(),
            "\xe5\xbd\xbc\xe5\xa5\xb3\xe3\x80\x8c\xe3\x81\x82\xe3\x81\x82\xe3"
            "\x81\x82\xe3\x81\x82\xe3\x81\x82\xe3\x81\x82\xe3\x81\x82\xe3\x81"
            "\x82\xe3\x81\x82\xe3\x81\x82\xe3\x81\x82\xe3\x81\x82\xe3\x81\x82"
            "\xe3\x81\x82\xe3\x81\x82\xe3\x81\x82\xe3\x81\x82\xe3\x81\x82\xe3"
            "\x81\x82\x0a\xe3\x81\x82\xe3\x80\x8d");
}

// Like NormalLineBreaking, but should have 19 Hiragana 'A's to make sure we
// don't call HardBrake() and squeeze the final close quote onto the first
// line.
TEST_F(TextWindowTest, SqueezeOneKinsokuCharacter) {
  kanonLikeTextbox();

  TestTextWindow window(system, 0);

  // "彼女", "「"
  window.SetName(kGirl, kOpenQuote);

  // "「", 19x"あ", "」"
  std::string str = kOpenQuote;
  for (int i = 0; i < 19; ++i)
    str += kHiraganaA;
  str += kCloseQuote;

  PrintTextToFunction(
      bind(&TextWindow::DisplayCharacter, std::ref(window), _1, _2), str, "");

  EXPECT_EQ(window.current_contents(),
            "\xe5\xbd\xbc\xe5\xa5\xb3\xe3\x80\x8c\xe3\x81\x82\xe3\x81\x82\xe3"
            "\x81\x82\xe3\x81\x82\xe3\x81\x82\xe3\x81\x82\xe3\x81\x82\xe3\x81"
            "\x82\xe3\x81\x82\xe3\x81\x82\xe3\x81\x82\xe3\x81\x82\xe3\x81\x82"
            "\xe3\x81\x82\xe3\x81\x82\xe3\x81\x82\xe3\x81\x82\xe3\x81\x82\xe3"
            "\x81\x82\xe3\x80\x8d");
}

// Tests the string "<two character name> <quote open> 19x<random hiragana
// char> <period> <quote close>" in a Kanon-like textbox. This time, we should
// break before the final 'A' because both the <elipsis> and the <close quote>
// are kinsoku characters.
TEST_F(TextWindowTest, MultipleKinsokuCharacters) {
  kanonLikeTextbox();

  TestTextWindow window(system, 0);

  // "彼女", "「"
  window.SetName(kGirl, kOpenQuote);

  // "「", 19x"あ", "」"
  std::string str = kOpenQuote;
  for (int i = 0; i < 19; ++i)
    str += kHiraganaA;
  str += kPeriod;
  str += kCloseQuote;

  PrintTextToFunction(
      bind(&TextWindow::DisplayCharacter, std::ref(window), _1, _2), str, "");

  EXPECT_EQ(window.current_contents(),
            "\xe5\xbd\xbc\xe5\xa5\xb3\xe3\x80\x8c\xe3\x81\x82\xe3\x81\x82\xe3"
            "\x81\x82\xe3\x81\x82\xe3\x81\x82\xe3\x81\x82\xe3\x81\x82\xe3\x81"
            "\x82\xe3\x81\x82\xe3\x81\x82\xe3\x81\x82\xe3\x81\x82\xe3\x81\x82"
            "\xe3\x81\x82\xe3\x81\x82\xe3\x81\x82\xe3\x81\x82\xe3\x81\x82\x0a"
            "\xe3\x81\x82\xe3\x80\x82\xe3\x80\x8d");
}
