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
 * @file
 * @ingroup TestCase
 *
 * This test group tests the String module. 
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
tf module_str_group("Module_Str");

// -----------------------------------------------------------------------

/**
 * Tests strcpy_0, which should copy the string valid int strS[0].
 *
 * Corresponding kepago listing:
 * @code
 * strS[0] = "valid"
 * @endcode
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

// -----------------------------------------------------------------------

/** 
 * Tests strcpy_1, which should copy the first 2 chracters into strS[0].
 * 
 * Corresponding kepago listing:
 * @code
 * strcpy(strS[0], "valid", 2)
 * @endcode
 */
template<>
template<>
void object::test<2>()
{
  Reallive::Archive arc("test/seenFiles/strcpy_1.TXT");
  RLMachine rlmachine(arc);
  rlmachine.attatchModule(new StrModule);
  rlmachine.executeUntilHalted();

  string one = rlmachine.getStringValue(0x12, 0);
  ensure_equals("strcpy_1 script failed to set value", one, "va");
}

// -----------------------------------------------------------------------

/** 
 * Tests strclear_0, which should clear the value of a string.
 * 
 * Corresponding kepago listing:
 * @code
 * strS[0] = "valid"
 * strS[1] = "valid"
 * strclear(strS[0])
 * @endcode
 */
template<>
template<>
void object::test<3>()
{
  Reallive::Archive arc("test/seenFiles/strclear_0.TXT");
  RLMachine rlmachine(arc);
  rlmachine.attatchModule(new StrModule);
  rlmachine.executeUntilHalted();

  string one = rlmachine.getStringValue(0x12, 0);
  ensure_equals("strclear_0 script failed to unset value", one, "");

  // We include this check to make sure the machine is sane and that
  // the first assignment works, so strclear doesn't appear to work
  // because assignment failed.
  string two = rlmachine.getStringValue(0x12, 1);
  ensure_equals("strclear_0 script failed to set value", two, "valid");
}

// -----------------------------------------------------------------------

/** 
 * Tests strclear_1, which should clear out a whole range of values.
 * 
 * @code
 * strS[0] = "valid"
 * strS[1] = "valid"
 * strS[2] = "valid"
 * strclear(strS[0], strS[1])
 * @endcode
 */
template<>
template<>
void object::test<4>()
{
  Reallive::Archive arc("test/seenFiles/strclear_1.TXT");
  RLMachine rlmachine(arc);
  rlmachine.attatchModule(new StrModule);
  rlmachine.executeUntilHalted();

  string one = rlmachine.getStringValue(0x12, 0);
  ensure_equals("strclear_1 script failed to unset value", one, "");
  string two = rlmachine.getStringValue(0x12, 1);
  ensure_equals("strclear_1 script failed to unset value", two, "");

  // We include this check to make sure the machine is sane and that
  // the first assignment works, so strclear doesn't appear to work
  // because assignment failed.
  string three = rlmachine.getStringValue(0x12, 2);
  ensure_equals("strclear_1 script failed to set value", three, "valid");
}

// -----------------------------------------------------------------------

/**
 * Tests strcat, which should end up with the string "valid" in strS[0].
 *
 * Corresponding kepago listing:
 * @code
 * strS[0] = "va"
 * strS[0] += "lid"
 * @endcode
 */
template<>
template<>
void object::test<5>()
{
  Reallive::Archive arc("test/seenFiles/strcat_0.TXT");
  RLMachine rlmachine(arc);
  rlmachine.attatchModule(new StrModule);
  rlmachine.executeUntilHalted();

  string one = rlmachine.getStringValue(0x12, 0);
  ensure_equals("strcat script failed to set value", one, "valid");
}

// -----------------------------------------------------------------------

/**
 * Tests strlen. Should be 5, for "vaild"
 *
 * Corresponding kepago listing:
 * @code
 * strS[0] = "valid"
 * intA[0] = strlen(strS[0])
 * @endcode
 */
template<>
template<>
void object::test<6>()
{
  Reallive::Archive arc("test/seenFiles/strlen_0.TXT");
  RLMachine rlmachine(arc);
  rlmachine.attatchModule(new StrModule);
  rlmachine.executeUntilHalted();

  int one = rlmachine.getIntValue(0, 0);
  ensure_equals("strlen script failed to set the strlen(\"valid\") to 5", one, 5);
}

// -----------------------------------------------------------------------

/** 
 * Tests strcmp; make sure that it gives the same values as normal strcmp.
 *
 * Corresponding kepago listing:
 * @code
 * strS[0] = "a"
 * strS[1] = "b"
 * strS[2] = "b"
 * strS[3] = "c"
 * intA[0] = strcmp(strS[0], strS[1])
 * intA[1] = strcmp(strS[1], strS[2])
 * intA[2] = strcmp(strS[2], strS[3])
 * @endcode
 */
template<>
template<>
void object::test<7>()
{
  Reallive::Archive arc("test/seenFiles/strcmp_0.TXT");
  RLMachine rlmachine(arc);
  rlmachine.attatchModule(new StrModule);
  rlmachine.executeUntilHalted();

  ensure_equals("Different values for strcmp(\"a\", \"b\")",
                rlmachine.getIntValue(0, 0),
                strcmp("a", "b"));
  ensure_equals("Different values for strcmp(\"b\", \"b\")",
                rlmachine.getIntValue(0, 1),
                strcmp("b", "b"));
  ensure_equals("Different values for strcmp(\"b\", \"c\")",
                rlmachine.getIntValue(0, 2),
                strcmp("b", "c"));
}

// -----------------------------------------------------------------------



}


