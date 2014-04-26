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

#include "libreallive/archive.h"
#include "long_operations/textout_long_operation.h"
#include "machine/rlmachine.h"
#include "systems/base/text_page.h"
#include "test_system/mock_surface.h"
#include "test_system/mock_text_window.h"
#include "test_system/test_system.h"

#include "test_utils.h"

#include <string>
#include <memory>

using namespace std;

using ::testing::_;

// Increments everytime you ask for ticks to simulate time going by in the real
// world. Neccessary because textout measures ticks so it knows how fast to
// print out characters.
class IncrementingTickCounter : public EventSystemMockHandler {
 public:
  IncrementingTickCounter() : ticks(0) {}
  virtual unsigned int GetTicks() const { return ticks++; }

 private:
  mutable unsigned int ticks;
};

class TextSystemTest : public FullSystemTest {
 protected:
  TextSystemTest() {
    event_mock.reset(new IncrementingTickCounter);
    dynamic_cast<TestEventSystem&>(system.event()).SetMockHandler(event_mock);

    system.text().set_active_window(0);
  }

  TestTextSystem& GetTextSystem() {
    return dynamic_cast<TestTextSystem&>(system.text());
  }

  TestGraphicsSystem& GetGraphicsSystem() {
    return dynamic_cast<TestGraphicsSystem&>(system.graphics());
  }

  MockTextWindow& GetTextWindow(int twn) {
    return dynamic_cast<MockTextWindow&>(*system.text().GetTextWindow(twn));
  }

  TextPage& GetCurrentPage() { return system.text().GetCurrentPage(); }

  void WriteString(const std::string& text, bool nowait) {
    unique_ptr<TextoutLongOperation> tolo(
        new TextoutLongOperation(rlmachine, text));

    if (nowait)
      tolo->set_no_wait();

    while (!(*tolo)(rlmachine))
      ;
  }

  void SnapshotAndClear() {
    TextSystem& text = rlmachine.system().text();
    text.Snapshot();
    text.GetTextWindow(0)->ClearWin();
    text.NewPageOnWindow(0);
  }

  std::shared_ptr<EventSystemMockHandler> event_mock;
};

TEST_F(TextSystemTest, NormalTextDisplay) {
  WriteString("A text string.", false);
  EXPECT_EQ("A text string.", GetTextWindow(0).current_contents());
}

TEST_F(TextSystemTest, NormalTextWithNoWait) {
  WriteString("A text string.", true);
  EXPECT_EQ("A text string.", GetTextWindow(0).current_contents());
}

TEST_F(TextSystemTest, PrintEmptyString) {
  WriteString("", false);
  EXPECT_EQ("", GetTextWindow(0).current_contents());
}

TEST_F(TextSystemTest, BackLogFunctionality) {
  TextSystem& text = rlmachine.system().text();

  WriteString("Page one.", true);
  SnapshotAndClear();
  WriteString("Page two.", true);
  SnapshotAndClear();
  WriteString("Page three.", true);
  SnapshotAndClear();
  WriteString("Page four.", true);

  EXPECT_EQ("Page four.", GetTextWindow(0).current_contents())
      << "We're on the final page!";
  EXPECT_FALSE(text.IsReadingBacklog()) << "We're not reading the backlog.";

  // Reply our way back to the front
  text.BackPage();
  EXPECT_EQ("Page three.", GetTextWindow(0).current_contents())
      << "We're on the 3rd page!";
  EXPECT_TRUE(text.IsReadingBacklog()) << "We're reading the backlog.";

  text.BackPage();
  EXPECT_EQ("Page two.", GetTextWindow(0).current_contents())
      << "We're on the 2nd page!";
  EXPECT_TRUE(text.IsReadingBacklog()) << "We're reading the backlog.";

  text.BackPage();
  EXPECT_EQ("Page one.", GetTextWindow(0).current_contents())
      << "We're on the 1st page!";
  EXPECT_TRUE(text.IsReadingBacklog()) << "We're reading the backlog.";

  // Trying to go back past the first page doesn't do anything.
  text.BackPage();
  EXPECT_EQ("Page one.", GetTextWindow(0).current_contents())
      << "We're still on the 1st page!";
  EXPECT_TRUE(text.IsReadingBacklog()) << "We're reading the backlog.";

  text.ForwardPage();
  EXPECT_EQ("Page two.", GetTextWindow(0).current_contents())
      << "We're back to the 2nd page!";
  EXPECT_TRUE(text.IsReadingBacklog()) << "We're reading the backlog.";

  text.StopReadingBacklog();
  EXPECT_EQ("Page four.", GetTextWindow(0).current_contents())
      << "We're back to the current page!";
  EXPECT_FALSE(text.IsReadingBacklog())
      << "We're no longer reading the backlog.";
}

