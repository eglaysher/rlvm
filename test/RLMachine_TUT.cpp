// This file is part of RLVM, a RealLive virtual machine clone.
//
// -----------------------------------------------------------------------
//
// Copyright (C) 2007 Elliot Glaysher
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

#include "Modules/Module_Str.hpp"
#include "libReallive/archive.h"
#include "libReallive/intmemref.h"
#include "MachineBase/RLMachine.hpp"

#include "NullSystem/NullSystem.hpp"

#include "Utilities.h"
#include "testUtils.hpp"
#include "tut.hpp"

#include <boost/assign/list_of.hpp>
#include <boost/lexical_cast.hpp>
#include <iostream>

using namespace std;
using libReallive::IntMemRef;
using boost::lexical_cast;
using boost::assign::list_of;

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
struct RLMachine_data
{ 
  // Use any old test case; it isn't getting executed
  libReallive::Archive arc;
  NullSystem system;
  RLMachine rlmachine;

  RLMachine_data()
	: arc(locateTestCase("Module_Str_SEEN/strcpy_0.TXT")),
	  system(),
	  rlmachine(system, arc)
  {}
};

/**
 * This group of declarations is just to register
 * test group in test-application-wide singleton.
 * Name of test group object (auto_ptr_group) shall
 * be unique in tut:: namespace. Alternatively, you
 * you may put it into anonymous namespace.
 */
typedef test_group<RLMachine_data> tf;
typedef tf::object object;
tf rlmachine_group("RLMachine");

// -----------------------------------------------------------------------

/**
 * Test halting semantics.
 */
template<>
template<>
void object::test<1>()
{
  ensure("Machine does not start halted.", !rlmachine.halted());
  rlmachine.halt();
  ensure("Machine is halted.", rlmachine.halted());
}

// -----------------------------------------------------------------------

/**
 * Test register storage.
 */
template<>
template<>
void object::test<2>()
{
  for(int i = 0; i < 10; ++i)
  {
	rlmachine.setStoreRegister(i);
	ensure_equals("Store register remember value", i, 
				  rlmachine.getStoreRegisterValue());
  }
}

// -----------------------------------------------------------------------

/**
 * Test valid string memory access.
 */
template<>
template<>
void object::test<3>()
{
  vector<int> types = list_of(0x0A)(0x0C)(0x12);

  for(vector<int>::const_iterator it = types.begin(); it != types.end(); ++it)
  {
	const string str = "Stored at " + lexical_cast<string>(*it);

	rlmachine.setStringValue(*it, 0, str);
	ensure_equals("RLMachine remembered the string", 
				  rlmachine.getStringValue(*it, 0), str);
  }
}

// -----------------------------------------------------------------------

/**
 * Test error-inducing, string memory access.
 */
template<>
template<>
void object::test<4>()
{
  try {
	rlmachine.setStringValue(0x0A, 3, "Blah");
	fail("Did not catch out of bound exception on set for strK");
  } catch(rlvm::Exception) {}

  try {
	rlmachine.setStringValue(0x0C, 2000, "Blah");
	fail("Did not catch out of bound exception on set for strM/strS");
  } catch(rlvm::Exception) {}

  try {
	rlmachine.getStringValue(0x0A, 3);
	fail("Did not catch out of bound exception on get for strK");
  } catch(rlvm::Exception) {}

  try {
	rlmachine.getStringValue(0x0C, 2000);
	fail("Did not catch out of bound exception on get for strM/strS");
  } catch(rlvm::Exception) {}
}

// -----------------------------------------------------------------------

/**
 * Test valid integer access of all types.
 *
 * For reference to understand the following
 * signed 32-bit integer: 10281 (0010 1000 0010 1001b)
 *        8-bit integers: 0,0,40,41
 *
 * For    8-bit integers: 38,39,40,41
 */
template<>
template<>
void object::test<5>()
{
  vector<int> banks = list_of(0)(1)(2)(3)(4)(5)(6)(7)(8);

  const int in8b[] = {38,  39, 40, 41};
  const int base = (in8b[0] << 24) | (in8b[1] << 16) | (in8b[2] << 8) | in8b[3];

  for(vector<int>::const_iterator it = banks.begin(); it != banks.end(); ++it)
  {
	IntMemRef wordRef(*it, 0, 0);
	rlmachine.setIntValue(wordRef, base);
	ensure_equals("Didn't record full value", 
				  rlmachine.getIntValue(wordRef), base);

// 	ensure_equals("Can't read 1st 8bit seq.", 
// 				  rlmachine.getIntValue(*it + 25, 0), in8b[0]);
  }
}

// -----------------------------------------------------------------------

/**
 * Test illegal access to integer memory.
 */
// template<>
// template<>
// void object::test<6>()
// {
  

// }


}
