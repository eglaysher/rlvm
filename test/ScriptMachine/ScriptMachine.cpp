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

#include "ScriptMachine/ScriptMachine.hpp"
#include "Modules/Module_Sel.hpp"
#include "MachineBase/Serialization.hpp"

#include <iostream>
#include <typeinfo>

using namespace std;

// -----------------------------------------------------------------------
// ScriptMachine
// -----------------------------------------------------------------------
ScriptMachine::ScriptMachine(
  System& in_system, libReallive::Archive& in_archive)
  : RLMachine(in_system, in_archive),
    current_decision_(0),
    save_on_decision_slot_(-1)
{

}

// -----------------------------------------------------------------------

ScriptMachine::~ScriptMachine() { }

// -----------------------------------------------------------------------

void ScriptMachine::setDecisionList(
  const std::vector<std::string>& decisions)
{
  decisions_ = decisions;
  current_decision_ = 0;
}

// -----------------------------------------------------------------------

void ScriptMachine::setHandlers(
  const std::map<std::pair<int, int>, luabind::object>& handlers) {
  handlers_ = handlers;
}

// -----------------------------------------------------------------------

void ScriptMachine::setLineNumber(const int i) {
  // Intercept and see if there are any tests that need to be done here:
  Handlers::iterator it = handlers_.find(make_pair(sceneNumber(), i));
  if (it != handlers_.end()) {
    try {
      luabind::call_function<void>(it->second);
    } catch(const luabind::error& e) {
      lua_State* state = e.state();
      std::cerr << lua_tostring( state, -1) << endl;
    }
  }

  RLMachine::setLineNumber(i);
}

// -----------------------------------------------------------------------

void ScriptMachine::pushLongOperation(LongOperation* long_operation) {
  // Intercept various LongOperations and modify them.
  if (typeid(*long_operation) == typeid(Sel_LongOperation)) {
    std::string to_select = decisions_.at(current_decision_);

    Sel_LongOperation& sel = dynamic_cast<Sel_LongOperation&>(*long_operation);
    bool optionFound = sel.selectOption(to_select);

    if (!optionFound) {
      cerr << "WARNING! Couldn't call option " << current_decision_
           << ". Options are: " << endl;

      // Dear C++: I can't wait for the 'auto' keyword:
      const std::vector<std::string>& options = sel.options();
      for (std::vector<std::string>::const_iterator it = options.begin();
           it != options.end(); ++it) {
        cerr << "- \"" << *it << "\"" << endl;
      }
    } else {
      cerr << "Selected '" << to_select << "'" << endl;

      if (save_on_decision_slot_ != -1) {
        cerr << "(Automatically saving to slot " << save_on_decision_slot_
             << ")" << endl;
        Serialization::saveGameForSlot(*this, save_on_decision_slot_);
      }
    }

    current_decision_++;
  }

  RLMachine::pushLongOperation(long_operation);
}
