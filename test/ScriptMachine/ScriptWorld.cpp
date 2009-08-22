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
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
// -----------------------------------------------------------------------

#include "Precompiled.hpp"

// -----------------------------------------------------------------------

#include "ScriptMachine/ScriptWorld.hpp"
#include "ScriptMachine/ScriptMachine.hpp"
#include "Modules/Module_Sel.hpp"

#include "Systems/Base/System.hpp"

#include "ScriptMachine/luabind_Machine.hpp"
#include "ScriptMachine/luabind_System.hpp"
#include "ScriptMachine/luabind_EventSystem.hpp"
#include "ScriptMachine/luabind_GraphicsSystem.hpp"
#include "ScriptMachine/luabind_GraphicsObject.hpp"
#include "ScriptMachine/luabind_utility.hpp"

#include <iostream>
#include <typeinfo>
#include <boost/filesystem/path.hpp>

extern "C" {
    #include "lua.h"
    #include "lualib.h"
    #include "lauxlib.h"
}

#include <luabind/luabind.hpp>

using namespace std;
using namespace luabind;
namespace fs = boost::filesystem;

// -----------------------------------------------------------------------

ScriptWorld::ScriptWorld() {
  L = lua_open();
  luaopen_base(L);
  luaopen_string(L);
  luaopen_table(L);
  InitializeLuabind(L);

  luabind::globals(L)["World"] = this;
}

// -----------------------------------------------------------------------

ScriptWorld::~ScriptWorld() {
  // Must manually destroy all lua objects before we close the L context.
  handlers_.clear();

  lua_close(L);
}

// -----------------------------------------------------------------------

void ScriptWorld::loadToplevelFile(const std::string& lua_file) {
  script_dir_ = fs::path(lua_file).branch_path();

  if (luaL_dofile(L, lua_file.c_str())) {
    ostringstream oss;
    oss << "Error while running script: " << lua_file << " ("
        << lua_tostring(L, -1) << ")";
    throw std::runtime_error(oss.str());
  }
}

// -----------------------------------------------------------------------

void ScriptWorld::import(const std::string& file_name) {
  fs::path script_path(script_dir_ / file_name);

  if (!fs::exists(script_path)) {
    ostringstream oss;
    oss << "Could not read script file: " << script_path;
    throw std::runtime_error(oss.str());
  }

  if (luaL_dofile(L, script_path.file_string().c_str())) {
    ostringstream oss;
    oss << "Error while running script: " << script_path;
    throw std::runtime_error(oss.str());
  }
}

// -----------------------------------------------------------------------

std::string ScriptWorld::regname() const {
  ScriptMachine* machine = luabind::object_cast<ScriptMachine*>(
    luabind::globals(L)["Machine"]);
  if (machine) {
    return machine->system().regname();
  } else {
    throw std::logic_error("No machine!?");
  }
}

// -----------------------------------------------------------------------

void ScriptWorld::setDecisionList(luabind::object table) {
  decisions_.clear();
  for (luabind::iterator itr(table), end; itr != end; ++itr) {
    boost::optional<std::string> v = object_cast_nothrow<std::string>(*itr);

    if (v) {
      decisions_.push_back(*v);
    }
  }

  ScriptMachine* machine = luabind::object_cast<ScriptMachine*>(
    luabind::globals(L)["Machine"]);
  if (machine) {
    machine->setDecisionList(decisions_);
  }
}

// -----------------------------------------------------------------------

void ScriptWorld::error(const std::string& error_message) {
  ScriptMachine* machine = luabind::object_cast<ScriptMachine*>(
    luabind::globals(L)["Machine"]);
  if (machine)
    machine->halt();

  cerr << "ERROR: " << error_message << endl;
}

// -----------------------------------------------------------------------

void ScriptWorld::addHandler(int scene, int lineNo, luabind::object handler) {
  handlers_[make_pair(scene, lineNo)] = handler;

  ScriptMachine* machine = luabind::object_cast<ScriptMachine*>(
    luabind::globals(L)["Machine"]);
  if (machine) {
    machine->setHandlers(handlers_);
  }
}

// -----------------------------------------------------------------------

void ScriptWorld::initializeMachine(ScriptMachine& machine) {
  luabind::globals(L)["Machine"] = &machine;
  luabind::globals(L)["System"] = &(machine.system());
}

// -----------------------------------------------------------------------

/* static */
void ScriptWorld::InitializeLuabind(lua_State* L) {
  using namespace luabind;

  open(L);
  module(L)
  [
    // High level interface
    class_<ScriptWorld>("World").
    def("import", &ScriptWorld::import).
    def("regname", &ScriptWorld::regname).
    def("setDecisionList", &ScriptWorld::setDecisionList).
    def("error", &ScriptWorld::error).
    def("addHandler", &ScriptWorld::addHandler),

    register_utility(),

    register_machine(),
    register_system(),
    register_event_system(),
    register_graphics_system(),
    register_graphics_object()
  ];
}
