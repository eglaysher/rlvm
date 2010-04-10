// -*- Mode: C++; tab-width:2; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi:tw=80:et:ts=2:sts=2
//
// -----------------------------------------------------------------------
//
// This file is part of RLVM, a RealLive virtual machine clone.
//
// -----------------------------------------------------------------------
//
// Copyright (C) 2007 Elliot Glaysher
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

#include "Modules/Module_Sys_Syscom.hpp"

#include <boost/lexical_cast.hpp>

#include "MachineBase/GeneralOperations.hpp"
#include "MachineBase/RLMachine.hpp"
#include "MachineBase/RLModule.hpp"
#include "MachineBase/RLOperation.hpp"
#include "MachineBase/RLOperation/RLOp_Store.hpp"
#include "MachineBase/RLOperation/References.hpp"
#include "Systems/Base/EventSystem.hpp"
#include "Systems/Base/System.hpp"

using namespace std;
using boost::lexical_cast;

// -----------------------------------------------------------------------

namespace {

struct ContextMenu : public RLOp_Void_Void {
  void operator()(RLMachine& machine) {
    // Based off of ALMA, it appears that we also are responsible for flushing
    // clicks.
    machine.system().event().flushMouseClicks();

    machine.system().showSyscomMenu(machine);
  }
};

struct SyscomEnabled : public RLOp_Store_1< IntConstant_T > {
  int operator()(RLMachine& machine, int num) {
    return machine.system().isSyscomEnabled(num);
  }
};

}  // namespace

// -----------------------------------------------------------------------

void addSysSyscomOpcodes(RLModule& m) {
  m.addOpcode(1210, 0, "ContextMenu", new ContextMenu);

  m.addOpcode(1211, 0, "EnableSyscom",
              callFunction(&System::enableSyscomEntry));
  m.addOpcode(1211, 1, "EnableSyscom",
              callFunction(&System::enableSyscom));

  m.addOpcode(1212, 0, "HideSyscom",
              callFunction(&System::hideSyscomEntry));
  m.addOpcode(1212, 1, "HideSyscom",
              callFunction(&System::hideSyscom));

  m.addOpcode(1213, 0, "DisableSyscom",
              callFunction(&System::disableSyscomEntry));

  m.addOpcode(1214, 0, "SyscomEnabled", new SyscomEnabled);

  m.addOpcode(1215, 0, "InvokeSyscom", callFunction(&System::invokeSyscom));
  m.addUnsupportedOpcode(1215, 1, "InvokeSyscom");
  m.addUnsupportedOpcode(1216, 0, "ReadSyscom");
}
