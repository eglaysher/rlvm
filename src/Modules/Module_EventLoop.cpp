// -*- Mode: C++; tab-width:2; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi:tw=80:et:ts=2:sts=2
//
// -----------------------------------------------------------------------
//
// This file is part of RLVM, a RealLive virtual machine clone.
//
// -----------------------------------------------------------------------
//
// Copyright (C) 2006 Elliot Glaysher
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

#include "Modules/Module_EventLoop.hpp"
#include "MachineBase/RLMachine.hpp"
#include "MachineBase/GeneralOperations.hpp"

// -----------------------------------------------------------------------

EventLoopModule::EventLoopModule()
  : RLModule("EventLoop", 0, 4)
{
  addUnsupportedOpcode(120, 0, "SetInterrupt");
  addUnsupportedOpcode(121, 0, "ClearInterrupt");
  addUnsupportedOpcode(303, 0, "yield");

  /// Theoretically the same as rtl, but we don't really know.
  addOpcode(300, 0, "rtlButton", callFunction(&RLMachine::returnFromFarcall));
  addOpcode(301, 0, "rtlCancel", callFunction(&RLMachine::returnFromFarcall));
  addOpcode(302, 0, "rtlSystem", callFunction(&RLMachine::returnFromFarcall));

  addUnsupportedOpcode(1000, 0, "ShowBackground");
  addUnsupportedOpcode(1100, 0, "SetSkipMode");
  addUnsupportedOpcode(1101, 0, "ClearSkipMode");
  addUnsupportedOpcode(1102, 0, "SkipMode");

  // opcode<0:4:1202, 0> and opcode<0:4:1200, 0> are used in the CLANNAD menu
  // system; no idea what they do.
}
