// -*- Mode: C++; tab-width:2; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi:tw=80:et:ts=2:sts=2
//
// -----------------------------------------------------------------------
//
// This file is part of RLVM, a RealLive virtual machine clone.
//
// -----------------------------------------------------------------------
//
// Copyright (C) 2006 Elliot Glaysher
//  
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 3 of the License, or
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
 * @file   Gameexe_TUT.cpp
 * @author Elliot Glaysher
 * @date   Tue Oct  3 13:59:24 2006
 * 
 * @brief  Tests the Gamexe interface. 
 */

#include "libReallive/gameexe.h"

#include "testUtils.hpp"
#include "tut/tut.hpp"

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
struct Gameexe_data
{ 

};

/**
 * This group of declarations is just to register
 * test group in test-application-wide singleton.
 * Name of test group object (auto_ptr_group) shall
 * be unique in tut:: namespace. Alternatively, you
 * you may put it into anonymous namespace.
 */
typedef test_group<Gameexe_data> tf;
typedef tf::object object;
tf gameexe_group("Gameexe");

// -----------------------------------------------------------------------

/**
 * Tests that we read all keys
 */
template<>
template<>
void object::test<1>()
{
  Gameexe ini(locateTestCase("Gameexe_data/Gameexe.ini"));
  ensure_equals("Wrong number of keys", ini.size(), 26);
}

// -----------------------------------------------------------------------

/** 
 * Make sure #CAPTION exists and that we read its value correctly.
 */
template<>
template<>
void object::test<2>()
{
  Gameexe ini(locateTestCase("Gameexe_data/Gameexe.ini"));
  ensure("#CAPTION exists", ini("CAPTION").exists());
  ensure_equals("Wrong value for CAPTION", ini("CAPTION").to_string(),
                string("Canon: A firearm"));
}

// -----------------------------------------------------------------------

/** 
 * Make sure \#RANDOM_KEY doesn't exist.
 */
template<>
template<>
void object::test<3>()
{
  Gameexe ini(locateTestCase("Gameexe_data/Gameexe.ini"));
  ensure("#RANDOM_KEY does not exist", !ini("RANDOM_KEY").exists());
}

// -----------------------------------------------------------------------

template<>
template<>
void object::test<4>()
{
  Gameexe ini(locateTestCase("Gameexe_data/Gameexe.ini"));
  ensure("#WINDOW_ATTR exists!", ini("WINDOW_ATTR").exists());
  vector<int> ints = ini("WINDOW_ATTR").to_intVector();

  for(int i = 0; i < 5; ++i)
    ensure_equals("Wrong value!", ints.at(i), i + 1);
}

// -----------------------------------------------------------------------

/**
 * Make sure operator() works with multiple keys...
 */
template<>
template<>
void object::test<5>()
{
  Gameexe ini(locateTestCase("Gameexe_data/Gameexe.ini"));
  ensure_equals("Wrong value!", ini("IMAGINE", "ONE"), 1);
  ensure_equals("Wrong value!", ini("IMAGINE", "TWO"), 2);
  ensure_equals("Wrong value!", ini("IMAGINE", "THREE"), 3);
}

// -----------------------------------------------------------------------

/**
 * Make sure GameexeInterpretObject chaining works correctly.
 */
template<>
template<>
void object::test<6>()
{
  Gameexe ini(locateTestCase("Gameexe_data/Gameexe.ini"));
  GameexeInterpretObject imagine = ini("IMAGINE");
  ensure_equals("Wrong value!", imagine("ONE"), 1);
  ensure_equals("Wrong value!", imagine("TWO"), 2);
  ensure_equals("Wrong value!", imagine("THREE"), 3);
}

// -----------------------------------------------------------------------

/**
 * Make sure the filtering iterators work on Gameexe...
 */
template<>
template<>
void object::test<7>()
{
  Gameexe ini(locateTestCase("Gameexe_data/Gameexe.ini"));
  GameexeFilteringIterator it = ini.filtering_begin("IMAGINE");
  GameexeFilteringIterator end = ini.filtering_end();
  for(; it != end; ++it)
  {
    if(it->key() != "IMAGINE.ONE" &&
       it->key() != "IMAGINE.TWO" &&
       it->key() != "IMAGINE.THREE")
    {
      fail("Failed to filter keys in GameexeFilteringIterator.");
    }
  }
}

// -----------------------------------------------------------------------

/**
 * Make sure we get the right values back from key_parts().
 */
template<>
template<>
void object::test<8>()
{
  Gameexe ini(locateTestCase("Gameexe_data/Gameexe.ini"));  
  GameexeInterpretObject gio = ini("WINDOW.000.ATTR_MOD");
  vector<string> pieces = gio.key_parts();
  ensure_equals(pieces.size(), 3);
  ensure_equals(pieces[0], "WINDOW");
  ensure_equals(pieces[1], "000");
  ensure_equals(pieces[2], "ATTR_MOD");
}

// -----------------------------------------------------------------------

}
