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

#ifndef TEST_SCRIPT_MACHINE_SCRIPT_WORLD_H_
#define TEST_SCRIPT_MACHINE_SCRIPT_WORLD_H_

#include <utility>
#include <map>
#include <string>
#include <vector>
#include <boost/filesystem/path.hpp>
#include <luabind/luabind.hpp>

extern "C" {
#include "lua.h"
}

class ScriptMachine;

// Class which encapsulates all lua scripting. It's created before a System or
// the RLMachine subclass used in scripting.
//
// Because the lua script is parsed first, the script can only call methods on
// the ScriptWorld object, exposed as "World:...". Callback functions, OTOH,
// can call into the Machine and System objects:
//
//   World:AddHandler(400, 0, function
//     Machine:dosomething()
//   end
class ScriptWorld {
 public:
  ScriptWorld();
  ~ScriptWorld();

  // Copies data from the initial script execution into the ScriptMachine,
  // since the lua script is executed during the ScriptWorld constructor, but
  // some of the data is meant for the ScriptMachine.
  void InitializeMachine(ScriptMachine& machine);

  // Try to make a decision based on the decision handler. Will return the
  // empty string if there is no decision handler added or if the decision
  // handler returned nil.
  std::string MakeDecision(const std::vector<std::string>& decisions);

  // Loads the "main" lua testing file (and settings the search path for
  // future import() statements).
  void LoadToplevelFile(const std::string& lua_file);

  // Loads a lua script in the same directory as lua_file.
  void Import(const std::string& file_name);

  // Returns the sanitized #REGNAME key (as would be found in the ~/.rlvm
  // folder).
  std::string Regname() const;

  void SetDecisionList(luabind::object table);
  void Error(const std::string& error_message);
  void AddHandler(int scene, int lineNo, luabind::object handler);

  // Sets a function that can override the decision list. The function takes an
  // table of strings, which are possible decisions. It will either return the
  // string to select or nil if the default behaviour should be used instead.
  void SetDecisionHandler(luabind::object obj);

 private:
  static void InitializeLuabind(lua_State* L);

  // Callback function used to implement addHandler().
  static void RunHandler(luabind::object handler);

  // The directory containg the passed in |lua_file|. Used as a search path.
  boost::filesystem::path script_dir_;

  std::vector<std::string> decisions_;

  luabind::object decision_handler_;

  lua_State* L;
};  // end of class ScriptWorld

#endif  // TEST_SCRIPT_MACHINE_SCRIPT_WORLD_H_
