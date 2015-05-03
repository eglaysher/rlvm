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
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
//
// -----------------------------------------------------------------------

#ifndef SRC_MODULES_MODULE_SYS_SAVE_H_
#define SRC_MODULES_MODULE_SYS_SAVE_H_

#include "machine/rloperation.h"

class RLMachine;
class RLModule;
class System;

// Public access to the frame counter operations. Meant to be called
// from the constructor of SysModule.
void AddSysSaveOpcodes(RLModule& module);

// -----------------------------------------------------------------------

// Implementation of fun load<1:Sys:03009, 0> ('slot'): Loads data
// from a save game slot.
//
// Internally, load is fairly complex, consisting of several
// LongOperations because we can't rely on normal flow control because
// we're going to nuke the call stack and system memory in
// LoadingGame.
struct Sys_load : public RLOpcode<IntConstant_T> {
  virtual bool AdvanceInstructionPointer() override;

  // Main entrypoint into the load command. Simply sets the callstack
  // up so that we will fade to black, clear the screen and render,
  // and then enter the next stage, the LongOperation LoadingGame.
  virtual void operator()(RLMachine& machine, int slot);
};

#endif  // SRC_MODULES_MODULE_SYS_SAVE_H_
