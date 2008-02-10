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
 * @file   Module_Sys_TUT.cpp
 * @author Elliot Glaysher
 * @date   Sat Sep 16 14:22:35 2006
 * @ingroup TestCase
 * 
 * Test cases for the randomish module
 */

#include "Modules/Module_Sys.hpp"
#include "Modules/Module_Jmp.hpp"
#include "libReallive/archive.h"
#include "libReallive/intmemref.h"
#include "MachineBase/RLMachine.hpp"

#include "NullSystem/NullSystem.hpp"

#include "testUtils.hpp"
#include "tut.hpp"

#include <iostream>
using namespace std;

using libReallive::IntMemRef;

namespace tut
{

/**
 * Sysuct which may contain test data members.
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
struct Module_Sys_data
{ 

};

/**
 * This group of declarations is just to register
 * test group in test-application-wide singleton.
 * Name of test group object (auto_ptr_group) shall
 * be unique in tut:: namespace. Alternatively, you
 * you may put it into anonymous namespace.
 */
typedef test_group<Module_Sys_data> tf;
typedef tf::object object;
tf module_sys_group("Module_Sys");

// -----------------------------------------------------------------------

/**
 * Tests SceneNum, which should return the current scene number. Calls
 * a SEEN.TXT made out of three SEENS, which just call SceneNum() and
 * jump to the next scenario.
 */
template<>
template<>
void object::test<1>()
{
  libReallive::Archive arc(locateTestCase("Module_Sys_SEEN/SceneNum/SEEN.TXT"));
  NullSystem system;
  RLMachine rlmachine(system, arc);
  rlmachine.attachModule(new SysModule);
  rlmachine.attachModule(new JmpModule);
  rlmachine.executeUntilHalted();

  int values[3];
  for(int i = 0; i < 3; ++i)
    values[i] = rlmachine.getIntValue(IntMemRef(0, i));

  ensure_equals("SceneNum::SEEN0001 didn't set value", values[0], 1);
  ensure_equals("SceneNum::SEEN0248 didn't set value", values[1], 248);
  ensure_equals("SceneNum::SEEN0639 didn't set value", values[2], 639);
}

// -----------------------------------------------------------------------

/**
 * Tests integer built-in functions.
 */
template<>
template<>
void object::test<2>()
{
  libReallive::Archive arc(locateTestCase("Module_Sys_SEEN/builtins.TXT"));
  NullSystem system;
  RLMachine rlmachine(system, arc);
  rlmachine.attachModule(new SysModule);
  rlmachine.executeUntilHalted();

  int values[6];
  for(int i = 0; i < sizeof(values) / sizeof(values[0]); ++i)
    values[i] = rlmachine.getIntValue(IntMemRef('A', i));

  // 5.6.1  Integer built-ins
  ensure_equals("Incorrect value for intA[0]", values[0], 0);
  ensure       ("Incorrect value for intA[1]", values[1] == 0 || values[1] == -1);
  ensure_equals("Incorrect value for intA[2]", values[2], -1);
  ensure       ("Incorrect value for intA[3]", -10 <= values[3] && values[3] < 0 );
  ensure       ("Incorrect value for intA[4]", -10 <= values[4] && values[4] <= 10 );
  ensure       ("Incorrect value for intA[5]", -10 <= values[5] && values[5] <= 10 );
}


// -----------------------------------------------------------------------

/*
template<>
template<>
void object::test<3>()
{
  
}
*/

}


