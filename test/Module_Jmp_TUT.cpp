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

// -----------------------------------------------------------------------

/**
 * Tests goto_if (if false)
 *
 * Corresponding kepago listing:
 * @code
 * // intB[0] may or maynot be set by the calling program.
 * intA[0] = 1
 * goto_if(intB[0]) @aa
 * intA[1] = 1
 * @aa
 * intA[2] = 1
 * @endcode
 */
template<>
template<>
void object::test<2>()
{
  Reallive::Archive arc("test/Module_Jmp_SEEN/goto_if_0.TXT");
  RLMachine rlmachine(arc);
  rlmachine.attatchModule(new JmpModule);
  rlmachine.executeUntilHalted();

  ensure_equals("Didn't set precondition (!?!?)",
                rlmachine.getIntValue(0, 0),
                1);
  ensure_equals("Didn't set intA[1]; this means the goto-ed when we had a false value!",
                rlmachine.getIntValue(0, 1),
                1);
  ensure_equals("Didn't set postcondition (!?!?)",
                rlmachine.getIntValue(0, 0),
                1);
}

// -----------------------------------------------------------------------

/**
 * Tests goto_if (if true)
 *
 * Corresponding kepago listing:
 * @code
 * // intB[0] may or maynot be set by the calling program.
 * intA[0] = 1
 * goto_if(intB[0]) @aa
 * intA[1] = 1
 * @aa
 * intA[2] = 1
 * @endcode
 */
template<>
template<>
void object::test<3>()
{
  Reallive::Archive arc("test/Module_Jmp_SEEN/goto_if_0.TXT");
  RLMachine rlmachine(arc);
  rlmachine.attatchModule(new JmpModule);
  rlmachine.setIntValue(1, 0, 1);

  rlmachine.executeUntilHalted();

  ensure_equals("Didn't set precondition (!?!?)",
                rlmachine.getIntValue(0, 0),
                1);
  ensure_equals("Set intA[1]; this means that we failed to goto when we had a true value!",
                rlmachine.getIntValue(0, 1),
                0);
  ensure_equals("Didn't set postcondition (!?!?)",
                rlmachine.getIntValue(0, 0),
                1);
}

// -----------------------------------------------------------------------

/**
 * Tests goto_unless (if false)
 *
 * Corresponding kepago listing:
 * @code
 * // intB[0] may or maynot be set by the calling program.
 * intA[0] = 1
 * goto_unless(intB[0]) @aa
 * intA[1] = 1
 * @aa
 * intA[2] = 1
 * @endcode
 */
template<>
template<>
void object::test<4>()
{
  Reallive::Archive arc("test/Module_Jmp_SEEN/goto_unless_0.TXT");
  RLMachine rlmachine(arc);
  rlmachine.attatchModule(new JmpModule);

  rlmachine.executeUntilHalted();

  ensure_equals("Didn't set precondition (!?!?)",
                rlmachine.getIntValue(0, 0),
                1);
  ensure_equals("Set intA[1]; this means that we didn't goto but we had a false val",
                rlmachine.getIntValue(0, 1),
                0);
  ensure_equals("Didn't set postcondition (!?!?)",
                rlmachine.getIntValue(0, 0),
                1);
}

// -----------------------------------------------------------------------


/**
 * Tests goto_unless (if false)
 *
 * Corresponding kepago listing:
 * @code
 * // intB[0] may or maynot be set by the calling program.
 * intA[0] = 1
 * goto_unless(intB[0]) @aa
 * intA[1] = 1
 * @aa
 * intA[2] = 1
 * @endcode
 */
