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
#include "MachineBase/RLMachine.hpp"

#include "NullSystem/NullSystem.hpp"

#include "Utilities.h"
#include "testUtils.hpp"
#include "tut.hpp"

#include <boost/assign/list_of.hpp>
#include <boost/lexical_cast.hpp>
#include <iostream>

using namespace std;

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
 * Test register storage.
 */
template<>
template<>
void object::test<1>()
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
void object::test<2>()
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
void object::test<3>()
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


}
