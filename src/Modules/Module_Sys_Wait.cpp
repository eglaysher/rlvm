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
// -----------------------------------------------------------------------

#include "Precompiled.hpp"

// -----------------------------------------------------------------------

//#include "Modules/Module_Sys_Wait.hpp"

#include "MachineBase/RLMachine.hpp"
#include "MachineBase/RLModule.hpp"
#include "MachineBase/RLOperation.hpp"
#include "Systems/Base/EventListener.hpp"
#include "Systems/Base/System.hpp"
#include "Systems/Base/TextSystem.hpp"
#include "Systems/Base/EventSystem.hpp"
#include "Systems/Base/GraphicsSystem.hpp"
#include "Systems/Base/Rect.hpp"
#include "MachineBase/LongOperation.hpp"
#include "MachineBase/RLOperation/References.hpp"

// -----------------------------------------------------------------------

/**
 * LongOperation that handles any combenation of waiting, and getting a mouse
 * click.
 */
struct LongOp_wait : public LongOperation
{
  RLMachine& machine_;

  bool wait_until_target_time_;
  unsigned int target_time_;

  bool break_on_clicks_;
  int button_pressed_;

  bool break_on_ctrl_pressed_;
  bool ctrl_pressed_;

  bool mouse_moved_;

  bool save_click_location_;
  IntReferenceIterator x_;
  IntReferenceIterator y_;

  LongOp_wait(RLMachine& machine)
    : machine_(machine),
      wait_until_target_time_(false),
      target_time_(0),
      break_on_clicks_(false), button_pressed_(0),
      break_on_ctrl_pressed_(machine.system().text().ctrlKeySkip()),
      ctrl_pressed_(false),
      mouse_moved_(false),
      save_click_location_(false)
  {}

  /// This instance should wait time milliseconds and then return.
  void waitMilliseconds(unsigned int time)
  {
    wait_until_target_time_ = true;
    target_time_ = machine_.system().event().getTicks() + time;
  }

  /// Whether we should exit this long operation on a click.
  void breakOnClicks()
  {
    break_on_clicks_ = true;
  }

  /// Whether we write out the location of a mouse click. Implies that we're
  /// breaking on mouse click.
  void saveClickLocation(IntReferenceIterator x, IntReferenceIterator y)
  {
    break_on_clicks_ = true;
    save_click_location_ = true;
    x_ = x;
    y_ = y;
  }

  void mouseMotion(const Point&)
  {
    mouse_moved_ = true;
  }

  // Overridden from EventListener:
  virtual bool mouseButtonStateChanged(MouseButton mouseButton, bool pressed)
  {
    if (pressed && break_on_clicks_) {
      if (save_click_location_ &&
          (mouseButton == MOUSE_LEFT ||
           mouseButton == MOUSE_RIGHT)) {
        recordMouseCursorPosition();
      }

      if (mouseButton == MOUSE_LEFT) {
        button_pressed_ = 1;
        return true;
      } else if(mouseButton == MOUSE_RIGHT) {
        button_pressed_ = -1;
        return true;
      }
    }

    return false;
  }

  virtual bool keyStateChanged(KeyCode keyCode, bool pressed)
  {
    if (pressed && break_on_ctrl_pressed_ &&
        (keyCode == RLKEY_RCTRL || keyCode == RLKEY_LCTRL)) {
      ctrl_pressed_ = true;
      return true;
    }

    return false;
  }

  void recordMouseCursorPosition()
  {
    Point location = machine_.system().event().getCursorPos();
    *x_ = location.x();
    *y_ = location.y();
  }

  // Overridden from LongOperation:
  bool operator()(RLMachine& machine)
  {
    bool done = ctrl_pressed_ || machine.system().fastForward();

    if (!done && wait_until_target_time_) {
      done = machine.system().event().getTicks() > target_time_;
    }

    if (mouse_moved_) {
      machine.system().graphics().markScreenAsDirty(GUT_MOUSE_MOTION);
      mouse_moved_ = false;
    }

    if (break_on_clicks_) {
      if (button_pressed_) {
        done = true;
        machine.setStoreRegister(button_pressed_);
      } else if(done) {
        if (save_click_location_)
          recordMouseCursorPosition();
        machine.setStoreRegister(0);
      }
    }

    return done;
  }
};

// -----------------------------------------------------------------------

struct Sys_wait : public RLOp_Void_1< IntConstant_T > {
  const bool cancelable_;

  Sys_wait(bool cancelable) : cancelable_(cancelable) {}

  /// Simply set the long operation
  void operator()(RLMachine& machine, int time) {
    LongOp_wait* wait_op = new LongOp_wait(machine);
    wait_op->waitMilliseconds(time);
    if (cancelable_)
      wait_op->breakOnClicks();

    machine.pushLongOperation(wait_op);
  }
};

// -----------------------------------------------------------------------

struct Sys_GetClick : public RLOp_Void_2< IntReference_T, IntReference_T > {
  void operator()(RLMachine& machine, IntReferenceIterator x,
                  IntReferenceIterator y) {
    LongOp_wait* wait_op = new LongOp_wait(machine);
    wait_op->saveClickLocation(x, y);
    machine.pushLongOperation(wait_op);
  }
};

// -----------------------------------------------------------------------

// fun WaitClick (store) <1:Sys:00132, 0> ('time', int 'X', int 'Y')
struct Sys_WaitClick
  : public RLOp_Void_3<IntConstant_T, IntReference_T, IntReference_T > {
  void operator()(RLMachine& machine, int time, IntReferenceIterator x,
                  IntReferenceIterator y) {
    LongOp_wait* wait_op = new LongOp_wait(machine);
    wait_op->waitMilliseconds(time);
    wait_op->saveClickLocation(x, y);
    machine.pushLongOperation(wait_op);
  }
};

// -----------------------------------------------------------------------

void addWaitAndMouseOpcodes(RLModule& m)
{
  m.addOpcode( 100, 0, "wait", new Sys_wait(false));
  m.addOpcode( 101, 0, "waitC", new Sys_wait(true));

  m.addOpcode( 131, 0, "GetClick", new Sys_GetClick);
  m.addOpcode( 131, 0, "WaitClick", new Sys_WaitClick);
}
