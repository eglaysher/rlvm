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

#ifndef __ScriptWorld_hpp__
#define __ScriptWorld_hpp__

#include <vector>
#include <luabind/luabind.hpp>

extern "C"
{
    #include "lua.h"
}

// -----------------------------------------------------------------------

class ScriptMachine;

/**
 * Class which encapsulates all lua scripting. It's created before a System or
 * the RLMachine subclass used in scripting.
 *
 * Because the lua script is parsed first, the script can only call methods on
 * the ScriptWorld object, exposed as "World:...". Callback functions, OTOH,
 * can call into the Machine and System objects:
 *
 * @code
 * World:AddHandler(400, 0, function
 *   Machine:dosomething()
 * end
 * @endcode
 */
class ScriptWorld
{
public:
  ScriptWorld(const std::string& lua_file);
  ~ScriptWorld();

  const std::string& regname() const { return regname_; }
  void setRegname(const std::string& in) { regname_ = in; }

  const std::string& gameRoot() const { return game_root_; }
  void setGameRoot(const std::string& in) { game_root_ = in; }

  /**
   * Copies data from the initial script execution into the ScriptMachine,
   * since the lua script is executed during the ScriptWorld constructor, but
   * some of the data is meant for the ScriptMachine.
   */
  void initializeMachine(ScriptMachine& machine);

  void setDecisionList(luabind::object table);

private:
  static void InitializeLuabind(lua_State* L);

  std::vector<std::string> decisions_;

  std::string regname_;
  std::string game_root_;

  lua_State* L;
};  // end of class ScriptWorld


#endif
