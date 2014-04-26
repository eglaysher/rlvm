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
#include <luabind/out_value_policy.hpp>

#include "systems/base/graphics_object.h"

using namespace luabind;

namespace {

Point getClickPointHack(const GraphicsObject& obj) {
  // WARNING! THIS WILL BE REALLY WRONG WHEN THEY USE ADJUSTMENTS, OVERRIDES,
  // ET CETERA.
  //
  // The real solution is to make a GraphicsObject:center() method, but that
  // would involve some scary pulling code out of Texture... Math is hard,
  // let's go coding!
  return Point(obj.x() + 5, obj.y() + 5);
}

}  // namespace

scope register_graphics_object() {
  return class_<GraphicsObject>("GraphicsObject")
      .def("getClickPointHack", &getClickPointHack)
      .def("text", &GraphicsObject::GetTextText)
      .def("visible", &GraphicsObject::visible);
}
