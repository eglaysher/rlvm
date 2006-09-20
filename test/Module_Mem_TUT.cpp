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
// -----------------------------------------------------------------------

/**
 * Tests setarray_stepped_0.
 *
 * Corresponding kepago listing:
 * @code
 * setrng(intA[0], intA[5], -1)
 * setarray_stepped(intA[0], 2, 1, 2, 3)
 * @endcode
 */
template<>
template<>
void object::test<5>()
{
  Reallive::Archive arc("test/Module_Mem_SEEN/setarray_stepped_0.TXT");
  RLMachine rlmachine(arc);
  rlmachine.attatchModule(new MemModule);
  rlmachine.executeUntilHalted();

  // First make sure setarray_stepped did what we expected it to, and that it
  // didn't overwrite the work of setrng
  ensure_equals("setarray_stepped returned wrong value for intA[0]",
                rlmachine.getIntValue(0, 0),
                1);
  ensure_equals("setarray_stepped touched the value of intA[1]",
                rlmachine.getIntValue(0, 1),
                -1);
  ensure_equals("setarray_stepped returned wrong value for intA[2]",
                rlmachine.getIntValue(0, 2),
                2);
  ensure_equals("setarray_stepped touched the value for intA[3]",
                rlmachine.getIntValue(0, 3),
                -1);
  ensure_equals("setarray_stepped returned wrong value for intA[4]",
                rlmachine.getIntValue(0, 4),
                3);
  ensure_equals("setarray_stepped touched the value for intA[5]",
                rlmachine.getIntValue(0, 5),
                -1);
}

// -----------------------------------------------------------------------

/**
 * Tests setrng_stepped_0.
 *
 * Corresponding kepago listing:
 * @code
 * setrng(intA[0], intA[5], -1)
 * setrng_stepped(intA[0], 2, 3)
 * @endcode
 */
template<>
template<>
void object::test<6>()
{
  Reallive::Archive arc("test/Module_Mem_SEEN/setrng_stepped_0.TXT");
  RLMachine rlmachine(arc);
  rlmachine.attatchModule(new MemModule);
  rlmachine.executeUntilHalted();

  // First make sure setrng_stepped did what we expected it to, and that it
  // didn't overwrite the work of setrng
  ensure_equals("setrng_stepped returned wrong value for intA[0]",
                rlmachine.getIntValue(0, 0),
                0);
  ensure_equals("setrng_stepped touched the value of intA[1]",
                rlmachine.getIntValue(0, 1),
                -1);
  ensure_equals("setrng_stepped returned wrong value for intA[2]",
                rlmachine.getIntValue(0, 2),
                0);
  ensure_equals("setrng_stepped touched the value for intA[3]",
                rlmachine.getIntValue(0, 3),
                -1);
  ensure_equals("setrng_stepped returned wrong value for intA[4]",
                rlmachine.getIntValue(0, 4),
                0);
  ensure_equals("setrng_stepped touched the value for intA[5]",
                rlmachine.getIntValue(0, 5),
                -1);
}

// -----------------------------------------------------------------------

/**
 * Tests setrng_stepped_1.
 *
 * Corresponding kepago listing:
 * @code
 * setrng(intA[0], intA[5], -1)
 * setrng_stepped(intA[0], 2, 3, 5)
 * @endcode
 */
template<>
template<>
void object::test<7>()
{
  Reallive::Archive arc("test/Module_Mem_SEEN/setrng_stepped_1.TXT");
  RLMachine rlmachine(arc);
  rlmachine.attatchModule(new MemModule);
  rlmachine.executeUntilHalted();

  // First make sure setrng_stepped did what we expected it to, and that it
  // didn't overwrite the work of setrng
  ensure_equals("setrng_stepped returned wrong value for intA[0]",
                rlmachine.getIntValue(0, 0),
                5);
  ensure_equals("setrng_stepped touched the value of intA[1]",
                rlmachine.getIntValue(0, 1),
                -1);
  ensure_equals("setrng_stepped returned wrong value for intA[2]",
                rlmachine.getIntValue(0, 2),
                5);
  ensure_equals("setrng_stepped touched the value for intA[3]",
                rlmachine.getIntValue(0, 3),
                -1);
  ensure_equals("setrng_stepped returned wrong value for intA[4]",
                rlmachine.getIntValue(0, 4),
                5);
  ensure_equals("setrng_stepped touched the value for intA[5]",
                rlmachine.getIntValue(0, 5),
                -1);
}

// -----------------------------------------------------------------------

/**
 * Tests cpyvars.
 *
 * Corresponding kepago listing:
 * @code
 * intB[3] = 5
 * intB[5] = 1
 * intB[8] = 2
 * cpyvars(intA[0], 2, intB[1], intB[3], intB[6])
 * @endcode
 */
template<>
template<>
void object::test<8>()
{
  Reallive::Archive arc("test/Module_Mem_SEEN/cpyvars_0.TXT");
  RLMachine rlmachine(arc);
  rlmachine.attatchModule(new MemModule);
  rlmachine.executeUntilHalted();

  // First make sure cpyvars did what we expected it to...
  ensure_equals("cpyvars set wrong value for intA[0]",
                rlmachine.getIntValue(0, 0),
                5);
  ensure_equals("cpyvars set wrong value for intA[1]",
                rlmachine.getIntValue(0, 1),
                1);
  ensure_equals("cpyvars set wrong value for intA[2]",
                rlmachine.getIntValue(0, 2),
                2);
}

// -----------------------------------------------------------------------

/**
 * Tests sum.
 *
 * Corresponding kepago listing:
 * @code
 * intA[0] = 0
 * intA[1] = 1
 * intA[2] = 2
 * intA[3] = 3
 * intA[10] = sum(intA[0], intA[3])
 * @endcode
 */
template<>
template<>
void object::test<9>()
{
  Reallive::Archive arc("test/Module_Mem_SEEN/sum_0.TXT");
  RLMachine rlmachine(arc);
  rlmachine.attatchModule(new MemModule);
  rlmachine.executeUntilHalted();

  // First make sure sum did what we expected it to...
  ensure_equals("sum returned the wrong value for intA[10]",
                rlmachine.getIntValue(0, 10),
                6);
}

// -----------------------------------------------------------------------

/**
 * Tests sum.
 *
 * Corresponding kepago listing:
 * @code
 * intA[0] = 0
 * intA[1] = 1
 * intA[2] = 2
 * intA[3] = 3
 * intA[10] = sum(intA[0], intA[3])
 * @endcode
 */
template<>
template<>
void object::test<10>()
{
  Reallive::Archive arc("test/Module_Mem_SEEN/sums_0.TXT");
  RLMachine rlmachine(arc);
  rlmachine.attatchModule(new MemModule);
  rlmachine.executeUntilHalted();

  // First make sure sums did what we expected it to...
  ensure_equals("sums returned the wrong value for intA[0]",
                rlmachine.getIntValue(0, 0),
                45);
}

// -----------------------------------------------------------------------

}