// -----------------------------------------------------------------------

// Tests that the TextPage::name construct repeats correctly.
TEST_F(TextSystemTest, RepeatsTextPageName) {
  TestTextSystem& sys = GetTextSystem();
  MockTextWindow& win = GetTextWindow(0);
  EXPECT_CALL(win, SetName("Bob", "")).Times(1);
  GetCurrentPage().Name("Bob", "");
  SnapshotAndClear();
  ASSERT_TRUE(::testing::Mock::VerifyAndClearExpectations(&win));

  // Replay it:
  EXPECT_CALL(win, SetName("Bob", _)).Times(1);
  GetTextSystem().BackPage();
  ASSERT_TRUE(::testing::Mock::VerifyAndClearExpectations(&win));
}

// -----------------------------------------------------------------------

// Tests that the TextPgae::hardBreak construct repeats correctly.
TEST_F(TextSystemTest, TextPageHardBreakRepeats) {
  TestTextSystem& sys = GetTextSystem();
  MockTextWindow& win = GetTextWindow(0);
  EXPECT_CALL(win, HardBrake()).Times(1);
  GetCurrentPage().HardBrake();
  SnapshotAndClear();
  ASSERT_TRUE(::testing::Mock::VerifyAndClearExpectations(&win));

  // Replay it:
  EXPECT_CALL(win, HardBrake()).Times(1);
  GetTextSystem().BackPage();
  ASSERT_TRUE(::testing::Mock::VerifyAndClearExpectations(&win));
}

// -----------------------------------------------------------------------

// Tests that the TextPage::ResetIndentation construct repeats correctly.
TEST_F(TextSystemTest, TextPageResetIndentationRepeats) {
  TestTextSystem& sys = GetTextSystem();
  MockTextWindow& win = GetTextWindow(0);
  WriteString("test", true);

  EXPECT_CALL(win, ResetIndentation()).Times(1);
  GetCurrentPage().ResetIndentation();
  SnapshotAndClear();
  ASSERT_TRUE(::testing::Mock::VerifyAndClearExpectations(&win));

  // Replay it:
  EXPECT_CALL(win, ResetIndentation()).Times(1);
  GetTextSystem().BackPage();
  ASSERT_TRUE(::testing::Mock::VerifyAndClearExpectations(&win));
}

// -----------------------------------------------------------------------

// Tests that the TextPage::fontColor construct repeats correctly.
TEST_F(TextSystemTest, TextPageFontColorRepeats) {
  TestTextSystem& sys = GetTextSystem();
  MockTextWindow& win = GetTextWindow(0);

  EXPECT_CALL(win, SetFontColor(_)).Times(1);
  GetCurrentPage().FontColour(0);
  SnapshotAndClear();
  ASSERT_TRUE(::testing::Mock::VerifyAndClearExpectations(&win));

  // The scrollback shouldn't be colored.
  EXPECT_CALL(win, SetFontColor(_)).Times(0);
  GetTextSystem().BackPage();
  ASSERT_TRUE(::testing::Mock::VerifyAndClearExpectations(&win));
}

// -----------------------------------------------------------------------

// Tests that the ruby constructs repeat correctly.
TEST_F(TextSystemTest, RubyRepeats) {
  TestTextSystem& sys = GetTextSystem();
  MockTextWindow& win = GetTextWindow(0);

  EXPECT_CALL(win, MarkRubyBegin()).Times(1);
  EXPECT_CALL(win, DisplayRubyText("ruby")).Times(1);
  GetCurrentPage().MarkRubyBegin();
  WriteString("With Ruby", true);
  GetCurrentPage().DisplayRubyText("ruby");
  SnapshotAndClear();
  ASSERT_TRUE(::testing::Mock::VerifyAndClearExpectations(&win));

  // Replay it:
  EXPECT_CALL(win, MarkRubyBegin()).Times(1);
  EXPECT_CALL(win, DisplayRubyText("ruby")).Times(1);
  GetTextSystem().BackPage();
  ASSERT_TRUE(::testing::Mock::VerifyAndClearExpectations(&win));
}

// -----------------------------------------------------------------------

