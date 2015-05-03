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

#include "modules/module_dll.h"

#include <string>

#include "machine/general_operations.h"
#include "machine/rlmachine.h"
#include "machine/rloperation.h"
#include "machine/rloperation/default_value.h"

namespace {

struct LoadDLL : public RLOpcode<IntConstant_T, StrConstant_T> {
  void operator()(RLMachine& machine, int slot, string name) {
    machine.LoadDLL(slot, name);
  }
};

struct CallDLL : public RLStoreOpcode<IntConstant_T,
                                      DefaultIntValue_T<0>,
                                      DefaultIntValue_T<0>,
                                      DefaultIntValue_T<0>,
                                      DefaultIntValue_T<0>,
                                      DefaultIntValue_T<0>> {
  int operator()(RLMachine& machine,
                 int index,
                 int one,
                 int two,
                 int three,
                 int four,
                 int five) {
    return machine.CallDLL(index, one, two, three, four, five);
  }
};

}  // namespace

DLLModule::DLLModule() : RLModule("DLL", 2, 1) {
  AddOpcode(10, 0, "LoadDLL", new LoadDLL);
  AddOpcode(11, 0, "UnloadDLL", CallFunction(&RLMachine::UnloadDLL));

  AddOpcode(12, 0, "CallDLL", new CallDLL);
  AddOpcode(12, 1, "CallDLL", new CallDLL);
  AddOpcode(12, 2, "CallDLL", new CallDLL);
  AddOpcode(12, 3, "CallDLL", new CallDLL);
  AddOpcode(12, 4, "CallDLL", new CallDLL);
  AddOpcode(12, 5, "CallDLL", new CallDLL);
}
