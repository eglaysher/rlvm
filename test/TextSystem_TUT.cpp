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

#include "libReallive/archive.h"
//#include "libReallive/intmemref.h"
#include "MachineBase/RLMachine.hpp"

#include "Modules/TextoutLongOperation.hpp"

#include "NullSystem/NullSystem.hpp"
#include "NullSystem/NullTextWindow.hpp"

#include "Utilities.h"
#include "testUtils.hpp"
#include "tut/tut.hpp"

#include <memory>

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

  NullTextWindow& getTextWindow(int twn)
  {
    return dynamic_cast<NullTextWindow&>(system.text().textWindow(rlmachine, twn));
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
    text.snapshot(rlmachine);
    text.textWindow(rlmachine, 0).clearWin();
    text.newPageOnWindow(rlmachine, 0);
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
  text.backPage(rlmachine);
  ensure_equals("We're on the 3rd page!", getTextWindow(0).currentContents(),
                "Page three.");
  ensure_equals("We're reading the backlog.", text.isReadingBacklog(), true);

  text.backPage(rlmachine);
  ensure_equals("We're on the 2nd page!", getTextWindow(0).currentContents(),
                "Page two.");
  ensure_equals("We're reading the backlog.", text.isReadingBacklog(), true);

  text.backPage(rlmachine);
  ensure_equals("We're on the 1st page!", getTextWindow(0).currentContents(),
                "Page one.");
  ensure_equals("We're reading the backlog.", text.isReadingBacklog(), true);

  // Trying to go back past the first page doesn't do anything.
  text.backPage(rlmachine);
  ensure_equals("We're still on the 1st page!", getTextWindow(0).currentContents(),
                "Page one.");
  ensure_equals("We're reading the backlog.", text.isReadingBacklog(), true);

  text.forwardPage(rlmachine);
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

};
