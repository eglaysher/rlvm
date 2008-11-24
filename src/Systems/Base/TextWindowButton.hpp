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
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
//
// -----------------------------------------------------------------------

#ifndef __TextWindowButton_hpp__
#define __TextWindowButton_hpp__

#include <boost/function.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/utility.hpp>
#include <boost/signals/trackable.hpp>

#include "libReallive/gameexe.h"

class Point;
class RLMachine;
class Rect;
class Surface;
class TextWindow;

// -----------------------------------------------------------------------

class TextWindowButton : public boost::noncopyable
{
public:
  TextWindowButton();
  TextWindowButton(bool use, GameexeInterpretObject location_box);
  virtual ~TextWindowButton();

  /// Returns the absolute screen coordinate of this button.
  Rect location(TextWindow& window);

  /// Checks to see if this is a valid, used button
  bool isValid() const;

  /// Track the mouse position to see if we need to alter our state
  void setMousePosition(RLMachine& machine, TextWindow& window,
                        const Point& pos);

  bool handleMouseClick(RLMachine& machine, TextWindow& window,
                        const Point& pos, bool pressed);

  //
  void render(RLMachine& machine, TextWindow& window,
              const boost::shared_ptr<Surface>& buttons,
              int base_pattern);

  /// Called when the button is pressed
  virtual void buttonPressed() {}

  /// Called by other execute() calls while the System object has its
  /// turn to do any updating
  virtual void execute() {}

  /// Called when the button is released
  virtual void buttonReleased() {}

protected:
  std::vector<int> location_;
  int state_;
};

// -----------------------------------------------------------------------

class ActionTextWindowButton : public TextWindowButton
{
public:
  typedef boost::function<void(void)> CallbackFunction;

public:
  ActionTextWindowButton(bool use, GameexeInterpretObject location_box,
                         CallbackFunction action);
  ~ActionTextWindowButton();

  virtual void buttonReleased();

private:
  CallbackFunction action_;
};

// -----------------------------------------------------------------------

class ActivationTextWindowButton : public TextWindowButton,
                                   public boost::signals::trackable
{
public:
  typedef boost::function<void(void)> CallbackFunction;

public:
  ActivationTextWindowButton(bool use, GameexeInterpretObject location_box,
                             CallbackFunction start,
                             CallbackFunction end);
  ~ActivationTextWindowButton();

  virtual void buttonReleased();

  void setEnabled(bool on);
  void setActivated(bool on);

private:
  void setState();

  CallbackFunction on_start_;
  CallbackFunction on_end_;
  bool on_;
  bool enabled_;
};

// -----------------------------------------------------------------------

class RepeatActionWhileHoldingWindowButton : public TextWindowButton
{
public:
  typedef boost::function<void(void)> CallbackFunction;

public:
  RepeatActionWhileHoldingWindowButton(
    bool use, GameexeInterpretObject location_box,
    RLMachine& machine,
    CallbackFunction callback,
    unsigned int time_between_invocations);
  ~RepeatActionWhileHoldingWindowButton();

  virtual void buttonPressed();
  virtual void execute();
  virtual void buttonReleased();

private:
  RLMachine& machine_;
  CallbackFunction callback_;
  bool held_down_;
  unsigned int last_invocation_;
  unsigned int time_between_invocations_;
};

// -----------------------------------------------------------------------

class ExbtnWindowButton : public TextWindowButton
{
public:
  ExbtnWindowButton(RLMachine& machine,
                    bool use, GameexeInterpretObject location_box,
                    GameexeInterpretObject to_call);
  ~ExbtnWindowButton();

  virtual void buttonReleased();

private:
  RLMachine& machine_;

  int scenario_;
  int entrypoint_;
};

#endif
