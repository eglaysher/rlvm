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
 * @file   Module_Jmp_TUT.cpp
 * @author Elliot Glaysher
 * @date   Tue Sep 19 21:27:49 2006
 * @ingroup TestCase
 * 
 * Test cases for the flow control module.
 */

#include "Module_Jmp.hpp"
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
struct Module_Jmp_data
{ 

};

/**
 * This group of declarations is just to register
 * test group in test-application-wide singleton.
 * Name of test group object (auto_ptr_group) shall
 * be unique in tut:: namespace. Alternatively, you
 * you may put it into anonymous namespace.
 */
typedef test_group<Module_Jmp_data> tf;
typedef tf::object object;
tf module_jmp_group("Module_Jmp");

// -----------------------------------------------------------------------

/**
 * Tests goto.
 *
 * Corresponding kepago listing:
 * @code
 * intA[0] = 1;
 * goto @aa
 * intA[1] = 1;
 * @aa
 * intA[2] = 1;
 * @endcode
 */
template<>
template<>
void object::test<1>()
{
  Reallive::Archive arc("test/Module_Jmp_SEEN/goto_0.TXT");
  RLMachine rlmachine(arc);
  rlmachine.attatchModule(new JmpModule);
  rlmachine.executeUntilHalted();

  ensure_equals("Didn't set precondition (!?!?)",
                rlmachine.getIntValue(0, 0),
                1);
  ensure_equals("Set intA[1]; this means the goto was ignored!",
                rlmachine.getIntValue(0, 1),
                0);
  ensure_equals("Didn't set postcondition (!?!?)",
                rlmachine.getIntValue(0, 0),
                1);
}

}
