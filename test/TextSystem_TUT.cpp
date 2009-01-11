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

#include "MachineBase/RLMachine.hpp"
#include "LongOperations/TextoutLongOperation.hpp"
#include "NullSystem/NullSystem.hpp"
#include "NullSystem/NullTextWindow.hpp"
#include "Systems/Base/TextPage.hpp"
#include "libReallive/archive.h"

#include "testUtils.hpp"
#include "tut/tut.hpp"

#include <memory>
#include <boost/assign/list_of.hpp>

using namespace std;

namespace tut
{

struct TextSystem_data
{
  // Use any old test case; it isn't getting executed
  libReallive::Archive arc;
  NullSystem system;
  RLMachine rlmachine;

  TextSystem_data()
    : arc(locateTestCase("Module_Str_SEEN/strcpy_0.TXT")),
      system(locateTestCase("Gameexe_data/Gameexe.ini")),
      rlmachine(system, arc)
  {
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

  void writeString(const std::string& text, bool nowait)
  {
    auto_ptr<TextoutLongOperation> tolo(
      new TextoutLongOperation(rlmachine, text));

    if(nowait)
      tolo->setNoWait();

    while(!(*tolo)(rlmachine));
  }

  void snapshotAndClear()
  {
    TextSystem& text = rlmachine.system().text();
    text.snapshot();
    text.textWindow(0)->clearWin();
    text.newPageOnWindow(0);
  }
};

typedef test_group<TextSystem_data> tf;
typedef tf::object object;
tf TextSystem_data("TextSystem");

/**
 * Tests to make sure we can display a text string while displaying
 * text normally.
 */
template<>
template<>
void object::test<1>()
{
  writeString("A text string.", false);

  // Make sure all the data was printed correctly
  ensure_equals("Data was printed correctly!",
                getTextWindow(0).currentContents(),
                "A text string.");
}

// -----------------------------------------------------------------------

/**
 * Same as <1>, but with nowait enabled
 */
template<>
template<>
void object::test<2>()
{
  writeString("A text string.", true);

  // Make sure all the data was printed correctly
  ensure_equals("Data was printed correctly!",
                getTextWindow(0).currentContents(),
                "A text string.");
}

// -----------------------------------------------------------------------

/**
 * Test the printing of an empty string. (This happens all the time in
 * Planetarian.)
 */
template<>
template<>
void object::test<3>()
{
  writeString("", false);

  // Make sure all the data was printed correctly
  ensure_equals("Data was printed correctly!",
                getTextWindow(0).currentContents(),
                "");
}

// -----------------------------------------------------------------------

/**
 * Test the printing of an empty string. (This happens all the time in
 * Planetarian.)
 */
template<>
template<>
void object::test<4>()
{
  writeString("", true);

  // Make sure all the data was printed correctly
  ensure_equals("Data was printed correctly!",
                getTextWindow(0).currentContents(),
                "");
}

// -----------------------------------------------------------------------

/**
 * Check the functionality of the backlog.
 */
template<>
template<>
void object::test<5>()
{
  TextSystem& text = rlmachine.system().text();

  writeString("Page one.", true);
  snapshotAndClear();
  writeString("Page two.", true);
  snapshotAndClear();
  writeString("Page three.", true);
  snapshotAndClear();
  writeString("Page four.", true);

  ensure_equals("We're on the final page!", getTextWindow(0).currentContents(),
                "Page four.");
  ensure_equals("We're not reading the backlog.", text.isReadingBacklog(), false);

  // Reply our way back to the front
  text.backPage();
  ensure_equals("We're on the 3rd page!", getTextWindow(0).currentContents(),
                "Page three.");
  ensure_equals("We're reading the backlog.", text.isReadingBacklog(), true);

  text.backPage();
  ensure_equals("We're on the 2nd page!", getTextWindow(0).currentContents(),
                "Page two.");
  ensure_equals("We're reading the backlog.", text.isReadingBacklog(), true);

  text.backPage();
  ensure_equals("We're on the 1st page!", getTextWindow(0).currentContents(),
                "Page one.");
  ensure_equals("We're reading the backlog.", text.isReadingBacklog(), true);

  // Trying to go back past the first page doesn't do anything.
  text.backPage();
  ensure_equals("We're still on the 1st page!", getTextWindow(0).currentContents(),
                "Page one.");
  ensure_equals("We're reading the backlog.", text.isReadingBacklog(), true);

  text.forwardPage();
  ensure_equals("We're back to the 2nd page!", getTextWindow(0).currentContents(),
                "Page two.");
  ensure_equals("We're reading the backlog.", text.isReadingBacklog(), true);

  text.stopReadingBacklog();
  ensure_equals("We're back to the current page!",
                getTextWindow(0).currentContents(),
                "Page four.");
  ensure_equals("We're no longer reading the backlog.",
                text.isReadingBacklog(), false);
}

// -----------------------------------------------------------------------

/**
 * Tests that the TextPage::name construct repeats correctly.
 */
template<>
template<>
void object::test<6>()
{
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

/**
 * Tests that the TextPgae::hardBreak construct repeats correctly.
 */
template<>
template<>
void object::test<7>()
{
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

/**
 * Tests that the TextPage::resetIndentation construct repeats correctly.
 */
template<>
template<>
void object::test<8>()
{
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

/**
 * Tests that the TextPage::fontColor construct repeats correctly.
 */
template<>
template<>
void object::test<9>()
{
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

/**
 * Tests that the ruby constructs repeat correctly.
 */
template<>
template<>
void object::test<10>()
{
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

};
