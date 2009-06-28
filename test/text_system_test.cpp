// -*- Mode: C++; tab-width:2; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi:tw=80:et:ts=2:sts=2
//
// -----------------------------------------------------------------------
//
// This file is part of RLVM, a RealLive virtual machine clone.
//
// -----------------------------------------------------------------------
//
// Copyright (C) 2007 Elliot Glaysher
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
//
// -----------------------------------------------------------------------

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "MachineBase/RLMachine.hpp"
#include "LongOperations/TextoutLongOperation.hpp"
#include "NullSystem/NullSystem.hpp"
#include "NullSystem/NullTextWindow.hpp"
#include "Systems/Base/TextPage.hpp"
#include "libReallive/archive.h"

#include "testUtils.hpp"

#include <memory>
#include <boost/assign/list_of.hpp>

using namespace std;

using ::testing::_;

class TextSystemTest : public ::testing::Test {
 protected:
  TextSystemTest()
      : arc(locateTestCase("Module_Str_SEEN/strcpy_0.TXT")),
        system(locateTestCase("Gameexe_data/Gameexe.ini")),
        rlmachine(system, arc) {
    system.text().setActiveWindow(0);
  }

  NullTextSystem& getTextSystem() {
    return dynamic_cast<NullTextSystem&>(system.text());
  }

  NullTextWindow& getTextWindow(int twn) {
    return dynamic_cast<NullTextWindow&>(*system.text().textWindow(twn));
  }

  TextPage& currentPage() {
    return system.text().currentPage();
  }

  void writeString(const std::string& text, bool nowait) {
    auto_ptr<TextoutLongOperation> tolo(
      new TextoutLongOperation(rlmachine, text));

    if (nowait)
      tolo->setNoWait();

    while(!(*tolo)(rlmachine));
  }

  void snapshotAndClear() {
    TextSystem& text = rlmachine.system().text();
    text.snapshot();
    text.textWindow(0)->clearWin();
    text.newPageOnWindow(0);
  }

  // Use any old test case; it isn't getting executed
  libReallive::Archive arc;
  NullSystem system;
  RLMachine rlmachine;
};

TEST_F(TextSystemTest, NormalTextDisplay) {
  writeString("A text string.", false);
  EXPECT_EQ("A text string.", getTextWindow(0).currentContents());
}

TEST_F(TextSystemTest, NormalTextWithNoWait) {
  writeString("A text string.", true);
  EXPECT_EQ("A text string.", getTextWindow(0).currentContents());
}

TEST_F(TextSystemTest, PrintEmptyString) {
  writeString("", false);
  EXPECT_EQ("", getTextWindow(0).currentContents());
}

TEST_F(TextSystemTest, BackLogFunctionality) {
  TextSystem& text = rlmachine.system().text();

  writeString("Page one.", true);
  snapshotAndClear();
  writeString("Page two.", true);
  snapshotAndClear();
  writeString("Page three.", true);
  snapshotAndClear();
  writeString("Page four.", true);

  EXPECT_EQ("Page four.", getTextWindow(0).currentContents())
      << "We're on the final page!";
  EXPECT_FALSE(text.isReadingBacklog()) << "We're not reading the backlog.";

  // Reply our way back to the front
  text.backPage();
  EXPECT_EQ("Page three.", getTextWindow(0).currentContents())
      << "We're on the 3rd page!";
  EXPECT_TRUE(text.isReadingBacklog()) << "We're reading the backlog.";

  text.backPage();
  EXPECT_EQ("Page two.", getTextWindow(0).currentContents())
      << "We're on the 2nd page!";
  EXPECT_TRUE(text.isReadingBacklog()) << "We're reading the backlog.";

  text.backPage();
  EXPECT_EQ("Page one.", getTextWindow(0).currentContents())
      << "We're on the 1st page!";
  EXPECT_TRUE(text.isReadingBacklog()) << "We're reading the backlog.";

  // Trying to go back past the first page doesn't do anything.
  text.backPage();
  EXPECT_EQ("Page one.", getTextWindow(0).currentContents())
      << "We're still on the 1st page!";
  EXPECT_TRUE(text.isReadingBacklog()) << "We're reading the backlog.";

  text.forwardPage();
  EXPECT_EQ("Page two.", getTextWindow(0).currentContents())
      << "We're back to the 2nd page!";
  EXPECT_TRUE(text.isReadingBacklog()) << "We're reading the backlog.";

  text.stopReadingBacklog();
  EXPECT_EQ("Page four.", getTextWindow(0).currentContents())
      << "We're back to the current page!";
  EXPECT_FALSE(text.isReadingBacklog())
      << "We're no longer reading the backlog.";
}

// -----------------------------------------------------------------------

// Tests that the TextPage::name construct repeats correctly.
TEST_F(TextSystemTest, DISABLED_TextPageNameRepeats) {
  NullTextSystem& sys = getTextSystem();
  currentPage().name("Bob", "");
  getTextWindow(0).log().ensure("setName", "Bob", "");
  getTextWindow(0).log().clear();
  snapshotAndClear();

  // Replay it:
  getTextSystem().backPage();
  getTextWindow(0).log().ensure("setName", "Bob", "");
}

// -----------------------------------------------------------------------

// Tests that the TextPgae::hardBreak construct repeats correctly.
TEST_F(TextSystemTest, DISABLED_TextPageHardBreakRepeats) {
  NullTextSystem& sys = getTextSystem();
  currentPage().hardBrake();
  getTextWindow(0).log().ensure("hardBrake");
  getTextWindow(0).log().clear();
  snapshotAndClear();

  // Replay it:
  getTextSystem().backPage();
  getTextWindow(0).log().ensure("hardBrake");
}

// -----------------------------------------------------------------------

// Tests that the TextPage::resetIndentation construct repeats correctly.
TEST_F(TextSystemTest, DISABLED_TextPageResetIndentationRepeats) {
  NullTextSystem& sys = getTextSystem();
  writeString("test", true);
  currentPage().resetIndentation();
  getTextWindow(0).log().ensure("resetIndentation");
  getTextWindow(0).log().clear();
  snapshotAndClear();

  // Replay it:
  getTextSystem().backPage();
  getTextWindow(0).log().ensure("resetIndentation");
}

// -----------------------------------------------------------------------

// Tests that the TextPage::fontColor construct repeats correctly.
TEST_F(TextSystemTest, DISABLED_TextPageFontColorRepeats) {
  NullTextSystem& sys = getTextSystem();
  currentPage().fontColour(0);
  getTextWindow(0).log().ensure("setFontColor");
  getTextWindow(0).log().clear();
  snapshotAndClear();

  // Replay it:
  getTextSystem().backPage();
  getTextWindow(0).log().ensure("setFontColor");
}

// -----------------------------------------------------------------------

// Tests that the ruby constructs repeat correctly.
TEST_F(TextSystemTest, DISABLED_RubyRepeats) {
  NullTextSystem& sys = getTextSystem();
  currentPage().markRubyBegin();
  writeString("With Ruby", true);
  currentPage().displayRubyText("ruby");
  getTextWindow(0).log().ensure("markRubyBegin");
  getTextWindow(0).log().ensure("displayRubyText", "ruby");
  getTextWindow(0).log().clear();
  snapshotAndClear();

  // Replay it:
  getTextSystem().backPage();
  getTextWindow(0).log().ensure("markRubyBegin");
  getTextWindow(0).log().ensure("displayRubyText", "ruby");
}