template<>
template<>
void object::test<5>()
{
  Reallive::Archive arc("test/Module_Jmp_SEEN/goto_unless_0.TXT");
  RLMachine rlmachine(arc);
  rlmachine.attatchModule(new JmpModule);
  rlmachine.setIntValue(1, 0, 1);
  rlmachine.executeUntilHalted();

  ensure_equals("Didn't set precondition (!?!?)",
                rlmachine.getIntValue(0, 0),
                1);
  ensure_equals("Set intA[1]; this means that we gotoed when we had a true val",
                rlmachine.getIntValue(0, 1),
                1);
  ensure_equals("Didn't set postcondition (!?!?)",
                rlmachine.getIntValue(0, 0),
                1);
}

// -----------------------------------------------------------------------

/**
 * Tests goto_on (table jumps)
 *
 * Corresponding kepago listing:
 * @code
 * goto_on(intB[0]) { @a , @b , @c }
 * intA[0] = -1
 * goto @end
 *
 * @a
 * intA[0] = 0
 * goto @end
 * 
 * @b
 * intA[0] = 1
 * goto @end
 *
 * @c
 * intA[0] = 2
 * goto @end
 *
 * @end
 * @endcode
 */
template<>
template<>
void object::test<6>()
{
  // Repeat this test to hit each label
  for(int i = 0; i < 3; ++i) 
  {
    Reallive::Archive arc("test/Module_Jmp_SEEN/goto_on_0.TXT");
    RLMachine rlmachine(arc);
    rlmachine.attatchModule(new JmpModule);
    rlmachine.setIntValue(1, 0, i);
    rlmachine.executeUntilHalted();

    ensure_equals("Wrong destination on goto_on",
                  rlmachine.getIntValue(0, 0),
                  i);
  }
}

// -----------------------------------------------------------------------

/**
 * Tests goto_on (with an illegal value)
 *
 * Corresponding kepago listing:
 * @code
 * goto_on(intB[0]) { @a , @b , @c }
 * intA[0] = -1
 * goto @end
 *
 * @a
 * intA[0] = 0
 * goto @end
 * 
 * @b
 * intA[0] = 1
 * goto @end
 *
 * @c
 * intA[0] = 2
 * goto @end
 *
 * @end
 * @endcode
 */
template<>
template<>
void object::test<7>()
{
  Reallive::Archive arc("test/Module_Jmp_SEEN/goto_on_0.TXT");
  RLMachine rlmachine(arc);
  rlmachine.attatchModule(new JmpModule);
  rlmachine.setIntValue(1, 0, 7);
  rlmachine.executeUntilHalted();
  
  ensure_equals("We jumped somewhere unexpected on a bad value!", 
                rlmachine.getIntValue(0, 0),
                -1);
}

// -----------------------------------------------------------------------

/**
 * Tests goto_case
 *
 * Corresponding kepago listing:
 * @code
 * goto_case(intB[0]) { 0: @a; 1: @b; 2: @c; _: @default }
 *
 * @a
 * intA[0] = 0
 * goto @end
 * 
 * @b
 * intA[0] = 1
 * goto @end
 *
 * @c
 * intA[0] = 2
 * goto @end
 *
 * @default
 * intA[0] = 3
 *
 * @end
 * @endcode
 */
template<>
template<>
void object::test<8>()
{
  for(int i = 0; i < 3; ++i) 
  {
    Reallive::Archive arc("test/Module_Jmp_SEEN/goto_case_0.TXT");
    RLMachine rlmachine(arc);
    rlmachine.attatchModule(new JmpModule);
    rlmachine.setIntValue(1, 0, i);
    rlmachine.executeUntilHalted();
  
    ensure_equals("We jumped somewhere unexpected on a bad value!", 
                  rlmachine.getIntValue(0, 0),
                  i);
  }

  // Now test the default value
  Reallive::Archive arc("test/Module_Jmp_SEEN/goto_case_0.TXT");
  RLMachine rlmachine(arc);
  rlmachine.attatchModule(new JmpModule);
  rlmachine.setIntValue(1, 0, 29);
  rlmachine.executeUntilHalted();
  
  ensure_equals("We jumped somewhere unexpected on a bad value!", 
                rlmachine.getIntValue(0, 0),
                3);
}

