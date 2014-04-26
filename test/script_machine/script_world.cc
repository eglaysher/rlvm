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

#include "script_machine/script_world.h"

#include <iostream>
#include <typeinfo>
#include <sstream>
#include <string>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>

#include "modules/module_sel.h"
#include "script_machine/luabind_event_system.h"
#include "script_machine/luabind_graphics_object.h"
#include "script_machine/luabind_graphics_system.h"
#include "script_machine/luabind_machine.h"
#include "script_machine/luabind_system.h"
#include "script_machine/luabind_utility.h"
#include "script_machine/script_machine.h"
#include "systems/base/system.h"

extern "C" {
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
}

#include <luabind/luabind.hpp>

using namespace std;
using namespace luabind;
namespace fs = boost::filesystem;

ScriptWorld::ScriptWorld() {
  L = lua_open();
  luaopen_base(L);
  luaopen_string(L);
  luaopen_table(L);
  InitializeLuabind(L);

  luabind::globals(L)["World"] = this;
}

ScriptWorld::~ScriptWorld() { lua_close(L); }

void ScriptWorld::LoadToplevelFile(const std::string& lua_file) {
  script_dir_ = fs::path(lua_file).branch_path();

  if (luaL_dofile(L, lua_file.c_str())) {
    ostringstream oss;
    oss << "Error while running script: " << lua_file << " ("
        << lua_tostring(L, -1) << ")";
    throw std::runtime_error(oss.str());
  }
}

void ScriptWorld::Import(const std::string& file_name) {
  fs::path script_path(script_dir_ / file_name);

  if (!fs::exists(script_path)) {
    ostringstream oss;
    oss << "Could not read script file: " << script_path;
    throw std::runtime_error(oss.str());
  }

  if (luaL_dofile(L, script_path.string().c_str())) {
    ostringstream oss;
    oss << "Error while running script: " << script_path << " ("
        << lua_tostring(L, -1) << ")";
    throw std::runtime_error(oss.str());
  }
}

std::string ScriptWorld::Regname() const {
  ScriptMachine* machine =
      luabind::object_cast<ScriptMachine*>(luabind::globals(L)["Machine"]);
  if (machine) {
    return machine->system().Regname();
  } else {
    throw std::logic_error("No machine!?");
  }
}

void ScriptWorld::SetDecisionList(luabind::object table) {
  decisions_.clear();
  for (luabind::iterator itr(table), end; itr != end; ++itr) {
    boost::optional<std::string> v = object_cast_nothrow<std::string>(*itr);

    if (v) {
      decisions_.push_back(*v);
    }
  }

  ScriptMachine* machine =
      luabind::object_cast<ScriptMachine*>(luabind::globals(L)["Machine"]);
  if (machine) {
    machine->SetDecisionList(decisions_);
  }
}

void ScriptWorld::Error(const std::string& error_message) {
  ScriptMachine* machine =
      luabind::object_cast<ScriptMachine*>(luabind::globals(L)["Machine"]);
  if (machine)
    machine->Halt();

  cerr << "ERROR: " << error_message << endl;
}

void ScriptWorld::AddHandler(int scene, int lineNo, luabind::object handler) {
  ScriptMachine* machine =
      luabind::object_cast<ScriptMachine*>(luabind::globals(L)["Machine"]);
  if (machine) {
    machine->AddLineAction(
        scene, lineNo, boost::bind(&ScriptWorld::RunHandler, handler));
  }
}

void ScriptWorld::SetDecisionHandler(luabind::object obj) {
  luabind::globals(L)["DecisionHandler"] = obj;
}

std::string ScriptWorld::MakeDecision(
    const std::vector<std::string>& decisions) {
  luabind::object handler = luabind::globals(L)["DecisionHandler"];

  if (type(handler) == LUA_TFUNCTION) {
    object table = newtable(L);
    for (int i = 0; i < decisions.size(); ++i) {
      settable(table, i, decisions[i]);
    }

    luabind::object ret = handler(table);
    if (type(ret) == LUA_TSTRING) {
      return object_cast<std::string>(ret);
    }
  }

  return "";
}

void ScriptWorld::InitializeMachine(ScriptMachine& machine) {
  luabind::globals(L)["Machine"] = &machine;
  luabind::globals(L)["System"] = &(machine.system());
}

// static
void ScriptWorld::InitializeLuabind(lua_State* L) {
  using namespace luabind;

  open(L);
  module(L)[
    // High level interface
    class_<ScriptWorld>("World")
        .def("import", &ScriptWorld::Import)
        .def("regname", &ScriptWorld::Regname)
        .def("setDecisionList", &ScriptWorld::SetDecisionList)
        .def("error", &ScriptWorld::Error)
        .def("addHandler", &ScriptWorld::AddHandler)
        .def("setDecisionHandler", &ScriptWorld::SetDecisionHandler),
    register_utility(),
    register_machine(),
    register_system(),
    register_event_system(),
    register_graphics_system(),
    register_graphics_object()
  ];
}

// static
void ScriptWorld::RunHandler(luabind::object handler) {
  try {
    luabind::call_function<void>(handler);
  }
  catch (const luabind::error& e) {
    lua_State* state = e.state();
    std::cerr << lua_tostring(state, -1) << endl;
  }
}
