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

#include "libReallive/archive.h"
#include "libReallive/expression.h"

#include "MachineBase/RLMachine.hpp"

#include "Modules/Module_Jmp.hpp"

#include "Systems/Null/NullSystem.hpp"

#include "tut.hpp"

#include <iostream>
using namespace std;
using namespace libReallive;

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
 * Tests basic arithmatic operations
 */
template<>
template<>
void object::test<1>()
{
  libReallive::Archive arc("test/ExpressionTest_SEEN/basicOperators.TXT");
  RLMachine rlmachine(arc);
  rlmachine.executeUntilHalted();

  int values[10];
  for(int i = 0; i < 10; ++i)
    values[i] = rlmachine.getIntValue(0, i);

  ensure_equals("Incorect value for intA[0] (+ test)", values[0], 2);
  ensure_equals("Incorect value for intA[1] (+= test)", values[1], 3);
  ensure_equals("Incorect value for intA[2] (- test)", values[2], 0);
  ensure_equals("Incorect value for intA[3] (-= test)", values[3], 3);
  ensure_equals("Incorect value for intA[4] (* test)", values[4], 10);
  ensure_equals("Incorect value for intA[5] (*= test)", values[5], 30);
  ensure_equals("Incorect value for intA[6] (/ test)", values[6], 10);
  ensure_equals("Incorect value for intA[7] (/= test)", values[7], 2);
  ensure_equals("Incorect value for intA[8] (% test)", values[8], 2);
  ensure_equals("Incorect value for intA[9] (%= test)", values[9], 1);
}

// -----------------------------------------------------------------------

/**
 * Tests basic arithmatic operations
 */
template<>
template<>
void object::test<2>()
{
/*
  libReallive::Archive arc("test/ExpressionTest_SEEN/bitwiseOperators.TXT");
  RLMachine rlmachine(arc);
  rlmachine.executeUntilHalted();

  int values[10];
  for(int i = 0; i < 10; ++i)
    values[i] = rlmachine.getIntValue(0, i);

  ensure_equals("Incorect value for intA[0] (+ test)", values[0], 2);
  ensure_equals("Incorect value for intA[1] (+= test)", values[1], 3);
  ensure_equals("Incorect value for intA[2] (- test)", values[2], 0);
  ensure_equals("Incorect value for intA[3] (-= test)", values[3], 3);
  ensure_equals("Incorect value for intA[4] (* test)", values[4], 10);
  ensure_equals("Incorect value for intA[5] (*= test)", values[5], 30);
  ensure_equals("Incorect value for intA[6] (/ test)", values[6], 10);
  ensure_equals("Incorect value for intA[7] (/= test)", values[7], 2);
  ensure_equals("Incorect value for intA[8] (% test)", values[8], 2);
  ensure_equals("Incorect value for intA[9] (%= test)", values[9], 1);
*/
}

// -----------------------------------------------------------------------


/**
 * Tests logical operators in both basic and complex contexts.
 */
template<>
template<>
void object::test<3>()
{
  libReallive::Archive arc("test/ExpressionTest_SEEN/comparisonOperators.TXT");
  RLMachine rlmachine(arc);
  rlmachine.executeUntilHalted();

  int values[14];
  for(int i = 0; i < 14; ++i)
    values[i] = rlmachine.getIntValue(0, i);

  ensure_equals("Incorect value for intA[0]", values[0], 0);
  ensure_equals("Incorect value for intA[1]", values[1], 1);
  ensure_equals("Incorect value for intA[2]", values[2], 0);
  ensure_equals("Incorect value for intA[3]", values[3], 1);
  ensure_equals("Incorect value for intA[4]", values[4], 1);
  ensure_equals("Incorect value for intA[5]", values[5], 1);
  ensure_equals("Incorect value for intA[6]", values[6], 0);
  ensure_equals("Incorect value for intA[7]", values[7], 1);
  ensure_equals("Incorect value for intA[8]", values[8], 0);
  ensure_equals("Incorect value for intA[9]", values[9], 1);
  ensure_equals("Incorect value for intA[10]", values[10], 1);
  ensure_equals("Incorect value for intA[11]", values[11], 0);
  ensure_equals("Incorect value for intA[12]", values[12], 1);
  ensure_equals("Incorect value for intA[13]", values[13], 0);
}

// -----------------------------------------------------------------------

/**
 * Tests logical operators in both basic and complex contexts.
 */
template<>
template<>
void object::test<4>()
{
  libReallive::Archive arc("test/ExpressionTest_SEEN/logicalOperators.TXT");
  RLMachine rlmachine(arc);
  rlmachine.executeUntilHalted();

  int values[7];
  for(int i = 0; i < 7; ++i)
    values[i] = rlmachine.getIntValue(0, i);

  ensure_equals("Incorect value for intA[0]", values[0], 1);
  ensure_equals("Incorect value for intA[1]", values[1], 0);
  ensure_equals("Incorect value for intA[2]", values[2], 1);
  ensure_equals("Incorect value for intA[3]", values[3], 1);
  ensure_equals("Incorect value for intA[4]", values[4], 1);
  ensure_equals("Incorect value for intA[5]", values[5], 0);
  ensure_equals("Incorect value for intA[6]", values[6], 0);
}

// -----------------------------------------------------------------------

/**
 * Tests logical operators in both basic and complex contexts.
 */
template<>
template<>
void object::test<5>()
{
  libReallive::Archive arc("test/ExpressionTest_SEEN/previousErrors.TXT");
  RLMachine rlmachine(arc);
  rlmachine.attatchModule(new JmpModule);
  rlmachine.executeUntilHalted();

  int values[6];
  for(int i = 0; i < 6; ++i)
    values[i] = rlmachine.getIntValue(1, i);

  ensure_equals("Incorect value for intB[0]", values[0], 1);
  ensure_equals("Incorect value for intB[1]", values[1], 1);
  ensure_equals("Incorect value for intB[2]", values[2], 1);
  ensure_equals("Incorect value for intB[3]", values[3], 0);
  ensure_equals("Incorect value for intB[4]", values[4], 0);
  ensure_equals("Incorect value for intB[5]", values[5], 10);
}

// -----------------------------------------------------------------------

template<>
template<>
void object::test<6>()
{
  string exprString = printableToParsableString("( ( $ 00 [ $ ff e8 03 ] ");
}

}


