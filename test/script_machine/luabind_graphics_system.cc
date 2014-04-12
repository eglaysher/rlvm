// -*- Mode: C++; tab-width:2; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi:tw=80:et:ts=2:sts=2
//
// -----------------------------------------------------------------------
//
// This file is part of RLVM, a RealLive virtual machine clone.
//
// -----------------------------------------------------------------------
//
// Copyright (C) 2008 Elliot Glaysher
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
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
// -----------------------------------------------------------------------

#include "script_machine/luabind_system.h"

#include <luabind/luabind.hpp>

#include "systems/base/graphics_system.h"
#include "systems/base/graphics_object.h"
#include "systems/base/parent_graphics_object_data.h"

#include <iostream>

using namespace luabind;
using namespace std;

namespace {

GraphicsObject& getFgObject(GraphicsSystem& sys, int obj_number) {
  return sys.GetObject(0, obj_number);
}

GraphicsObject& getChildFgObject(GraphicsSystem& sys, int parent, int child) {
  GraphicsObject& obj = sys.GetObject(0, parent);
  if (obj.has_object_data() && obj.GetObjectData().IsParentLayer()) {
    return static_cast<ParentGraphicsObjectData&>(obj.GetObjectData())
        .GetObject(child);
  }

  cerr << "WARNING: Couldn't get child object (" << parent << ", " << child
       << "). Returning just the parent object instead." << endl;
  return obj;
}

}  // namespace

scope register_graphics_system() {
  return class_<GraphicsSystem>("GraphicsSystem")
      .def("getFgObject", getFgObject)
      .def("getChildFgObject", getChildFgObject);
}
