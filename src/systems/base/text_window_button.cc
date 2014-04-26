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

#include "systems/base/text_window_button.h"

#include <stdexcept>
#include <vector>

#include "base/notification_details.h"
#include "base/notification_service.h"
#include "machine/long_operation.h"
#include "machine/rlmachine.h"
#include "systems/base/event_system.h"
#include "systems/base/graphics_system.h"
#include "systems/base/rect.h"
#include "systems/base/surface.h"
#include "systems/base/system.h"
#include "systems/base/system_error.h"
#include "systems/base/text_system.h"
#include "systems/base/text_window.h"

// Describes the state of a Waku button
enum ButtonState {
  BUTTONSTATE_BUTTON_NOT_USED = -1,
  BUTTONSTATE_NORMAL = 0,
  BUTTONSTATE_HIGHLIGHTED = 1,
  BUTTONSTATE_PRESSED = 2,
  BUTTONSTATE_ACTIVATED = 4,
  BUTTONSTATE_DISABLED = 3
};

// -----------------------------------------------------------------------
// TextWindowButton
// -----------------------------------------------------------------------

TextWindowButton::TextWindowButton(System& system)
    : system_(system), state_(BUTTONSTATE_BUTTON_NOT_USED) {}

TextWindowButton::TextWindowButton(System& system,
                                   bool use_this_button,
                                   GameexeInterpretObject location_box)
    : system_(system), state_(BUTTONSTATE_BUTTON_NOT_USED) {
  if (use_this_button && location_box.Exists()) {
    std::vector<int> z = location_box;
    location_ = location_box;
    state_ = BUTTONSTATE_NORMAL;
  }
}

TextWindowButton::~TextWindowButton() {}

Rect TextWindowButton::Location(TextWindow& window) {
  int type = location_.at(0);
  Size size(location_.at(3), location_.at(4));
  Rect win_rect = window.GetWindowRect();

  switch (type) {
    case 0: {
      // Top and left
      Point origin = win_rect.origin() + Size(location_.at(1), location_.at(2));
      return Rect(origin, size);
    }
    case 1: {
      // Top and right
      Point origin = win_rect.origin() +
                     Size(-location_.at(1), location_.at(2)) +
                     Size(win_rect.size().width() - size.width(), 0);
      return Rect(origin, size);
    }
    // TODO(erg): While 0 is used in pretty much everything I've tried, and 1
    // is used in the Maiden Halo demo, I'm not certain about these other two
    // calculations. Both KareKare games screw up here, but it may be because
    // of the win_rect. Needs further investigation.
    case 2: {
      // Bottom and left
      Point origin = win_rect.origin() +
                     Size(location_.at(1), -location_.at(2)) +
                     Size(0, win_rect.size().height() - size.height());
      return Rect(origin, size);
    }
    case 3: {
      // Bottom and right
      Point origin = win_rect.origin() +
                     Size(-location_.at(1), -location_.at(2)) +
                     Size(win_rect.size().width() - size.width(),
                          win_rect.size().height() - size.height());
      return Rect(origin, size);
    }
    default: { throw SystemError("Unsupported coordinate system"); }
  }
}

bool TextWindowButton::IsValid() const {
  return state_ != BUTTONSTATE_BUTTON_NOT_USED && location_.size() == 5 &&
         !(location_[0] == 0 && location_[1] == 0 && location_[2] == 0 &&
           location_[3] == 0 && location_[4] == 0);
}

void TextWindowButton::SetMousePosition(TextWindow& window, const Point& pos) {
  if (state_ == BUTTONSTATE_DISABLED)
    return;

  if (IsValid()) {
    int orig_state = state_;
    bool in_box = Location(window).Contains(pos);
    if (in_box && state_ == BUTTONSTATE_NORMAL)
      state_ = BUTTONSTATE_HIGHLIGHTED;
    else if (!in_box && state_ == BUTTONSTATE_HIGHLIGHTED)
      state_ = BUTTONSTATE_NORMAL;
    else if (!in_box && state_ == BUTTONSTATE_PRESSED)
      state_ = BUTTONSTATE_NORMAL;

    if (orig_state != state_)
      system_.graphics().MarkScreenAsDirty(GUT_TEXTSYS);
  }
}

bool TextWindowButton::HandleMouseClick(RLMachine& machine,
                                        TextWindow& window,
                                        const Point& pos,
                                        bool pressed) {
  if (state_ == BUTTONSTATE_DISABLED)
    return false;

  if (IsValid()) {
    bool in_box = Location(window).Contains(pos);

    if (in_box) {
      // Perform any activation
      if (pressed) {
        state_ = BUTTONSTATE_PRESSED;
        ButtonPressed();
      } else {
        state_ = BUTTONSTATE_HIGHLIGHTED;
        ButtonReleased(machine);
      }

      system_.graphics().MarkScreenAsDirty(GUT_TEXTSYS);

      return true;
    }
  }

  return false;
}

