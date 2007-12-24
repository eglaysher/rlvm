// -*- Mode: C++; tab-width:2; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi:tw=80:et:ts=2:sts=2
//
// -----------------------------------------------------------------------
//
// This file is part of RLVM, a RealLive virtual machine clone.
//
// -----------------------------------------------------------------------
//
// Copyright (C) 2006, 2007 Elliot Glaysher
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

#include "Modules/Module_Jmp.hpp"
#include "Modules/Module_Str.hpp"
#include "Modules/Module_Msg.hpp"
#include "libReallive/archive.h"
#include "libReallive/intmemref.h"
#include "MachineBase/RLMachine.hpp"

#include "NullSystem/NullSystem.hpp"

#include "testUtils.hpp"
#include "tut.hpp"

//#include "SDLSystem.hpp"
//#include "SDLGraphicsSystem.hpp"

#include <iostream>
using namespace std;
using namespace libReallive;

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
  libReallive::Archive arc(locateTestCase("Module_Jmp_SEEN/goto_0.TXT"));
  NullSystem system;
  RLMachine rlmachine(system, arc);
  rlmachine.attachModule(new JmpModule);
  rlmachine.executeUntilHalted();

  ensure_equals("Didn't set precondition (!?!?)",
                rlmachine.getIntValue(IntMemRef('A', 0)),
                1);
  ensure_equals("Set intA[1]; this means the goto was ignored!",
                rlmachine.getIntValue(IntMemRef('A', 1)),
                0);
  ensure_equals("Didn't set postcondition (!?!?)",
                rlmachine.getIntValue(IntMemRef('A', 0)),
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
  libReallive::Archive arc(locateTestCase("Module_Jmp_SEEN/goto_if_0.TXT"));
  NullSystem system;
  RLMachine rlmachine(system, arc);
  rlmachine.attachModule(new JmpModule);
  rlmachine.executeUntilHalted();

  ensure_equals("Didn't set precondition (!?!?)",
                rlmachine.getIntValue(IntMemRef('A', 0)),
                1);
  ensure_equals("Didn't set intA[1]; this means the goto-ed when we had a false value!",
                rlmachine.getIntValue(IntMemRef('A', 1)),
                1);
  ensure_equals("Didn't set postcondition (!?!?)",
                rlmachine.getIntValue(IntMemRef('A', 0)),
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
  libReallive::Archive arc(locateTestCase("Module_Jmp_SEEN/goto_if_0.TXT"));
  NullSystem system;
  RLMachine rlmachine(system, arc);
  rlmachine.attachModule(new JmpModule);
  rlmachine.setIntValue(IntMemRef('B', 0), 1);

  rlmachine.executeUntilHalted();

  ensure_equals("Didn't set precondition (!?!?)",
                rlmachine.getIntValue(IntMemRef('A', 0)),
                1);
  ensure_equals("Set intA[1]; this means that we failed to goto when we had a true value!",
                rlmachine.getIntValue(IntMemRef('A', 1)),
                0);
  ensure_equals("Didn't set postcondition (!?!?)",
                rlmachine.getIntValue(IntMemRef('A', 0)),
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
  libReallive::Archive arc(locateTestCase("Module_Jmp_SEEN/goto_unless_0.TXT"));
  NullSystem system;
  RLMachine rlmachine(system, arc);
  rlmachine.attachModule(new JmpModule);

  rlmachine.executeUntilHalted();

  ensure_equals("Didn't set precondition (!?!?)",
                rlmachine.getIntValue(IntMemRef('A', 0)),
                1);
  ensure_equals("Set intA[1]; this means that we didn't goto but we had a false val",
                rlmachine.getIntValue(IntMemRef('A', 1)),
                0);
  ensure_equals("Didn't set postcondition (!?!?)",
                rlmachine.getIntValue(IntMemRef('A', 0)),
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
  libReallive::Archive arc(locateTestCase("Module_Jmp_SEEN/goto_unless_0.TXT"));
  NullSystem system;
  RLMachine rlmachine(system, arc);
  rlmachine.attachModule(new JmpModule);
  rlmachine.setIntValue(IntMemRef('B', 0), 1);
  rlmachine.executeUntilHalted();

  ensure_equals("Didn't set precondition (!?!?)",
                rlmachine.getIntValue(IntMemRef('A', 0)),
                1);
  ensure_equals("Set intA[1]; this means that we gotoed when we had a true val",
                rlmachine.getIntValue(IntMemRef('A', 1)),
                1);
  ensure_equals("Didn't set postcondition (!?!?)",
                rlmachine.getIntValue(IntMemRef('A', 0)),
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
    libReallive::Archive arc(locateTestCase("Module_Jmp_SEEN/goto_on_0.TXT"));
    NullSystem system;
    RLMachine rlmachine(system, arc);
    rlmachine.attachModule(new JmpModule);
    rlmachine.setIntValue(IntMemRef('B', 0), i);
    rlmachine.executeUntilHalted();

    ensure_equals("Wrong destination on goto_on",
                  rlmachine.getIntValue(IntMemRef('A', 0)),
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
  libReallive::Archive arc(locateTestCase("Module_Jmp_SEEN/goto_on_0.TXT"));
  NullSystem system(locateTestCase("Gameexe_data/Gameexe.ini"));
  RLMachine rlmachine(system, arc);
  rlmachine.attachModule(new JmpModule);
  rlmachine.setIntValue(IntMemRef('B', 0), 7);
  rlmachine.executeUntilHalted();
  
  ensure_equals("We jumped somewhere unexpected on a bad value!", 
                rlmachine.getIntValue(IntMemRef('A', 0)),
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
    libReallive::Archive arc(locateTestCase("Module_Jmp_SEEN/goto_case_0.TXT"));
    NullSystem system;
    RLMachine rlmachine(system, arc);
    rlmachine.attachModule(new JmpModule);
    rlmachine.setIntValue(IntMemRef('B', 0), i);
    rlmachine.executeUntilHalted();
  
    ensure_equals("We jumped somewhere unexpected on a bad value!", 
                  rlmachine.getIntValue(IntMemRef('A', 0)),
                  i);
  }

  // Now test the default value
  libReallive::Archive arc(locateTestCase("Module_Jmp_SEEN/goto_case_0.TXT"));
  NullSystem system;
  RLMachine rlmachine(system, arc);
  rlmachine.attachModule(new JmpModule);
  rlmachine.setIntValue(IntMemRef('B', 0), 29);
  rlmachine.executeUntilHalted();
  
  ensure_equals("We jumped somewhere unexpected on a bad value!", 
                rlmachine.getIntValue(IntMemRef(0, 0)),
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
  libReallive::Archive arc(locateTestCase("Module_Jmp_SEEN/gosub_0.TXT"));
  NullSystem system;
  RLMachine rlmachine(system, arc);
  rlmachine.attachModule(new JmpModule);
  rlmachine.executeUntilHalted();

  ensure_equals("Didn't set precondition (!?!?)",
                rlmachine.getIntValue(IntMemRef('A', 0)),
                1);
  ensure_equals("intA[1] is unset; this means the gosub or ret was ignored!",
                rlmachine.getIntValue(IntMemRef('A', 1)),
                1);
  ensure_equals("Didn't set postcondition (!?!?)",
                rlmachine.getIntValue(IntMemRef('A', 0)),
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
  libReallive::Archive arc(locateTestCase("Module_Jmp_SEEN/gosub_0.TXT"));
  NullSystem system;
  RLMachine rlmachine(system, arc);
  rlmachine.attachModule(new JmpModule);
  rlmachine.executeUntilHalted();

  ensure_equals("Didn't set precondition (!?!?)",
                rlmachine.getIntValue(IntMemRef('A', 0)),
                1);
  ensure_equals("intA[1] is unset; this means the gosub or ret was ignored!",
                rlmachine.getIntValue(IntMemRef('A', 1)),
                1);
  ensure_equals("Didn't set postcondition (!?!?)",
                rlmachine.getIntValue(IntMemRef('A', 0)),
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
  libReallive::Archive arc(locateTestCase("Module_Jmp_SEEN/gosub_if_0.TXT"));
  NullSystem system;
  RLMachine rlmachine(system, arc);
  rlmachine.attachModule(new JmpModule);
  rlmachine.executeUntilHalted();

  ensure_equals("Didn't set precondition (!?!?)",
                rlmachine.getIntValue(IntMemRef('A', 0)),
                1);
  ensure_equals("Set intA[1]; this means we gosubed on a false value (or ret problem)",
                rlmachine.getIntValue(IntMemRef('A', 1)),
                0);
  ensure_equals("Didn't set postcondition (!?!?)",
                rlmachine.getIntValue(IntMemRef('A', 0)),
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
  libReallive::Archive arc(locateTestCase("Module_Jmp_SEEN/gosub_if_0.TXT"));
  NullSystem system;
  RLMachine rlmachine(system, arc);
  rlmachine.attachModule(new JmpModule);
  rlmachine.setIntValue(IntMemRef('B', 0), 1);

  rlmachine.executeUntilHalted();

  ensure_equals("Didn't set precondition (!?!?)",
                rlmachine.getIntValue(IntMemRef('A', 0)),
                1);
  ensure_equals("Didn't set intA[1]; didn't gosub on true!",
                rlmachine.getIntValue(IntMemRef('A', 1)),
                1);
  ensure_equals("Didn't set postcondition (!?!?)",
                rlmachine.getIntValue(IntMemRef('A', 0)),
                1);
}

// -----------------------------------------------------------------------


/**
 * Tests gosub_unless (if false)
 *
 * Corresponding kepago listing:
 * @code
 * intA[0] = 1
 * gosub_unless(intB[0]) @sub
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
void object::test<13>()
{
  libReallive::Archive arc(locateTestCase("Module_Jmp_SEEN/gosub_unless_0.TXT"));
  NullSystem system;
  RLMachine rlmachine(system, arc);
  rlmachine.attachModule(new JmpModule);
  rlmachine.executeUntilHalted();

  ensure_equals("Didn't set precondition (!?!?)",
                rlmachine.getIntValue(IntMemRef('A', 0)),
                1);
  ensure_equals("Didn't set intA[1]; didn't gosub on false!",
                rlmachine.getIntValue(IntMemRef('A', 1)),
                1);
  ensure_equals("Didn't set postcondition (!?!?)",
                rlmachine.getIntValue(IntMemRef('A', 0)),
                1);
}

// -----------------------------------------------------------------------

/**
 * Tests gosub_unless (if true)
 *
 * Corresponding kepago listing:
 * @code
 * intA[0] = 1
 * gosub_unless(intB[0]) @sub
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
void object::test<14>()
{
  libReallive::Archive arc(locateTestCase("Module_Jmp_SEEN/gosub_unless_0.TXT"));
  NullSystem system;
  RLMachine rlmachine(system, arc);
  rlmachine.attachModule(new JmpModule);
  rlmachine.setIntValue(IntMemRef('B', 0), 1);

  rlmachine.executeUntilHalted();

  ensure_equals("Didn't set precondition (!?!?)",
                rlmachine.getIntValue(IntMemRef('A', 0)),
                1);
  ensure_equals("Set intA[1]; this means we gosubed on a true value (or ret problem)",
                rlmachine.getIntValue(IntMemRef('A', 1)),
                0);
  ensure_equals("Didn't set postcondition (!?!?)",
                rlmachine.getIntValue(IntMemRef('A', 0)),
                1);
}

// -----------------------------------------------------------------------

/**
 * Tests gosub_case
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
void object::test<15>()
{
  for(int i = 0; i < 3; ++i) 
  {
    libReallive::Archive arc(locateTestCase("Module_Jmp_SEEN/gosub_case_0.TXT"));
    NullSystem system(locateTestCase("Gameexe_data/Gameexe.ini"));
    RLMachine rlmachine(system, arc);
    rlmachine.attachModule(new JmpModule);
    rlmachine.setIntValue(IntMemRef('B', 0), i);
    rlmachine.executeUntilHalted();
  
    ensure_equals("We jumped somewhere unexpected on a bad value!", 
                  rlmachine.getIntValue(IntMemRef('A', 0)),
                  i);
  }

  // Now test the default value
  libReallive::Archive arc(locateTestCase("Module_Jmp_SEEN/gosub_case_0.TXT"));
  NullSystem system(locateTestCase("Gameexe_data/Gameexe.ini"));
  RLMachine rlmachine(system, arc);
  rlmachine.attachModule(new JmpModule);
  rlmachine.setIntValue(IntMemRef('B', 0), 29);
  rlmachine.executeUntilHalted();
  
  ensure_equals("We jumped somewhere unexpected on a bad value!", 
                rlmachine.getIntValue(IntMemRef('A', 0)),
                3);
}

// -----------------------------------------------------------------------

/**
 * Tests jump (within current file). In this test, we use entrypoints
 * like goto labels.
 *
 * Corresponding kepago listing:
 * @code
 * jump(1, intB[0]);
 *
 * #ENTRYPOINT 1
 * intA[0] = 1
 * goto @end
 * 
 * #ENTRYPOINT 2
 * intA[0] = 2
 * goto @end
 *
 * #ENTRYPOINT 3
 * intA[0] = 3
 * goto @end
 *
 * @end
 * @endcode
 */
template<>
template<>
void object::test<16>()
{
  for(int i = 1; i < 4; ++i) 
  {
    libReallive::Archive arc(locateTestCase("Module_Jmp_SEEN/jump_0.TXT"));
    NullSystem system;
    RLMachine rlmachine(system, arc);
    rlmachine.attachModule(new JmpModule);
    rlmachine.setIntValue(IntMemRef('B', 0), i);
    rlmachine.executeUntilHalted();
  
    ensure_equals("We jumped somewhere unexpected on a bad value!", 
                  rlmachine.getIntValue(IntMemRef('A', 0)),
                  i);
  }
}

// -----------------------------------------------------------------------

/** 
 * 
 * @code
 * // ------------- In seen00001
 * jump(2, intB[0])
 *
 * // ------------- In seen00002
 * #ENTRYPOINT 1
 * intA[0] = 1
 * goto @end
 * 
 * #ENTRYPOINT 2
 * intA[0] = 2
 * goto @end
 *
 * #ENTRYPOINT 3
 * intA[0] = 3
 * goto @end
 *
 * @end
 * @endcode
 */
template<>
template<>
void object::test<17>()
{
  for(int i = 1; i < 4; ++i) 
  {
    libReallive::Archive arc(locateTestCase("Module_Jmp_SEEN/jumpTest/SEEN.TXT"));
    NullSystem system;
    RLMachine rlmachine(system, arc);
    rlmachine.attachModule(new JmpModule);
    rlmachine.setIntValue(IntMemRef('B', 0), i);
    rlmachine.executeUntilHalted();
  
    ensure_equals("We jumped somewhere unexpected on a bad value!", 
                  rlmachine.getIntValue(IntMemRef('A', 0)),
                  i);
  }
}

// -----------------------------------------------------------------------


/** 
 * 
 * @code
 * // ------------- In seen00001
 * intA[0] = 1
 * farcall(2, intB[0])
 * intA[2] = 1
 *
 * // ------------- In seen00002
 * #ENTRYPOINT 1
 * intA[1] = 1
 * ret
 * 
 * #ENTRYPOINT 2
 * intA[1] = 2
 * ret
 *
 * #ENTRYPOINT 3
 * intA[1] = 3
 * ret
 * @endcode
 */
template<>
template<>
void object::test<18>()
{
  for(int i = 1; i < 4; ++i) 
  {
    libReallive::Archive arc(locateTestCase("Module_Jmp_SEEN/farcallTest_0/SEEN.TXT"));
    NullSystem system;
    RLMachine rlmachine(system, arc);
    rlmachine.attachModule(new JmpModule);
    rlmachine.setIntValue(IntMemRef('B', 0), i);
    rlmachine.executeUntilHalted();
  
    ensure_equals("Precondition not set! (!?!?!?!)",
                  rlmachine.getIntValue(IntMemRef('A', 0)),
                  1);
    ensure_equals("We jumped somewhere unexpected on a bad value!", 
                  rlmachine.getIntValue(IntMemRef('A', 1)),
                  i);
    ensure_equals("Postcondition not set! (We didn't return correctly!)",
                  rlmachine.getIntValue(IntMemRef('A', 2)),
                  1);
  }
}

// -----------------------------------------------------------------------

/** 
 * 
 * @code
 * #version 1.3
 *
 * intB[0] = 1
 * intB[1] = 2
 * strS[0] = "one"
 * strS[1] = "two"
 *
 * intA[0] = gosub_with(intB[0], intB[1]) @intTest
 * intA[1] = gosub_with(strS[0], strS[1]) @stringTest
 * goto @end
 *
 * @stringTest
 * strS[3] = strK[0] + strK[1]
 * ret_with(strlen(strS[3]))
 * goto @end
 *
 * @intTest
 * intD[0] = intL[0] + intL[1]
 * ret_with(intD[0])
 * goto @end
 *
 * @end
 * @endcode
 */
template<>
template<>
void object::test<19>()
{
  libReallive::Archive arc(locateTestCase("Module_Jmp_SEEN/gosub_with_0.TXT"));
  NullSystem system;
  RLMachine rlmachine(system, arc);
  rlmachine.attachModule(new JmpModule);
  rlmachine.attachModule(new StrModule);
  rlmachine.executeUntilHalted();

  // @nextaction Continue writing the test case from here.
  
  // Original states that shouldn't be modified
  ensure_equals("Precondition not set! (!?!?!?!) (intB[0])",
                rlmachine.getIntValue(IntMemRef('B', 0)),
                1);
  ensure_equals("Precondition not set! (!?!?!?!) (intB[1])",
                rlmachine.getIntValue(IntMemRef('B', 1)),
                2);
  ensure_equals("Precondition not set! (!?!?!?!) (strS[0])",
                rlmachine.getStringValue(STRS_LOCATION, 0),
                "one");
  ensure_equals("Precondition not set! (!?!?!?!) (strS[1])",
                rlmachine.getStringValue(STRS_LOCATION, 1),
                "two");

  // Check the intermediate values that were calculated in the
  // functions
  ensure_equals("Wrong intermediary value for strS[3] in @stringTest!",
                rlmachine.getStringValue(STRS_LOCATION, 3),
                "onetwo");
  ensure_equals("Wrong intermediary value for intD[0] in @intTest!",
                rlmachine.getIntValue(IntMemRef('D', 0)),
                3);

  // Make sure that intA[0] and intA[1] are set correctly when the
  // gosub_with returns
  ensure_equals("Wrong final value for intA[0]!",
                rlmachine.getIntValue(IntMemRef('A', 0)),
                3);
  ensure_equals("Wrong final value for intA[1]!",
                rlmachine.getIntValue(IntMemRef('A', 1)),
                6);
}

// -----------------------------------------------------------------------

/** 
 * Recursive C implementation of fibonaci. Doesn't have all the weird
 * stack maitanance of the Reallive one I wrote, so this should help
 * test it. Used in test<20>
 */
static int recFib(int input) {
  if(input == 0) return 0;
  if(input == 1) return 1;
  return recFib(input - 1) + recFib(input - 2);
}

// -----------------------------------------------------------------------

/** 
 * Tests the most complex test regarding goto_with and the stack:
 * calculate fibonacci numbers.
 * 
 * @code
 * #version 1.3
 *
 * // Stack marker. Since Reallive provides no variable
 * // stack, we need to manually keep track of this.
 * intA[0] = 0
 *
 * // intB[0] is the top of our stack.
 * intB[0] = 0
 *
 * // Call fibonacci
 * intE[0] = gosub_with(intD[0]) @fib
 * goto @end
 *
 * // -----------------------------------------------------
 *
 * @fib
 * // Check the termination conditions
 * if intL[0] == 0 ret_with(0)
 * if intL[0] == 1 ret_with(1)
 *
 * // Copy the incoming data onto the stack, and 
 * // increment the pointer.
 * intB[intA[0]] = intL[0] - 1
 * intA[0] += 1
 * intB[intA[0]] = intL[0] - 2
 * intA[0] += 1
 * 
 * intB[intA[0] - 2] = gosub_with(intB[intA[0] - 2]) @fib
 * intB[intA[0] - 1] = gosub_with(intB[intA[0] - 1]) @fib
 *
 * // Decrement the stack point
 * intA[0] -= 2
 * ret_with(intB[intA[0]] + intB[intA[0] + 1])
 *
 * @end
 *
 * @endcode
 *
 */
template<>
template<>
void object::test<20>()
{
  for(int i = 0; i < 10; ++i) 
  {
    libReallive::Archive arc(locateTestCase("Module_Jmp_SEEN/fibonacci.TXT"));
    NullSystem system;
    RLMachine rlmachine(system, arc);
    rlmachine.attachModule(new JmpModule);
    rlmachine.attachModule(new StrModule);
    rlmachine.setIntValue(IntMemRef('D', 0), i);
    rlmachine.executeUntilHalted();

    stringstream ss;
    ss << "Wrong output value for fib(" << i << ")";
    ensure_equals(ss.str().c_str(),
                  rlmachine.getIntValue(IntMemRef('E', 0)),
                  recFib(i));
  }
}

// -----------------------------------------------------------------------

/** 
 * Tests farcall_with()/rtl_with().
 * 
 * @code
 * // ---------------------------- SEEN00001
 * #version 1.3
 *
 * intA[0] = 1
 * intA[1] = farcall_with(2, intB[0], intB[1])
 * intA[2] = 1
 *
 * // ---------------------------- SEEN00002
 * #version 1.3
 *
 * #ENTRYPOINT 1
 * rtl_with(1 + intL[0])
 *
 * #ENTRYPOINT 2
 * rtl_with(2 + intL[0])
 *
 * #ENTRYPOINT 3
 * rtl_with(3 + intL[0])
 * @endcode
 */
template<>
template<>
void object::test<21>()
{
  for(int offset = 0; offset < 2; ++offset)
  {
    for(int entrypoint = 1; entrypoint < 4; ++entrypoint) 
    {
      libReallive::Archive arc(locateTestCase("Module_Jmp_SEEN/farcall_withTest/SEEN.TXT"));
      NullSystem system;
      RLMachine rlmachine(system, arc);
      rlmachine.attachModule(new JmpModule);
      rlmachine.attachModule(new StrModule);
      rlmachine.setIntValue(IntMemRef('B', 0), entrypoint);
      rlmachine.setIntValue(IntMemRef('B', 1), offset);
      rlmachine.executeUntilHalted();

      stringstream ss;
      ss << "Wrong output value for pair (" << offset << ", " << entrypoint
         << ") in farcall_with test!";
      ensure_equals(ss.str().c_str(),
                    rlmachine.getIntValue(IntMemRef('A', 1)),
                    entrypoint + offset);
    }
  }
}

// -----------------------------------------------------------------------

// template<>
// template<>
// void object::test<22>()
// {
// //   libReallive::Archive arc(locateTestCase("Module_Jmp_SEEN/graphicTest.TXT"));
// //   NullSystem system;
// //   RLMachine rlmachine(system, arc);
// //   rlmachine.attachModule(new JmpModule);
// //   rlmachine.attachModule(new MsgModule);
// //   rlmachine.executeUntilHalted();
// }

}
