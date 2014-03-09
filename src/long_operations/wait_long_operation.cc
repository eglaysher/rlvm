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
#include "Systems/Base/EventListener.hpp"
#include "Systems/Base/EventSystem.hpp"
#include "Systems/Base/GraphicsSystem.hpp"
#include "Systems/Base/Rect.hpp"
#include "Systems/Base/System.hpp"
#include "Systems/Base/TextSystem.hpp"

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
      has_sleep_time_provider_(false),
      break_on_ctrl_pressed_(machine.system().text().ctrlKeySkip()),
      ctrl_pressed_(false),
      mouse_moved_(false),
      save_click_location_(false) {}

WaitLongOperation::~WaitLongOperation() {}

void WaitLongOperation::waitMilliseconds(unsigned int time) {
  wait_until_target_time_ = true;
  target_time_ = machine_.system().event().getTicks() + time;
}

void WaitLongOperation::breakOnClicks() { break_on_clicks_ = true; }

void WaitLongOperation::breakOnEvent(const std::function<bool()>& function) {
  break_on_event_ = true;
  event_function_ = function;
}

void WaitLongOperation::saveClickLocation(IntReferenceIterator x,
                                          IntReferenceIterator y) {
  break_on_clicks_ = true;
  save_click_location_ = true;
  x_ = x;
  y_ = y;
}

void WaitLongOperation::mouseMotion(const Point&) { mouse_moved_ = true; }

bool WaitLongOperation::mouseButtonStateChanged(MouseButton mouseButton,
                                                bool pressed) {
  if (pressed && break_on_clicks_) {
    if (save_click_location_ &&
        (mouseButton == MOUSE_LEFT || mouseButton == MOUSE_RIGHT)) {
      recordMouseCursorPosition();
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

bool WaitLongOperation::keyStateChanged(KeyCode keyCode, bool pressed) {
  if (pressed && break_on_ctrl_pressed_ &&
      (keyCode == RLKEY_RCTRL || keyCode == RLKEY_LCTRL)) {
    ctrl_pressed_ = true;
    return true;
  }

  return false;
}

void WaitLongOperation::recordMouseCursorPosition() {
  Point location = machine_.system().event().getCursorPos();
  *x_ = location.x();
  *y_ = location.y();
}

bool WaitLongOperation::operator()(RLMachine& machine) {
  bool done = ctrl_pressed_ || machine.system().fastForward();

  if (!done && wait_until_target_time_) {
    done = machine.system().event().getTicks() > target_time_;
  }

  if (!done && break_on_event_) {
    done = event_function_();
  }

  GraphicsSystem& graphics = machine.system().graphics();
  if (mouse_moved_) {
    graphics.markScreenAsDirty(GUT_MOUSE_MOTION);
    mouse_moved_ = false;
  } else if (graphics.objectStateDirty()) {
    graphics.markScreenAsDirty(GUT_DISPLAY_OBJ);
  }

  if (break_on_clicks_) {
    if (button_pressed_) {
      done = true;
      machine.setStoreRegister(button_pressed_);
    } else if (done) {
      // TODO(erg): this is fishy. shouldn't we record when clicked?
      if (save_click_location_)
        recordMouseCursorPosition();
      machine.setStoreRegister(0);
    }
  }

  return done;
}