// -----------------------------------------------------------------------

/**
 * Tests gosub
 *
 * Corresponding kepago listing:
 * @code
 * intA[0] = 1
 * gosub @sub
 * intA[2] = 1
 * goto @end
 *
 * @sub
 * intA[1] = 1
 * ret()
 *
 * // If we fall through, reset the flag
 * intA[1] = 0
 * @end
 * @endcode
 */
template<>
template<>
void object::test<9>()
{
  Reallive::Archive arc("test/Module_Jmp_SEEN/gosub_0.TXT");
  RLMachine rlmachine(arc);
  rlmachine.attatchModule(new JmpModule);
  rlmachine.executeUntilHalted();

  ensure_equals("Didn't set precondition (!?!?)",
                rlmachine.getIntValue(0, 0),
                1);
  ensure_equals("intA[1] is unset; this means the gosub or ret was ignored!",
                rlmachine.getIntValue(0, 1),
                1);
  ensure_equals("Didn't set postcondition (!?!?)",
                rlmachine.getIntValue(0, 0),
                1);
}

// -----------------------------------------------------------------------

/**
 * Tests gosub
 *
 * Corresponding kepago listing:
 * @code
 * intA[0] = 1
 * gosub @sub
 * intA[2] = 1
 * goto @end
 *
 * @sub
 * intA[1] = 1
 * ret()
 *
 * // If we fall through, reset the flag
 * intA[1] = 0
 * @end
 * @endcode
 */
template<>
template<>
void object::test<10>()
{
  Reallive::Archive arc("test/Module_Jmp_SEEN/gosub_0.TXT");
  RLMachine rlmachine(arc);
  rlmachine.attatchModule(new JmpModule);
  rlmachine.executeUntilHalted();

  ensure_equals("Didn't set precondition (!?!?)",
                rlmachine.getIntValue(0, 0),
                1);
  ensure_equals("intA[1] is unset; this means the gosub or ret was ignored!",
                rlmachine.getIntValue(0, 1),
                1);
  ensure_equals("Didn't set postcondition (!?!?)",
                rlmachine.getIntValue(0, 0),
                1);
}

// -----------------------------------------------------------------------

/**
 * Tests gosub_if (if false)
 *
 * Corresponding kepago listing:
 * @code
 * @endcode
 */
template<>
template<>
void object::test<11>()
{
  Reallive::Archive arc("test/Module_Jmp_SEEN/gosub_if_0.TXT");
  RLMachine rlmachine(arc);
  rlmachine.attatchModule(new JmpModule);
  rlmachine.executeUntilHalted();

  ensure_equals("Didn't set precondition (!?!?)",
                rlmachine.getIntValue(0, 0),
                1);
  ensure_equals("Set intA[1]; this means we gosubed on a false value (or ret problem)",
                rlmachine.getIntValue(0, 1),
                0);
  ensure_equals("Didn't set postcondition (!?!?)",
                rlmachine.getIntValue(0, 0),
                1);
}

// -----------------------------------------------------------------------

/**
 * Tests gosub_if (if true)
 *
 * Corresponding kepago listing:
 * @code
 * @endcode
 */
template<>
template<>
void object::test<12>()
{
  Reallive::Archive arc("test/Module_Jmp_SEEN/gosub_if_0.TXT");
  RLMachine rlmachine(arc);
  rlmachine.attatchModule(new JmpModule);
  rlmachine.setIntValue(1, 0, 1);

  rlmachine.executeUntilHalted();

  ensure_equals("Didn't set precondition (!?!?)",
                rlmachine.getIntValue(0, 0),
                1);
  ensure_equals("Didn't set intA[1]; didn't gosub on true!",
                rlmachine.getIntValue(0, 1),
                1);
  ensure_equals("Didn't set postcondition (!?!?)",
                rlmachine.getIntValue(0, 0),
                1);
}



}
