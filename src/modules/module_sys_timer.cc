// -*- Mode: C++; tab-width:2; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi:tw=80:et:ts=2:sts=2
//
// -----------------------------------------------------------------------
//
// This file is part of RLVM, a RealLive virtual machine clone.
//
// -----------------------------------------------------------------------
//
// Copyright (C) 2006, 2007 Elliot Glaysher
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

#include "modules/module_sys_frame.h"

#include <boost/numeric/conversion/cast.hpp>

#include "long_operations/wait_long_operation.h"
#include "machine/long_operation.h"
#include "machine/rlmachine.h"
#include "machine/rlmodule.h"
#include "machine/rloperation.h"
#include "machine/rloperation/default_value.h"
#include "machine/rloperation/rlop_store.h"
#include "systems/base/event_listener.h"
#include "systems/base/event_system.h"
#include "systems/base/frame_counter.h"
#include "systems/base/graphics_system.h"
#include "systems/base/system.h"

using boost::numeric_cast;

namespace {

struct ResetTimer : public RLOpcode<DefaultIntValue_T<0>> {
  const int layer_;
  explicit ResetTimer(const int in) : layer_(in) {}

  void operator()(RLMachine& machine, int counter) {
    EventSystem& es = machine.system().event();
    es.GetTimer(layer_, counter).Set(es);
  }
};

bool TimerIsDone(RLMachine& machine,
                 int layer,
                 int counter,
                 unsigned int target_time) {
  EventSystem& es = machine.system().event();
  return es.GetTimer(layer, counter).Read(es) > target_time;
}

struct Sys_time : public RLOpcode<IntConstant_T, DefaultIntValue_T<0>> {
  const int layer_;
  const bool in_time_c_;
  Sys_time(const int in, const bool timeC) : layer_(in), in_time_c_(timeC) {}

  void operator()(RLMachine& machine, int time, int counter) {
    EventSystem& es = machine.system().event();

    if (es.GetTimer(layer_, counter).Read(es) <
        numeric_cast<unsigned int>(time)) {
      WaitLongOperation* wait_op = new WaitLongOperation(machine);
      if (in_time_c_)
        wait_op->BreakOnClicks();
      wait_op->BreakOnEvent(
          std::bind(TimerIsDone, std::ref(machine), layer_, counter, time));
      machine.PushLongOperation(wait_op);
    }
  }
};

struct Timer : public RLStoreOpcode<DefaultIntValue_T<0>> {
  const int layer_;
  explicit Timer(const int in) : layer_(in) {}

  int operator()(RLMachine& machine, int counter) {
    EventSystem& es = machine.system().event();
    return es.GetTimer(layer_, counter).Read(es);
  }
};

struct CmpTimer : public RLStoreOpcode<IntConstant_T, DefaultIntValue_T<0>> {
  const int layer_;
  explicit CmpTimer(const int in) : layer_(in) {}

  int operator()(RLMachine& machine, int val, int counter) {
    EventSystem& es = machine.system().event();
    return es.GetTimer(layer_, counter).Read(es) > val;
  }
};

struct SetTimer : public RLOpcode<IntConstant_T, DefaultIntValue_T<0>> {
  const int layer_;
  explicit SetTimer(const int in) : layer_(in) {}

  void operator()(RLMachine& machine, int val, int counter) {
    EventSystem& es = machine.system().event();
    es.GetTimer(layer_, counter).Set(es, val);
  }
};

}  // namespace

// -----------------------------------------------------------------------

void AddSysTimerOpcodes(RLModule& m) {
  m.AddOpcode(110, 0, "ResetTimer", new ResetTimer(0));
  m.AddOpcode(110, 1, "ResetTimer", new ResetTimer(0));
  m.AddOpcode(111, 0, "time", new Sys_time(0, false));
  m.AddOpcode(111, 1, "time", new Sys_time(0, false));
  m.AddOpcode(112, 0, "timeC", new Sys_time(0, true));
  m.AddOpcode(112, 1, "timeC", new Sys_time(0, true));
  m.AddOpcode(114, 0, "Timer", new Timer(0));
  m.AddOpcode(114, 1, "Timer", new Timer(0));
  m.AddOpcode(115, 0, "CmpTimer", new CmpTimer(0));
  m.AddOpcode(115, 1, "CmpTimer", new CmpTimer(0));
  m.AddOpcode(116, 0, "CmpTimer", new SetTimer(0));
  m.AddOpcode(116, 1, "CmpTimer", new SetTimer(0));

  m.AddOpcode(120, 0, "ResetExTimer", new ResetTimer(1));
  m.AddOpcode(120, 1, "ResetExTimer", new ResetTimer(1));
  m.AddOpcode(121, 0, "timeEx", new Sys_time(1, false));
  m.AddOpcode(121, 1, "timeEx", new Sys_time(1, false));
  m.AddOpcode(122, 0, "timeExC", new Sys_time(1, true));
  m.AddOpcode(122, 1, "timeExC", new Sys_time(1, true));
  m.AddOpcode(124, 0, "ExTimer", new Timer(1));
  m.AddOpcode(124, 1, "ExTimer", new Timer(1));
  m.AddOpcode(125, 0, "CmpExTimer", new CmpTimer(1));
  m.AddOpcode(125, 1, "CmpExTimer", new CmpTimer(1));
  m.AddOpcode(126, 0, "SetExTimer", new SetTimer(1));
  m.AddOpcode(126, 1, "SetExTimer", new SetTimer(1));
}
