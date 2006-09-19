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

/**
 * @file   Module_Mem_TUT.cpp
 * @author Elliot Glaysher
 * @date   Tue Sep 19 07:53:13 2006
 * @ingroup TestCases 
 *
 * Test cases for the memory module
 */

#include "Module_Mem.hpp"
#include "Archive.h"
#include "RLMachine.hpp"

#include "tut.hpp"


#include <iostream>
using namespace std;

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
struct Module_Mem_data
{ 

};

/**
 * This group of declarations is just to register
 * test group in test-application-wide singleton.
 * Name of test group object (auto_ptr_group) shall
 * be unique in tut:: namespace. Alternatively, you
 * you may put it into anonymous namespace.
 */
typedef test_group<Module_Mem_data> tf;
typedef tf::object object;
tf module_mem_group("Module_Mem");

// -----------------------------------------------------------------------

/**
 * Tests setarray_0.
 *
 * Corresponding kepago listing:
 * @code
 * intA[3] = -1
 * setarray(intA[0], 1, 2, 3)
 * @endcode
 */
template<>
template<>
void object::test<1>()
{
  Reallive::Archive arc("test/Module_Mem_SEEN/setarray_0.TXT");
  RLMachine rlmachine(arc);
  rlmachine.attatchModule(new MemModule);
  rlmachine.executeUntilHalted();

  ensure_equals("setarray returned wrong value for intA[0]",
                rlmachine.getIntValue(0, 0),
                1);
  ensure_equals("setarray returned wrong value for intA[1]",
                rlmachine.getIntValue(0, 1),
                2);
  ensure_equals("setarray returned wrong value for intA[2]",
                rlmachine.getIntValue(0, 2),
                3);
  ensure_equals("setarray touched the value in intA[3]!!",
                rlmachine.getIntValue(0, 3),
                -1);
}

// -----------------------------------------------------------------------

/**
 * Tests setarray_0.
 *
 * Corresponding kepago listing:
 * @code
 * intA[4] = -1
 * intA[0] = -1
 * setrng(intA[0], intA[3])
 * @endcode
 */
template<>
template<>
void object::test<2>()
{
  Reallive::Archive arc("test/Module_Mem_SEEN/setrng_0.TXT");
  RLMachine rlmachine(arc);
  rlmachine.attatchModule(new MemModule);
  rlmachine.executeUntilHalted();

  ensure_equals("setrng returned wrong value for intA[0]",
                rlmachine.getIntValue(0, 0),
                0);
  ensure_equals("setrng returned wrong value for intA[1]",
                rlmachine.getIntValue(0, 1),
                0);
  ensure_equals("setrng returned wrong value for intA[2]",
                rlmachine.getIntValue(0, 2),
                0);
  ensure_equals("setrng returned wrong value for intA[3]",
                rlmachine.getIntValue(0, 3),
                0);
  ensure_equals("setrng touched the value in intA[4]!!!",
                rlmachine.getIntValue(0, 4),
                -1);
}

// -----------------------------------------------------------------------

/**
 * Tests setarray_1.
 *
 * Corresponding kepago listing:
 * @code
 * intA[4] = -1
 * intA[0] = -1
 * setrng(intA[0], intA[3], 4)
 * @endcode
 */
template<>
template<>
void object::test<3>()
{
  Reallive::Archive arc("test/Module_Mem_SEEN/setrng_1.TXT");
  RLMachine rlmachine(arc);
  rlmachine.attatchModule(new MemModule);
  rlmachine.executeUntilHalted();

  ensure_equals("setrng returned wrong value for intA[0]",
                rlmachine.getIntValue(0, 0),
                4);
  ensure_equals("setrng returned wrong value for intA[1]",
                rlmachine.getIntValue(0, 1),
                4);
  ensure_equals("setrng returned wrong value for intA[2]",
                rlmachine.getIntValue(0, 2),
                4);
  ensure_equals("setrng returned wrong value for intA[3]",
                rlmachine.getIntValue(0, 3),
                4);
  ensure_equals("setrng touched the value in intA[4]!!!",
                rlmachine.getIntValue(0, 4),
                -1);
}

// -----------------------------------------------------------------------

/**
 * Tests setarray_0.
 *
 * Corresponding kepago listing:
 * @code
 * setarray(intA[0], 1, 2, 3)
 * @endcode
 */
template<>
template<>
void object::test<4>()
{
  Reallive::Archive arc("test/Module_Mem_SEEN/cpyrng_0.TXT");
  RLMachine rlmachine(arc);
  rlmachine.attatchModule(new MemModule);
  rlmachine.executeUntilHalted();

  // First make sure setarray did what we expected it to...
  ensure_equals("setarray returned wrong value for intA[0]",
                rlmachine.getIntValue(0, 0),
                1);
  ensure_equals("setarray returned wrong value for intA[1]",
                rlmachine.getIntValue(0, 1),
                2);
  ensure_equals("setarray returned wrong value for intA[2]",
                rlmachine.getIntValue(0, 2),
                3);

  // Now make sure cpyrng did its job.
  ensure_equals("cpyrng returned wrong value for intB[0]",
                rlmachine.getIntValue(1, 0),
                1);
  ensure_equals("cpyrng returned wrong value for intB[1]",
                rlmachine.getIntValue(1, 1),
                2);
  ensure_equals("cpyrng returned wrong value for intB[2]",
                rlmachine.getIntValue(1, 2),
                3);

}

}
