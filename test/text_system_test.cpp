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
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
//
// -----------------------------------------------------------------------

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "MachineBase/RLMachine.hpp"
#include "LongOperations/TextoutLongOperation.hpp"
#include "TestSystem/TestSystem.hpp"
#include "TestSystem/MockTextWindow.hpp"
#include "Systems/Base/TextPage.hpp"
#include "libReallive/archive.h"

#include "testUtils.hpp"

#include <string>
#include <memory>
#include <boost/assign/list_of.hpp>

using namespace std;

using ::testing::_;

// Increments everytime you ask for ticks to simulate time going by in the real
// world. Neccessary because textout measures ticks so it knows how fast to
// print out characters.
class IncrementingTickCounter : public EventSystemMockHandler {
 public:
  IncrementingTickCounter() : ticks(0) {}
  virtual unsigned int getTicks() const { return ticks++; }
 private:
  mutable unsigned int ticks;
};

class TextSystemTest : public FullSystemTest {
 protected:
  TextSystemTest() {
    event_mock.reset(new IncrementingTickCounter);
    dynamic_cast<TestEventSystem&>(system.event())
        .setMockHandler(event_mock);

    system.text().setActiveWindow(0);
  }

  TestTextSystem& getTextSystem() {
    return dynamic_cast<TestTextSystem&>(system.text());
  }

  MockTextWindow& getTextWindow(int twn) {
    return dynamic_cast<MockTextWindow&>(*system.text().textWindow(twn));
  }

  TextPage& currentPage() {
    return system.text().currentPage();
  }

  void writeString(const std::string& text, bool nowait) {
    auto_ptr<TextoutLongOperation> tolo(
      new TextoutLongOperation(rlmachine, text));

    if (nowait)
      tolo->setNoWait();

    while (!(*tolo)(rlmachine));
  }

  void snapshotAndClear() {
    TextSystem& text = rlmachine.system().text();
    text.snapshot();
    text.textWindow(0)->clearWin();
    text.newPageOnWindow(0);
  }

  boost::shared_ptr<EventSystemMockHandler> event_mock;
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
TEST_F(TextSystemTest, RepeatsTextPageName) {
  TestTextSystem& sys = getTextSystem();
  MockTextWindow& win = getTextWindow(0);
  EXPECT_CALL(win, setName("Bob", "")).Times(1);
  currentPage().name("Bob", "");
  snapshotAndClear();
  ASSERT_TRUE(::testing::Mock::VerifyAndClearExpectations(&win));

  // Replay it:
  EXPECT_CALL(win, setName("Bob", _)).Times(1);
  getTextSystem().backPage();
  ASSERT_TRUE(::testing::Mock::VerifyAndClearExpectations(&win));
}

// -----------------------------------------------------------------------

// Tests that the TextPgae::hardBreak construct repeats correctly.
TEST_F(TextSystemTest, TextPageHardBreakRepeats) {
  TestTextSystem& sys = getTextSystem();
  MockTextWindow& win = getTextWindow(0);
  EXPECT_CALL(win, hardBrake()).Times(1);
  currentPage().hardBrake();
  snapshotAndClear();
  ASSERT_TRUE(::testing::Mock::VerifyAndClearExpectations(&win));

  // Replay it:
  EXPECT_CALL(win, hardBrake()).Times(1);
  getTextSystem().backPage();
  ASSERT_TRUE(::testing::Mock::VerifyAndClearExpectations(&win));
}

// -----------------------------------------------------------------------

// Tests that the TextPage::resetIndentation construct repeats correctly.
TEST_F(TextSystemTest, TextPageResetIndentationRepeats) {
  TestTextSystem& sys = getTextSystem();
  MockTextWindow& win = getTextWindow(0);
  writeString("test", true);

  EXPECT_CALL(win, resetIndentation()).Times(1);
  currentPage().resetIndentation();
  snapshotAndClear();
  ASSERT_TRUE(::testing::Mock::VerifyAndClearExpectations(&win));

  // Replay it:
  EXPECT_CALL(win, resetIndentation()).Times(1);
  getTextSystem().backPage();
  ASSERT_TRUE(::testing::Mock::VerifyAndClearExpectations(&win));
}

// -----------------------------------------------------------------------

// Tests that the TextPage::fontColor construct repeats correctly.
TEST_F(TextSystemTest, TextPageFontColorRepeats) {
  TestTextSystem& sys = getTextSystem();
  MockTextWindow& win = getTextWindow(0);

  EXPECT_CALL(win, setFontColor(_)).Times(1);
  currentPage().fontColour(0);
  snapshotAndClear();
  ASSERT_TRUE(::testing::Mock::VerifyAndClearExpectations(&win));

  // The scrollback shouldn't be colored.
  EXPECT_CALL(win, setFontColor(_)).Times(0);
  getTextSystem().backPage();
  ASSERT_TRUE(::testing::Mock::VerifyAndClearExpectations(&win));
}

// -----------------------------------------------------------------------

// Tests that the ruby constructs repeat correctly.
TEST_F(TextSystemTest, RubyRepeats) {
  TestTextSystem& sys = getTextSystem();
  MockTextWindow& win = getTextWindow(0);

  EXPECT_CALL(win, markRubyBegin()).Times(1);
  EXPECT_CALL(win, displayRubyText("ruby")).Times(1);
  currentPage().markRubyBegin();
  writeString("With Ruby", true);
  currentPage().displayRubyText("ruby");
  snapshotAndClear();
  ASSERT_TRUE(::testing::Mock::VerifyAndClearExpectations(&win));

  // Replay it:
  EXPECT_CALL(win, markRubyBegin()).Times(1);
  EXPECT_CALL(win, displayRubyText("ruby")).Times(1);
  getTextSystem().backPage();
  ASSERT_TRUE(::testing::Mock::VerifyAndClearExpectations(&win));
}
