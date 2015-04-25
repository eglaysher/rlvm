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
//
// -----------------------------------------------------------------------

#include "modules/module_sys_name.h"

#include <string>

#include "machine/rloperation.h"
#include "machine/rloperation/references.h"
#include "machine/rlmachine.h"
#include "machine/rlmodule.h"
#include "machine/memory.h"

// -----------------------------------------------------------------------

namespace {

struct GetName : public RLOpcode<IntConstant_T, StrReference_T> {
  void operator()(RLMachine& machine,
                  int index,
                  StringReferenceIterator strIt) {
    *strIt = machine.memory().GetName(index);
  }
};

struct SetName : public RLOpcode<IntConstant_T, StrConstant_T> {
  void operator()(RLMachine& machine, int index, string name) {
    machine.memory().SetName(index, name);
  }
};

struct GetLocalName : public RLOpcode<IntConstant_T, StrReference_T> {
  void operator()(RLMachine& machine,
                  int index,
                  StringReferenceIterator strIt) {
    *strIt = machine.memory().GetLocalName(index);
  }
};

struct SetLocalName : public RLOpcode<IntConstant_T, StrConstant_T> {
  void operator()(RLMachine& machine, int index, string name) {
    machine.memory().SetLocalName(index, name);
  }
};

}  // namespace

// -----------------------------------------------------------------------

void AddSysNameOpcodes(RLModule& m) {
  m.AddOpcode(1300, 0, "GetName", new GetName);
  m.AddOpcode(1301, 0, "SetName", new SetName);
  m.AddOpcode(1310, 0, "GetLocalName", new GetLocalName);
  m.AddOpcode(1311, 0, "SetLocalName", new SetLocalName);
}
