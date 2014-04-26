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
#include <luabind/raw_policy.hpp>

#include "script_machine/script_machine.h"
#include "systems/base/event_system.h"

using namespace luabind;

namespace {

// For convenience, we don't force script users to manually specify the
// RLMachine argument, and fetch it ourselves from the globals.

void injectMouseMovement(lua_State* L, EventSystem& sys, const Point& loc) {
  ScriptMachine* machine =
      luabind::object_cast<ScriptMachine*>(luabind::globals(L)["Machine"]);
  sys.InjectMouseMovement(*machine, loc);
}

void injectMouseDown(lua_State* L, EventSystem& sys) {
  ScriptMachine* machine =
      luabind::object_cast<ScriptMachine*>(luabind::globals(L)["Machine"]);
  sys.InjectMouseDown(*machine);
}

void injectMouseUp(lua_State* L, EventSystem& sys) {
  ScriptMachine* machine =
      luabind::object_cast<ScriptMachine*>(luabind::globals(L)["Machine"]);
  sys.InjectMouseUp(*machine);
}

}  // namespace

scope register_event_system() {
  return class_<EventSystem>("EventSystem")
      .def("injectMouseMovement", &injectMouseMovement, raw(_1))
      .def("injectMouseDown", &injectMouseDown, raw(_1))
      .def("injectMouseUp", &injectMouseUp, raw(_1));
}
