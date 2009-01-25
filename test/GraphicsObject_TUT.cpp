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

#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/serialization/scoped_ptr.hpp>
#include <boost/assign/list_of.hpp>
#include <boost/tuple/tuple.hpp>

#include "Modules/Module_Str.hpp"
#include "libReallive/archive.h"
#include "libReallive/intmemref.h"
#include "MachineBase/RLMachine.hpp"
#include "MachineBase/Serialization.hpp"
#include "MachineBase/Memory.hpp"

#include "NullSystem/NullSystem.hpp"
#include "NullSystem/NullGraphicsSystem.hpp"
#include "Systems/Base/GraphicsObject.hpp"
#include "Systems/Base/GraphicsObjectOfFile.hpp"

#include "testUtils.hpp"
#include "tut/tut.hpp"
#include <boost/scoped_ptr.hpp>
#include <boost/function.hpp>
#include <iostream>

using namespace boost;
using namespace boost::assign;
using namespace std;
using namespace libReallive;
using namespace Serialization;

// -----------------------------------------------------------------------

namespace tut
{

const string FILE_NAME = "doesntmatter";

struct GraphicsObject_data
{
  // Use any old test case; it isn't getting executed
  libReallive::Archive arc;
  NullSystem system;
  RLMachine rlmachine;

  GraphicsObject_data()
	: arc(locateTestCase("Module_Str_SEEN/strcpy_0.TXT")),
	  system(),
	  rlmachine(system, arc)
  {}
};

typedef test_group<GraphicsObject_data> tf;
typedef tf::object object;
tf GraphicsObject_data("GraphicsObject");

// -----------------------------------------------------------------------

/**
 * Test the serialization of an individual GraphicsObjectOfFile
 * object.
 */
template<>
template<>
void object::test<1>()
{
  stringstream ss;
  Serialization::g_current_machine = &rlmachine;

  {
    const scoped_ptr<GraphicsObjectData> inputObjOfFile(
      new GraphicsObjectOfFile(system, FILE_NAME));
    boost::archive::text_oarchive oa(ss);
    oa << inputObjOfFile;
  }

  {
    scoped_ptr<GraphicsObjectData> dst;
    boost::archive::text_iarchive ia(ss);
    ia >> dst;

    GraphicsObjectOfFile& obj = dynamic_cast<GraphicsObjectOfFile&>(*dst);
    ensure_equals("Preserved file name", obj.filename(), FILE_NAME);
  }

  Serialization::g_current_machine = NULL;
}

// -----------------------------------------------------------------------

/**
 * Try it again, this time wrapped in the GraphicsObject
 */
template<>
template<>
void object::test<2>()
{
  stringstream ss;
  Serialization::g_current_machine = &rlmachine;

  {
    const scoped_ptr<GraphicsObject> obj(new GraphicsObject());
    obj->setObjectData(new GraphicsObjectOfFile(system, FILE_NAME));

    boost::archive::text_oarchive oa(ss);
    oa << obj;
  }

  {
    scoped_ptr<GraphicsObject> dst;
    boost::archive::text_iarchive ia(ss);
    ia >> dst;

    GraphicsObjectOfFile& obj =
      dynamic_cast<GraphicsObjectOfFile&>(dst->objectData());

    // Now query invariants.
    ensure_equals("Didn't preserve filename", obj.filename(), FILE_NAME);
  }

  Serialization::g_current_machine = NULL;
}

// -----------------------------------------------------------------------

/**
 * Tests to make sure that calling a mutating method on a
 * GraphicsObject instance will force the copy-on-write semantics to
 * kick in.
 */
template<>
template<>
void object::test<3>()
{
  // Automatable ones
  typedef boost::tuple<
    boost::function<void(GraphicsObject&, const int)>,
    boost::function<int(const GraphicsObject&)> > TupleT;

  typedef vector<TupleT> SetterVec;
  SetterVec setters =
    tuple_list_of(&GraphicsObject::setVisible, &GraphicsObject::visible)
    (&GraphicsObject::setX, &GraphicsObject::x)
    (&GraphicsObject::setY, &GraphicsObject::y)
    (&GraphicsObject::setVert, &GraphicsObject::vert)
    (&GraphicsObject::setXOrigin, &GraphicsObject::xOrigin)
    (&GraphicsObject::setYOrigin, &GraphicsObject::yOrigin)
    (&GraphicsObject::setWidth, &GraphicsObject::width)
    (&GraphicsObject::setHeight, &GraphicsObject::height)
    (&GraphicsObject::setRotation, &GraphicsObject::rotation)
    (&GraphicsObject::setPattNo, &GraphicsObject::pattNo)
    (&GraphicsObject::setInvert, &GraphicsObject::invert)
    (&GraphicsObject::setLight, &GraphicsObject::light)
    (&GraphicsObject::setCompositeMode, &GraphicsObject::compositeMode)
    (&GraphicsObject::setScrollRateX, &GraphicsObject::scrollRateX)
    (&GraphicsObject::setScrollRateY, &GraphicsObject::scrollRateY)
    (&GraphicsObject::setAlpha, &GraphicsObject::alpha)
    (&GraphicsObject::setWipeCopy, &GraphicsObject::wipeCopy);

  for(SetterVec::iterator it = setters.begin(); it != setters.end(); ++it)
  {
    GraphicsObject obj;
    GraphicsObject objCopy(obj);

    // At this step, it is equal to three because they all share the
    // empty object
    ensure_equals("Both objects have the same internal object",
                  obj.referenceCount(), 3);
    ensure_equals("Both objects have the same internal object",
                  objCopy.referenceCount(), 3);

    // Call the getter method (ignoring the result). We expect that
    // this won't force a copy-on-write.
    (it->get<1>())(objCopy);

    ensure_equals("Both objects have the same internal object",
                  obj.referenceCount(), 3);
    ensure_equals("Both objects have the same internal object",
                  objCopy.referenceCount(), 3);

    // Call this setter function. This should force the copy-on-write
    // code to trigger.
    (it->get<0>())(objCopy, 1);

    ensure_equals("Untouched object still points to empty",
                  obj.referenceCount(), 2);
    ensure_equals("Modified object has its own impl",
                  objCopy.referenceCount(), 1);

    // Make sure we get the right value back
  }

  // OTHER: setXAdjustment, setYAdjustment, clearClip, setClip,
  // setTint[RGB]?, setColour[RGB]?
}

// -----------------------------------------------------------------------

};
