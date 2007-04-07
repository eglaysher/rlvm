// This file is part of RLVM, a RealLive virtual machine clone.
//
// -----------------------------------------------------------------------
//
// Copyright (C) 2006 El Riot
//  
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
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

#include "Modules/Module_EventLoop.hpp"
#include "MachineBase/RLMachine.hpp"
#include "MachineBase/RLOperation.hpp"

/** 
 * Copy/pasted Jmp_rtl. There may be a difference, but it appears to
 * behave identically.
 */
struct EL_rtl : public RLOp_Void_Void {
  void operator()(RLMachine& machine) {
    machine.returnFromFarcall();
  }
};

// -----------------------------------------------------------------------

EventLoopModule::EventLoopModule()
  : RLModule("EventLoop", 0, 4)
{
  addOpcode(300, 0, new EL_rtl());
  addOpcode(301, 0, new EL_rtl());
  addOpcode(302, 0, new EL_rtl());
}
