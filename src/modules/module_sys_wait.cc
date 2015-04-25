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

#include "modules/module_sys_wait.h"

#include "long_operations/wait_long_operation.h"
#include "machine/rlmachine.h"
#include "machine/rlmodule.h"
#include "machine/rloperation.h"
#include "machine/rloperation/references.h"

// -----------------------------------------------------------------------

struct Sys_wait : public RLOpcode<IntConstant_T> {
  const bool cancelable_;

  explicit Sys_wait(bool cancelable) : cancelable_(cancelable) {}

  void operator()(RLMachine& machine, int time) {
    // Simply set the long operation
    WaitLongOperation* wait_op = new WaitLongOperation(machine);
    wait_op->WaitMilliseconds(time);
    if (cancelable_)
      wait_op->BreakOnClicks();

    machine.PushLongOperation(wait_op);
  }
};

// -----------------------------------------------------------------------

struct Sys_GetClick : public RLOpcode<IntReference_T, IntReference_T> {
  void operator()(RLMachine& machine,
                  IntReferenceIterator x,
                  IntReferenceIterator y) {
    WaitLongOperation* wait_op = new WaitLongOperation(machine);
    wait_op->SaveClickLocation(x, y);
    machine.PushLongOperation(wait_op);
  }
};

// -----------------------------------------------------------------------

// fun WaitClick (store) <1:Sys:00132, 0> ('time', int 'X', int 'Y')
struct Sys_WaitClick
    : public RLOpcode<IntConstant_T, IntReference_T, IntReference_T> {
  void operator()(RLMachine& machine,
                  int time,
                  IntReferenceIterator x,
                  IntReferenceIterator y) {
    WaitLongOperation* wait_op = new WaitLongOperation(machine);
    wait_op->WaitMilliseconds(time);
    wait_op->SaveClickLocation(x, y);
    machine.PushLongOperation(wait_op);
  }
};

// -----------------------------------------------------------------------

void AddWaitAndMouseOpcodes(RLModule& m) {
  m.AddOpcode(100, 0, "wait", new Sys_wait(false));
  m.AddOpcode(101, 0, "waitC", new Sys_wait(true));

  m.AddOpcode(131, 0, "GetClick", new Sys_GetClick);
  m.AddOpcode(132, 0, "WaitClick", new Sys_WaitClick);
}
