// -*- Mode: C++; tab-width:2; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi:tw=80:et:ts=2:sts=2
//
// -----------------------------------------------------------------------
//
// This file is part of RLVM, a RealLive virtual machine clone.
//
// -----------------------------------------------------------------------
//
// Copyright (C) 2009 Elliot Glaysher
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
// -----------------------------------------------------------------------

#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>

#include "testUtils.hpp"
#include "tut/tut.hpp"

#include "NullSystem/NullSoundSystem.hpp"
#include "libReallive/gameexe.h"

#include <sstream>

using std::stringstream;

namespace tut
{

struct SoundSystem_data
{
};

typedef test_group<SoundSystem_data> tf;
typedef tf::object object;
tf SoundSystem_data("SoundSystem");

/**
 * Makes sure we can parse the bizarre Gameexe.ini keys for KOEONOFF
 */
template<>
template<>
void object::test<1>()
{
  Gameexe gexe(locateTestCase("Gameexe_data/Gameexe_koeonoff.ini"));
  NullSoundSystem sys(gexe);

  // Test the UseKoe side of things
  ensure_equals(sys.useKoeForCharacter(0), 1);
  ensure_equals(sys.useKoeForCharacter(7), 0);
  ensure_equals(sys.useKoeForCharacter(8), 1);

  // Test the koePlay side of things
  ensure_equals(sys.globals().character_koe_enabled.size(), 5);
  ensure_equals(sys.globals().character_koe_enabled[0], 1);
  ensure_equals(sys.globals().character_koe_enabled[3], 0);
  ensure_equals(sys.globals().character_koe_enabled[2], 1);
  ensure_equals(sys.globals().character_koe_enabled[20], 1);
  ensure_equals(sys.globals().character_koe_enabled[105], 1);
}

// -----------------------------------------------------------------------

/**
 * Tests that SetUseKoe sets values correctly
 */
template<>
template<>
void object::test<2>()
{
  Gameexe gexe(locateTestCase("Gameexe_data/Gameexe_koeonoff.ini"));
  NullSoundSystem sys(gexe);

  sys.setUseKoeForCharacter(0, 0);
  sys.setUseKoeForCharacter(7, 1);
  sys.setUseKoeForCharacter(8, 0);

  // Make sure all values are flipped from the previous test.

  // Test the UseKoe side of things
  ensure_equals(sys.useKoeForCharacter(0), 0);
  ensure_equals(sys.useKoeForCharacter(7), 1);
  ensure_equals(sys.useKoeForCharacter(8), 0);

  // Test the koePlay side of things
  ensure_equals(sys.globals().character_koe_enabled.size(), 5);
  ensure_equals(sys.globals().character_koe_enabled[0], 0);
  ensure_equals(sys.globals().character_koe_enabled[3], 1);
  ensure_equals(sys.globals().character_koe_enabled[2], 0);
  ensure_equals(sys.globals().character_koe_enabled[20], 0);
  ensure_equals(sys.globals().character_koe_enabled[105], 0);
}

// -----------------------------------------------------------------------

/**
 * Make sure we thaw previously serialized character_koe_enabled data correctly.
 */
template<>
template<>
void object::test<3>()
{
  Gameexe gexe(locateTestCase("Gameexe_data/Gameexe_koeonoff.ini"));
  stringstream ss;

  {
    NullSoundSystem sys(gexe);

    // Reverse the values as in <2>.
    sys.setUseKoeForCharacter(0, 0);
    sys.setUseKoeForCharacter(7, 1);
    sys.setUseKoeForCharacter(8, 0);

    boost::archive::text_oarchive oa(ss);
    oa << const_cast<const SoundSystemGlobals&>(sys.globals());
  }

  {
    NullSoundSystem sys(gexe);
    boost::archive::text_iarchive ia(ss);
    ia >> sys.globals();

    // Do the flip tests as in <2>

    // Test the UseKoe side of things
    ensure_equals(sys.useKoeForCharacter(0), 0);
    ensure_equals(sys.useKoeForCharacter(7), 1);
    ensure_equals(sys.useKoeForCharacter(8), 0);

    // Test the koePlay side of things
    ensure_equals(sys.globals().character_koe_enabled.size(), 5);
    ensure_equals(sys.globals().character_koe_enabled[0], 0);
    ensure_equals(sys.globals().character_koe_enabled[3], 1);
    ensure_equals(sys.globals().character_koe_enabled[2], 0);
    ensure_equals(sys.globals().character_koe_enabled[20], 0);
    ensure_equals(sys.globals().character_koe_enabled[105], 0);
  }
}


};
