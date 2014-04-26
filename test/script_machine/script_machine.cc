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

#include "script_machine/script_machine.h"

#include <iostream>
#include <map>
#include <string>
#include <typeinfo>
#include <utility>
#include <vector>

#include "libreallive/intmemref.h"
#include "long_operations/button_object_select_long_operation.h"
#include "long_operations/select_long_operation.h"
#include "machine/serialization.h"
#include "script_machine/script_world.h"

using namespace std;
using libreallive::IntMemRef;

// -----------------------------------------------------------------------
// ScriptMachine
// -----------------------------------------------------------------------
ScriptMachine::ScriptMachine(ScriptWorld& world,
                             System& in_system,
                             libreallive::Archive& in_archive)
    : RLMachine(in_system, in_archive),
      world_(world),
      current_decision_(0),
      save_on_decision_slot_(-1),
      increment_on_save_(false) {}

ScriptMachine::~ScriptMachine() {}

void ScriptMachine::SetDecisionList(const std::vector<std::string>& decisions) {
  decisions_ = decisions;
  current_decision_ = 0;
}

void ScriptMachine::PushLongOperation(LongOperation* long_operation) {
  // Intercept various LongOperations and modify them.
  if (SelectLongOperation* sel =
          dynamic_cast<SelectLongOperation*>(long_operation)) {
    // Try our optional, script provided matching behaviour.
    std::string choice = world_.MakeDecision(sel->GetOptions());
    bool optionFound = false;
    if (choice != "") {
      optionFound = sel->SelectByText(choice);
      if (!optionFound) {
        cerr << "WTF? Script decision handler returned invalid choice" << endl;
      } else {
        cerr << "Selected '" << choice << "' (Chosen by decision handler)"
             << endl;
      }
    }

    // Try our normal matching behaviour.
    if (!optionFound) {
      int offset = 0;
      for (; offset < 4; ++offset) {
        if (current_decision_ + offset >= decisions_.size()) {
          cerr << "WARNING! Ran out of options on decision list." << endl;
          break;
        }

        std::string to_select = decisions_.at(current_decision_ + offset);
        optionFound = sel->SelectByText(to_select);

        if (optionFound) {
          cerr << "Selected '" << to_select << "'";
          if (offset > 0)
            cerr << "(Skipped " << offset << " selections)";
          cerr << endl;

          if (save_on_decision_slot_ != -1) {
            cerr << "(Automatically saving to slot " << save_on_decision_slot_
                 << ")" << endl;
            Serialization::saveGameForSlot(*this, save_on_decision_slot_);

            if (increment_on_save_) {
              save_on_decision_slot_++;
              if (save_on_decision_slot_ > 98) {
                cerr << "Warning: Overflowing save count." << endl;
                save_on_decision_slot_ = 0;
              }
            }
          }

          break;
        }
      }

      if (!optionFound) {
        cerr << "WARNING! Couldn't call option " << current_decision_
             << ". Options are: " << endl;

        for (const std::string& option : sel->GetOptions())
          cerr << "- \"" << option << "\"" << endl;

        current_decision_++;
      } else {
        current_decision_ += offset + 1;
      }
    }
  } else if (ButtonObjectSelectLongOperation* sel =
                 dynamic_cast<ButtonObjectSelectLongOperation*>(
                     long_operation)) {
    // We don't deal with these yet. Return 1 for the first option every time,
    // which corresponds with selecting "NO" for battle missions.
    set_store_register(1);
    delete sel;
    return;
  }

  RLMachine::PushLongOperation(long_operation);
}

int ScriptMachine::GetInt(const std::string& bank, int position) {
  char bchar = bank[0];

  return GetIntValue(IntMemRef(bchar, position));
}
