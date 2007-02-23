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
 * @file   ExpressionTest_TUT.cpp
 * @author Elliot Glaysher
 * @date   Sat Sep 16 14:22:35 2006
 * @ingroup TestCase
 * 
 * Test cases for the randomish module
 */

#include "libReallive/Archive.h"
#include "MachineBase/RLMachine.hpp"

#include "Systems/Null/NullSystem.hpp"

#include "tut.hpp"

#include <iostream>
using namespace std;

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
struct ExpressionTest_data
{ 

};

/**
 * This group of declarations is just to register
 * test group in test-application-wide singleton.
 * Name of test group object (auto_ptr_group) shall
 * be unique in tut:: namespace. Alternatively, you
 * you may put it into anonymous namespace.
 */
typedef test_group<ExpressionTest_data> tf;
typedef tf::object object;
tf expression_test_group("ExpressionTest");

// -----------------------------------------------------------------------

/**
 * Tests the use of addition and subtraction in kepago. Tests "+" and
 * "-" used in an expression, "+=", and "-=".
 */
template<>
template<>
void object::test<1>()
{
  libReallive::Archive arc("test/ExpressionTest_SEEN/addition.TXT");
  RLMachine rlmachine(arc);
  rlmachine.executeUntilHalted();

  int values[4];
  for(int i = 0; i < 4; ++i)
    values[i] = rlmachine.getIntValue(0, i);

  ensure_equals("Incorect value for intA[0]", values[0], 2);
  ensure_equals("Incorect value for intA[1]", values[1], 3);
  ensure_equals("Incorect value for intA[2]", values[2], 0);
  ensure_equals("Incorect value for intA[3]", values[3], 3);
}


}


