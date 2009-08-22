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
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
//
// -----------------------------------------------------------------------

#include "Precompiled.hpp"

// -----------------------------------------------------------------------

#include "Module_Sys_Frame.hpp"

#include "MachineBase/RLOperation.hpp"
#include "MachineBase/RLOperation/DefaultValue.hpp"
#include "MachineBase/RLOperation/RLOp_Store.hpp"

#include "MachineBase/RLMachine.hpp"
#include "MachineBase/RLModule.hpp"
#include "MachineBase/LongOperation.hpp"

#include "Systems/Base/System.hpp"
#include "Systems/Base/EventSystem.hpp"
#include "Systems/Base/GraphicsSystem.hpp"
#include "Systems/Base/EventListener.hpp"
#include "Systems/Base/FrameCounter.hpp"

#include <boost/numeric/conversion/cast.hpp>

//#include <iostream>
using namespace std;
using boost::numeric_cast;

struct Sys_ResetTimer : public RLOp_Void_1< DefaultIntValue_T< 0 > > {
  const int layer_;
  Sys_ResetTimer(const int in) : layer_(in) {}

  void operator()(RLMachine& machine, int counter) {
    EventSystem& es = machine.system().event();
    es.getTimer(layer_, counter).set(es);
  }
};

// -----------------------------------------------------------------------

struct LongOp_time : public LongOperation {
  const int layer_;
  const int counter_;
  const unsigned int target_time_;
  const bool cancel_on_click_;

  int button_pressed_;
  bool mouse_moved_;

  LongOp_time(RLMachine& machine, int layer, int counter, int time,
              bool cancelOnClick)
    : layer_(layer), counter_(counter), target_time_(time),
      cancel_on_click_(cancelOnClick), button_pressed_(0),
      mouse_moved_(false) {}

  void mouseMotion(const Point&) {
    mouse_moved_ = true;
  }

  // Overridden from EventListener:
  virtual bool mouseButtonStateChanged(MouseButton mouseButton, bool pressed) {
    if (pressed) {
      if (mouseButton == MOUSE_LEFT) {
        button_pressed_ = 1;
        return true;
      } else if (mouseButton == MOUSE_RIGHT) {
        button_pressed_ = -1;
        return true;
      }
    }

    return false;
  }

  // Overridden from LongOperation:
  bool operator()(RLMachine& machine) {
    EventSystem& es = machine.system().event();
    bool done = false;

    if (mouse_moved_) {
      machine.system().graphics().markScreenAsDirty(GUT_MOUSE_MOTION);
      mouse_moved_ = false;
    }

    // First check to see if we're done because of time.
    if (es.getTimer(layer_, counter_).read(es) > target_time_)
      done = true;

    if (cancel_on_click_) {
      // The underlying timeC returns a value. Manually set that
      // value here.
      if (button_pressed_) {
        done = true;
        machine.setStoreRegister(button_pressed_);
      } else if (done) {
        machine.setStoreRegister(0);
      }
    }

    return done;
  }
};

// -----------------------------------------------------------------------

struct Sys_time : public RLOp_Void_2< IntConstant_T, DefaultIntValue_T< 0 > > {
  const int layer_;
  const bool in_time_c_;
  Sys_time(const int in, const bool timeC) : layer_(in), in_time_c_(timeC) {}

  void operator()(RLMachine& machine, int time, int counter) {
    EventSystem& es = machine.system().event();

    if (es.getTimer(layer_, counter).read(es) < numeric_cast<unsigned int>(time)) {
      machine.pushLongOperation(new LongOp_time(machine, layer_, counter,
                                                time, in_time_c_));
    }
  }
};

// -----------------------------------------------------------------------

struct Sys_Timer : public RLOp_Store_1< DefaultIntValue_T<0> > {
  const int layer_;
  Sys_Timer(const int in) : layer_(in) {}

  int operator()(RLMachine& machine, int counter) {
    EventSystem& es = machine.system().event();
    return es.getTimer(layer_, counter).read(es);
  }
};

// -----------------------------------------------------------------------

struct Sys_CmpTimer : public RLOp_Store_2< IntConstant_T, DefaultIntValue_T<0> > {
  const int layer_;
  Sys_CmpTimer(const int in) : layer_(in) {}

  int operator()(RLMachine& machine, int val, int counter) {
    EventSystem& es = machine.system().event();
    return es.getTimer(layer_, counter).read(es) > numeric_cast<unsigned int>(val);
  }
};

// -----------------------------------------------------------------------

struct Sys_SetTimer : public RLOp_Void_2< IntConstant_T, DefaultIntValue_T<0> > {
  const int layer_;
  Sys_SetTimer(const int in) : layer_(in) {}

  void operator()(RLMachine& machine, int val, int counter) {
    EventSystem& es = machine.system().event();
    es.getTimer(layer_, counter).set(es, val);
  }
};


// -----------------------------------------------------------------------

void addSysTimerOpcodes(RLModule& m) {
  m.addOpcode(110, 0, "ResetTimer", new Sys_ResetTimer(0));
  m.addOpcode(110, 1, "ResetTimer", new Sys_ResetTimer(0));
  m.addOpcode(111, 0, "time", new Sys_time(0, false));
  m.addOpcode(111, 1, "time", new Sys_time(0, false));
  m.addOpcode(112, 0, "timeC", new Sys_time(0, true));
  m.addOpcode(112, 1, "timeC", new Sys_time(0, true));
  m.addOpcode(114, 0, "Timer", new Sys_Timer(0));
  m.addOpcode(114, 1, "Timer", new Sys_Timer(0));
  m.addOpcode(115, 0, "CmpTimer", new Sys_CmpTimer(0));
  m.addOpcode(115, 1, "CmpTimer", new Sys_CmpTimer(0));
  m.addOpcode(116, 0, "CmpTimer", new Sys_SetTimer(0));
  m.addOpcode(116, 1, "CmpTimer", new Sys_SetTimer(0));

  m.addOpcode(120, 0, "ResetExTimer", new Sys_ResetTimer(1));
  m.addOpcode(120, 1, "ResetExTimer", new Sys_ResetTimer(1));
  m.addOpcode(121, 0, "timeEx", new Sys_time(1, false));
  m.addOpcode(121, 1, "timeEx", new Sys_time(1, false));
  m.addOpcode(122, 0, "timeExC", new Sys_time(1, true));
  m.addOpcode(122, 1, "timeExC", new Sys_time(1, true));
  m.addOpcode(124, 0, "ExTimer", new Sys_Timer(1));
  m.addOpcode(124, 1, "ExTimer", new Sys_Timer(1));
  m.addOpcode(125, 0, "CmpExTimer", new Sys_CmpTimer(1));
  m.addOpcode(125, 1, "CmpExTimer", new Sys_CmpTimer(1));
  m.addOpcode(126, 0, "SetExTimer", new Sys_SetTimer(1));
  m.addOpcode(126, 1, "SetExTimer", new Sys_SetTimer(1));
}