void TextWindowButton::Render(TextWindow& window,
                              const std::shared_ptr<const Surface>& buttons,
                              int base_pattern) {
  if (IsValid()) {
    Surface::GrpRect rect = buttons->GetPattern(base_pattern + state_);
    if (!(rect.rect.is_empty())) {
      Rect dest = Rect(Location(window).origin(), rect.rect.size());
      buttons->RenderToScreen(rect.rect, dest, 255);
    }
  }
}

// -----------------------------------------------------------------------
// ActionTextWindowButton
// -----------------------------------------------------------------------

ActionTextWindowButton::ActionTextWindowButton(
    System& system,
    bool use,
    GameexeInterpretObject location_box,
    CallbackFunction action)
    : TextWindowButton(system, use, location_box), action_(action) {}

ActionTextWindowButton::~ActionTextWindowButton() {}

void ActionTextWindowButton::ButtonReleased(RLMachine& machine) { action_(); }

// -----------------------------------------------------------------------
// ActivationTextWindowButton
// -----------------------------------------------------------------------

ActivationTextWindowButton::ActivationTextWindowButton(
    System& system,
    bool use,
    GameexeInterpretObject location_box,
    CallbackFunction setter)
    : TextWindowButton(system, use, location_box),
      on_set_(setter),
      on_(false),
      enabled_(true),
      enabled_listener_(static_cast<NotificationType::Type>(-1)),
      change_listener_(static_cast<NotificationType::Type>(-1)) {}

ActivationTextWindowButton::~ActivationTextWindowButton() {}

void ActivationTextWindowButton::ButtonReleased(RLMachine& machine) {
  if (enabled_) {
    if (on_)
      on_set_(false);
    else
      on_set_(true);
  }
}

void ActivationTextWindowButton::SetEnabled(bool enabled) {
  enabled_ = enabled;
  SetState();
}

void ActivationTextWindowButton::SetEnabledNotification(
    NotificationType enabled_listener) {
  enabled_listener_ = enabled_listener;
  registrar_.Add(this, enabled_listener_, NotificationService::AllSources());
}

void ActivationTextWindowButton::SetChangeNotification(
    NotificationType change_listener) {
  change_listener_ = change_listener;
  registrar_.Add(this, change_listener_, NotificationService::AllSources());
}

void ActivationTextWindowButton::SetState() {
  if (enabled_)
    state_ = on_ ? BUTTONSTATE_ACTIVATED : BUTTONSTATE_NORMAL;
  else
    state_ = BUTTONSTATE_DISABLED;
}

void ActivationTextWindowButton::Observe(NotificationType type,
                                         const NotificationSource& source,
                                         const NotificationDetails& details) {
  if (type == enabled_listener_) {
    Details<int> i(details);
    enabled_ = *i.ptr();
  } else if (type == change_listener_) {
    Details<int> i(details);
    on_ = *i.ptr();
  }

  SetState();
}

// -----------------------------------------------------------------------
// RepeatActionWhileHoldingWindowButton
// -----------------------------------------------------------------------

RepeatActionWhileHoldingWindowButton::RepeatActionWhileHoldingWindowButton(
    System& system,
    bool use,
    GameexeInterpretObject location_box,
    CallbackFunction callback,
    unsigned int time_between_invocations)
    : TextWindowButton(system, use, location_box),
      callback_(callback),
      held_down_(false),
      time_between_invocations_(time_between_invocations) {}

RepeatActionWhileHoldingWindowButton::~RepeatActionWhileHoldingWindowButton() {}

void RepeatActionWhileHoldingWindowButton::ButtonPressed() {
  held_down_ = true;

  callback_();
  last_invocation_ = system_.event().GetTicks();
}

void RepeatActionWhileHoldingWindowButton::Execute() {
  if (held_down_) {
    unsigned int cur_time = system_.event().GetTicks();

    if (last_invocation_ + time_between_invocations_ > cur_time) {
      callback_();
      last_invocation_ = cur_time;
    }
  }
}

void RepeatActionWhileHoldingWindowButton::ButtonReleased(RLMachine& machine) {
  held_down_ = false;
}

// -----------------------------------------------------------------------
// ExbtnWindowButton
// -----------------------------------------------------------------------

ExbtnWindowButton::ExbtnWindowButton(System& system,
                                     bool use,
                                     GameexeInterpretObject location_box,
                                     GameexeInterpretObject to_call)
    : TextWindowButton(system, use, location_box),
      scenario_(0),
      entrypoint_(0) {
  if (location_box.Exists() && to_call.Exists()) {
    std::vector<int> farcall = to_call;
    scenario_ = farcall.at(0);
    entrypoint_ = farcall.at(1);
  }
}

ExbtnWindowButton::~ExbtnWindowButton() {}

void ExbtnWindowButton::ButtonReleased(RLMachine& machine) {
  // Hide all text boxes when entering an Exbtn
  machine.system().text().set_system_visible(false);

  // Push a LongOperation onto the stack which will restore
  // visibility when we return from this Exbtn call
  machine.PushLongOperation(new RestoreTextSystemVisibility);
  machine.Farcall(scenario_, entrypoint_);
}
