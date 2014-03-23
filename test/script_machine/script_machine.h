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

#ifndef TEST_SCRIPT_MACHINE_SCRIPT_MACHINE_H_
#define TEST_SCRIPT_MACHINE_SCRIPT_MACHINE_H_

#include "machine/rlmachine.h"

#include <luabind/luabind.hpp>
#include <map>
#include <string>
#include <utility>
#include <vector>

class ScriptWorld;

// A special RLMachine used in testing, which automatically selects specific
class ScriptMachine : public RLMachine {
 public:
  ScriptMachine(ScriptWorld& world,
                System& in_system,
                libreallive::Archive& in_archive);
  virtual ~ScriptMachine();

  void set_increment_on_save() { increment_on_save_ = true; }
  void set_save_on_decision_slot(int slot) { save_on_decision_slot_ = slot; }

  // Sets the decisions to take.
  void SetDecisionList(const std::vector<std::string>& decisions);

  // Memory accessor. (Maybe just translate this in luabind_Machine?)
  int GetInt(const std::string& bank, int position);

  // Overloaded from RLMachine:
  virtual void PushLongOperation(LongOperation* long_operation) override;

 private:
  typedef std::vector<std::string> Selections;
  Selections decisions_;

  ScriptWorld& world_;

  int current_decision_;

  // Which game save slot to automatically save to when we automatically make
  // a decision, or -1 if disabled.
  int save_on_decision_slot_;

  // Whether we increment |save_on_decision_slot_| every time we save.
  bool increment_on_save_;
};  // end of class ScriptMachine

#endif  // TEST_SCRIPT_MACHINE_SCRIPT_MACHINE_H_
