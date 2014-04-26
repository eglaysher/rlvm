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

#include "script_machine/luabind_utility.h"

#include <luabind/luabind.hpp>
#include <luabind/operator.hpp>

#include "systems/base/rect.h"

using namespace luabind;

scope register_utility() {
  return class_<Point>("Point")
             .def(constructor<>())
             .def(constructor<int, int>())
             .def("x", &Point::x)
             .def("y", &Point::y)
             .def("isEmpty", &Point::is_empty)
             .def(const_self + other<Point>())
             .def(const_self + other<Size>())
             .def(const_self - other<Size>())
             .def(const_self - other<Point>())
         // TODO: Figure out how to bind == and !=
             .def(tostring(self)),
         class_<Size>("Size")
             .def(constructor<>())
             .def(constructor<int, int>())
             .def("width", &Size::width)
             .def("height", &Size::height)
             .def("setWidth", &Size::set_width)
             .def("setHeight", &Size::set_height)
             .def("isEmpty", &Size::is_empty)
             .def(const_self + other<Size>())
             .def(const_self - other<Size>())
             .def(const_self * float())
         // TODO: Figure out how to bind == and !=
             .def(tostring(self)),
         class_<Rect>("Rect")
             .def(constructor<>())
             .def(constructor<const Point, const Point>())
             .def(constructor<const Point, const Size>())
             .def(constructor<const int, const int, const Size>())
             .def("x", &Rect::x)
             .def("y", &Rect::y)
             .def("origin", &Rect::origin)
             .def("x2", &Rect::x2)
             .def("y2", &Rect::y2)
             .def("width", &Rect::width)
             .def("height", &Rect::height)
             .def("size", &Rect::size)
             .def("isEmpty", &Rect::is_empty)
             .def("contains", &Rect::Contains)
             .def(tostring(self));
}