TEST_F(TextSystemTest, RenderGlyphOntoOneLine) {
  TestTextSystem& sys = GetTextSystem();
  std::shared_ptr<Surface> text_surface =
      sys.RenderText("One", 20, 0, 0, RGBColour::White(), NULL, 3);
  // Ensure that when the number of characters equals the max number of
  // characters, we only use one line.
  EXPECT_EQ(20, text_surface->GetSize().height());
}

TEST_F(TextSystemTest, RenderGlyphNoRestriction) {
  TestTextSystem& sys = GetTextSystem();
  std::shared_ptr<Surface> text_surface =
      sys.RenderText("A Very Long String That Goes On And On",
                     20,
                     0,
                     0,
                     RGBColour::White(),
                     NULL,
                     0);
  // Ensure that when the number of characters equals the max number of
  // characters, we only use one line.
  EXPECT_EQ(20, text_surface->GetSize().height());
}

TEST_F(TextSystemTest, RenderGlyphOntoTwoLines) {
  TestTextSystem& sys = GetTextSystem();
  std::shared_ptr<Surface> text_surface =
      sys.RenderText("OneTwo", 20, 0, 0, RGBColour::White(), NULL, 3);
  EXPECT_EQ(40, text_surface->GetSize().height());

  // Tests the location of rendered glyphs.
  struct {
    const char* str;
    int xpos;
    int ypos;
  } test_data[] = {{"O", 0, 0},
                   {"n", 20, 0},
                   {"e", 40, 0},
                   {"T", 0, 20},
                   {"w", 20, 20},
                   {"o", 40, 20}};

  std::vector<std::tuple<std::string, int, int>> data = sys.glyphs();
  ASSERT_EQ(6, data.size());
  for (int i = 0; i < data.size(); ++i) {
    EXPECT_EQ(test_data[i].str, get<0>(data[i]));
    EXPECT_EQ(test_data[i].xpos, get<1>(data[i]));
    EXPECT_EQ(test_data[i].ypos, get<2>(data[i]));
  }
}

TEST_F(TextSystemTest, DontCrashWithNoEmojiFile) {
  TestTextSystem& sys = GetTextSystem();
  std::shared_ptr<Surface> text_surface =
      sys.RenderText("One＃Ａ００Two", 20, 0, 0, RGBColour::White(), NULL, -1);
  EXPECT_EQ(20, text_surface->GetSize().height());
}

TEST_F(TextSystemTest, TestEmoji) {
  // Set up emoji printing:
  system.gameexe().SetStringAt("E_MOJI.004", "emoji_file");

  // Inject a fake surface of (24*5, 24). We will expect this to blit to the
  // text surface.
  std::shared_ptr<MockSurface> mock(
      MockSurface::Create("emoji_file", Size(24 * 5, 24)));
  GetGraphicsSystem().InjectSurface("emoji_file", mock);

  // Our mock surface should render its icon between the Es.
  EXPECT_CALL(*mock,
              BlitToSurface(_,
                            Rect(24 * 2, 0, Size(24, 24)),
                            Rect(20, 0, Size(24, 24)),
                            255,
                            false));

  TestTextSystem& sys = GetTextSystem();
  std::shared_ptr<Surface> text_surface =
      sys.RenderText("E＃Ａ０２E", 20, 0, 0, RGBColour::White(), NULL, -1);
  EXPECT_EQ(20, text_surface->GetSize().height());

  // Tests the location of rendered glyphs.
  struct {
    const char* str;
    int xpos;
    int ypos;
  } test_data[] = {{"E", 0, 0}, {"E", 40, 0}, };

  std::vector<std::tuple<std::string, int, int>> data = sys.glyphs();
  ASSERT_EQ(2, data.size());
  for (int i = 0; i < data.size(); ++i) {
    EXPECT_EQ(test_data[i].str, get<0>(data[i]));
    EXPECT_EQ(test_data[i].xpos, get<1>(data[i]));
    EXPECT_EQ(test_data[i].ypos, get<2>(data[i]));
  }
}

// If we return an empty surface, we crash. Make sure passing an empty string
// doesn't return an empty surface.
TEST_F(TextSystemTest, TestEmptyString) {
  TestTextSystem& sys = GetTextSystem();
  std::shared_ptr<Surface> text_surface =
      sys.RenderText("", 20, 0, 0, RGBColour::White(), NULL, -1);
  EXPECT_GT(text_surface->GetSize().width(), 0);
  EXPECT_GT(text_surface->GetSize().height(), 0);
}
