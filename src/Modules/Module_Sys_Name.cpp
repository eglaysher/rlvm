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
//
// -----------------------------------------------------------------------

#include "Precompiled.hpp"

// -----------------------------------------------------------------------

#include "Modules/Module_Sys_Save.hpp"
#include "MachineBase/RLOperation.hpp"
#include "MachineBase/RLOperation/References.hpp"
#include "MachineBase/RLMachine.hpp"
#include "MachineBase/RLModule.hpp"
#include "MachineBase/Memory.hpp"

// -----------------------------------------------------------------------

struct Sys_GetName : public RLOp_Void_2< IntConstant_T, StrReference_T > {
  void operator()(RLMachine& machine, int index, StringReferenceIterator strIt) {
    *strIt = machine.memory().getName(index);
  }
};

// -----------------------------------------------------------------------

struct Sys_SetName : public RLOp_Void_2< IntConstant_T, StrConstant_T > {
  void operator()(RLMachine& machine, int index, string name) {
    machine.memory().setName(index, name);
  }
};

// -----------------------------------------------------------------------

struct Sys_GetLocalName : public RLOp_Void_2< IntConstant_T, StrReference_T > {
  void operator()(RLMachine& machine, int index, StringReferenceIterator strIt) {
    *strIt = machine.memory().getLocalName(index);
  }
};

// -----------------------------------------------------------------------

struct Sys_SetLocalName : public RLOp_Void_2< IntConstant_T, StrConstant_T > {
  void operator()(RLMachine& machine, int index, string name) {
    machine.memory().setLocalName(index, name);
  }
};

// -----------------------------------------------------------------------

void addSysNameOpcodes(RLModule& m) {
  m.addOpcode(1300, 0, "GetName", new Sys_GetName);
  m.addOpcode(1301, 0, "SetName", new Sys_SetName);
  m.addOpcode(1310, 0, "GetLocalName", new Sys_GetLocalName);
  m.addOpcode(1311, 0, "SetLocalName", new Sys_SetLocalName);
}
