// This file is part of RLVM, a RealLive virutal machine clone.
//
// -----------------------------------------------------------------------
//
// Copyright (C) 2006 El Riot
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

#include "Module_Str.hpp"
#include "Archive.h"
#include "RLMachine.hpp"

#include "tut.hpp"

/**
 * This example test group tests std::auto_ptr implementation.
 * Tests are far from full, of course.
 */
namespace tut
{
  /**
   * Struct which may contain test data members.
   * Test object (class that contains test methods)
   * will inherite from it, so each test method can
   * access members directly.
   *
   * Additionally, for each test, test object is re-created
   * using defaut constructor. Thus, any prepare work can be put
   * into default constructor.
   *
   * Finally, after each test, test object is destroyed independently
   * of test result, so any cleanup work should be located in destructor.
   */
  struct Module_Str_data
  { 

  };

  /**
   * This group of declarations is just to register
   * test group in test-application-wide singleton.
   * Name of test group object (auto_ptr_group) shall
   * be unique in tut:: namespace. Alternatively, you
   * you may put it into anonymous namespace.
   */
  typedef test_group<Module_Str_data> tf;
  typedef tf::object object;
  tf auto_ptr_group("Module_Str");

  /**
   * Checks strcpy_0...
   */
  template<>
  template<>
  void object::test<1>()
  {
    Reallive::Archive arc("test/seenFiles/strcpy_0.TXT");
    RLMachine rlmachine(arc);
    rlmachine.attatchModule(new StrModule);
    rlmachine.executeUntilHalted();

    string one = rlmachine.getStringValue(0x12, 0);
    ensure_equals("strcpy_0 script failed to set value", one, "valid");
  }
}


