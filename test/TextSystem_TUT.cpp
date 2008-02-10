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
// the Free Software Foundation; either version 2 of the License, or
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
  {}

  NullTextWindow& getTextWindow(int twn)
  {
    return dynamic_cast<NullTextWindow&>(system.text().textWindow(rlmachine, twn));
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
  auto_ptr<TextoutLongOperation> tolo(
    new TextoutLongOperation(rlmachine, "A text string."));
  while(!(*tolo)(rlmachine));

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
  auto_ptr<TextoutLongOperation> tolo(
    new TextoutLongOperation(rlmachine, "A text string."));
  tolo->setNoWait();
  while(!(*tolo)(rlmachine));

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
  auto_ptr<TextoutLongOperation> tolo(
    new TextoutLongOperation(rlmachine, ""));
  while(!(*tolo)(rlmachine));

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
  auto_ptr<TextoutLongOperation> tolo(
    new TextoutLongOperation(rlmachine, ""));
  tolo->setNoWait();
  while(!(*tolo)(rlmachine));

  // Make sure all the data was printed correctly
  ensure_equals("Data was printed correctly!",
                getTextWindow(0).currentContents(),
                "");
}

// -----------------------------------------------------------------------

/**
 * Test printing a simple string
 */
template<>
template<>
void object::test<5>()
{
  auto_ptr<TextoutLongOperation> tolo(
    new TextoutLongOperation(rlmachine, "A simple string"));
  tolo->setNoWait();
  while(!(*tolo)(rlmachine));

  // Make sure all the data was printed correctly
  ensure_equals("Data was printed correctly!",
                getTextWindow(0).currentContents(),
                "A simple string");  
}

// -----------------------------------------------------------------------

};
