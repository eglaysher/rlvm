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

#ifndef SRC_SYSTEMS_BASE_TEXT_WINDOW_BUTTON_H_
#define SRC_SYSTEMS_BASE_TEXT_WINDOW_BUTTON_H_

#include <functional>
#include <memory>
#include <vector>

#include "base/notification_observer.h"
#include "base/notification_registrar.h"
#include "base/notification_type.h"
#include "libreallive/gameexe.h"

class Point;
class RLMachine;
class Rect;
class Surface;
class System;
class TextWindow;

class TextWindowButton {
 public:
  explicit TextWindowButton(System& system);
  TextWindowButton(System& system,
                   bool use,
                   GameexeInterpretObject location_box);
  virtual ~TextWindowButton();

  // Returns the absolute screen coordinate of this button.
  Rect Location(TextWindow& window);

  // Checks to see if this is a valid, used button
  bool IsValid() const;

  // Track the mouse position to see if we need to alter our state
  void SetMousePosition(TextWindow& window, const Point& pos);

  bool HandleMouseClick(RLMachine& machine,
                        TextWindow& window,
                        const Point& pos,
                        bool pressed);

  //
  void Render(TextWindow& window,
              const std::shared_ptr<const Surface>& buttons,
              int base_pattern);

  // Called when the button is pressed
  virtual void ButtonPressed() {}

  // Called by other execute() calls while the System object has its
  // turn to do any updating
  virtual void Execute() {}

  // Called when the button is released
  virtual void ButtonReleased(RLMachine& machine) {}

 protected:
  System& system_;

  std::vector<int> location_;
  int state_;
};

// -----------------------------------------------------------------------

class ActionTextWindowButton : public TextWindowButton {
 public:
  typedef std::function<void(void)> CallbackFunction;

 public:
  ActionTextWindowButton(System& system,
                         bool use,
                         GameexeInterpretObject location_box,
                         CallbackFunction action);
  virtual ~ActionTextWindowButton();

  virtual void ButtonReleased(RLMachine& machine) override;

 private:
  CallbackFunction action_;
};

// -----------------------------------------------------------------------

class ActivationTextWindowButton : public TextWindowButton,
                                   public NotificationObserver {
 public:
  typedef std::function<void(int)> CallbackFunction;

 public:
  ActivationTextWindowButton(System& system,
                             bool use,
                             GameexeInterpretObject location_box,
                             CallbackFunction setter);
  virtual ~ActivationTextWindowButton();

  virtual void ButtonReleased(RLMachine& machine) override;

  void SetEnabled(bool enabled);

  void SetEnabledNotification(NotificationType enabled_listener);
  void SetChangeNotification(NotificationType change_listener);

 private:
  void SetState();

  // NotificationObserver:
  virtual void Observe(NotificationType type,
                       const NotificationSource& source,
                       const NotificationDetails& details) override;

  CallbackFunction on_set_;
  bool on_;
  bool enabled_;

  // The type that we listen for to change our activated state.
  NotificationType enabled_listener_;
  NotificationType change_listener_;
  NotificationRegistrar registrar_;
};

// -----------------------------------------------------------------------

class RepeatActionWhileHoldingWindowButton : public TextWindowButton {
 public:
  typedef std::function<void(void)> CallbackFunction;

 public:
  RepeatActionWhileHoldingWindowButton(System& system,
                                       bool use,
                                       GameexeInterpretObject location_box,
                                       CallbackFunction callback,
                                       unsigned int time_between_invocations);
  virtual ~RepeatActionWhileHoldingWindowButton();

  virtual void ButtonPressed() override;
  virtual void Execute() override;
  virtual void ButtonReleased(RLMachine& machine) override;

 private:
  CallbackFunction callback_;
  bool held_down_;
  unsigned int last_invocation_;
  unsigned int time_between_invocations_;
};

// -----------------------------------------------------------------------

class ExbtnWindowButton : public TextWindowButton {
 public:
  ExbtnWindowButton(System& system,
                    bool use,
                    GameexeInterpretObject location_box,
                    GameexeInterpretObject to_call);
  virtual ~ExbtnWindowButton();

  virtual void ButtonReleased(RLMachine& machine) override;

 private:
  int scenario_;
  int entrypoint_;
};

#endif  // SRC_SYSTEMS_BASE_TEXT_WINDOW_BUTTON_H_
