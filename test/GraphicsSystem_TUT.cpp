// -*- Mode: C++; tab-width:2; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi:tw=80:et:ts=2:sts=2
//
// -----------------------------------------------------------------------
//
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
#include "MachineBase/Serialization.hpp"
#include "MachineBase/Memory.hpp"

#include "NullSystem/NullSystem.hpp"
#include "NullSystem/NullGraphicsSystem.hpp"
#include "Systems/Base/GraphicsObjectOfFile.hpp"

#include "Utilities.h"
#include "testUtils.hpp"
#include "tut.hpp"
#include <boost/scoped_ptr.hpp>
#include <iostream>

using namespace boost;
using namespace std;
using namespace libReallive;

// -----------------------------------------------------------------------

namespace tut
{

struct GraphicsSystem_data
{
  // Use any old test case; it isn't getting executed
  libReallive::Archive arc;
  NullSystem system;
  RLMachine rlmachine;

  GraphicsSystem_data()
	: arc(locateTestCase("Module_Str_SEEN/strcpy_0.TXT")),
	  system(),
	  rlmachine(system, arc)
  {}
};

typedef test_group<GraphicsSystem_data> tf;
typedef tf::object object;
tf GraphicsSystem_data("GraphicsSystem");

// -----------------------------------------------------------------------

/** 
 * Test the serialization of an individual GraphicsObjectOfFile
 * object.
 */
template<>
template<>
void object::test<1>()
{
  scoped_ptr<GraphicsObjectData> inputObjOfFile(
    new GraphicsObjectOfFile(rlmachine, "doesntmatter"));
}

// -----------------------------------------------------------------------

};
