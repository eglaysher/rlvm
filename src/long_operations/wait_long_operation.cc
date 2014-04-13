// -*- Mode: C++; tab-width:2; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi:tw=80:et:ts=2:sts=2
//
// -----------------------------------------------------------------------
//
// This file is part of RLVM, a RealLive virtual machine clone.
//
// -----------------------------------------------------------------------
//
// Copyright (C) 2009 Elliot Glaysher
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

#include "long_operations/wait_long_operation.h"

#include "machine/rlmachine.h"
#include "systems/base/event_listener.h"
#include "systems/base/event_system.h"
#include "systems/base/graphics_system.h"
#include "systems/base/rect.h"
#include "systems/base/system.h"
#include "systems/base/text_system.h"

// -----------------------------------------------------------------------
// WaitLongOperation
// -----------------------------------------------------------------------
WaitLongOperation::WaitLongOperation(RLMachine& machine)
    : machine_(machine),
      wait_until_target_time_(false),
      target_time_(0),
      break_on_clicks_(false),
      button_pressed_(0),
      break_on_event_(false),
      break_on_ctrl_pressed_(machine.system().text().ctrl_key_skip()),
      ctrl_pressed_(false),
      mouse_moved_(false),
      save_click_location_(false) {}

WaitLongOperation::~WaitLongOperation() {}

void WaitLongOperation::WaitMilliseconds(unsigned int time) {
  wait_until_target_time_ = true;
  target_time_ = machine_.system().event().GetTicks() + time;
}

void WaitLongOperation::BreakOnClicks() { break_on_clicks_ = true; }

void WaitLongOperation::BreakOnEvent(const std::function<bool()>& function) {
  break_on_event_ = true;
  event_function_ = function;
}

void WaitLongOperation::SaveClickLocation(IntReferenceIterator x,
                                          IntReferenceIterator y) {
  break_on_clicks_ = true;
  save_click_location_ = true;
  x_ = x;
  y_ = y;
}

void WaitLongOperation::MouseMotion(const Point&) { mouse_moved_ = true; }

bool WaitLongOperation::MouseButtonStateChanged(MouseButton mouseButton,
                                                bool pressed) {
  if (pressed && break_on_clicks_) {
    if (save_click_location_ &&
        (mouseButton == MOUSE_LEFT || mouseButton == MOUSE_RIGHT)) {
      RecordMouseCursorPosition();
    }

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

bool WaitLongOperation::KeyStateChanged(KeyCode keyCode, bool pressed) {
  if (pressed && break_on_ctrl_pressed_ &&
      (keyCode == RLKEY_RCTRL || keyCode == RLKEY_LCTRL)) {
    ctrl_pressed_ = true;
    return true;
  }

  return false;
}

void WaitLongOperation::RecordMouseCursorPosition() {
  Point location = machine_.system().event().GetCursorPos();
  *x_ = location.x();
  *y_ = location.y();
}

bool WaitLongOperation::operator()(RLMachine& machine) {
  bool done = ctrl_pressed_ || machine.system().ShouldFastForward();

  if (!done && wait_until_target_time_) {
    done = machine.system().event().GetTicks() > target_time_;
  }

  if (!done && break_on_event_) {
    done = event_function_();
  }

  GraphicsSystem& graphics = machine.system().graphics();
  if (mouse_moved_) {
    graphics.MarkScreenAsDirty(GUT_MOUSE_MOTION);
    mouse_moved_ = false;
  } else if (graphics.object_state_dirty()) {
    graphics.MarkScreenAsDirty(GUT_DISPLAY_OBJ);
  }

  if (break_on_clicks_) {
    if (button_pressed_) {
      done = true;
      machine.set_store_register(button_pressed_);
    } else if (done) {
      // TODO(erg): this is fishy. shouldn't we record when clicked?
      if (save_click_location_)
        RecordMouseCursorPosition();
      machine.set_store_register(0);
    }
  }

  return done;
}
