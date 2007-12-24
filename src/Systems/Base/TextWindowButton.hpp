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
// the Free Software Foundation; either version 2 of the License, or
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

#include "libReallive/gameexe.h"

class RLMachine;
class TextWindow;
class Surface;

class TextWindowButton : public boost::noncopyable
{
protected:
  std::vector<int> m_location;
  int m_state;

public:
  TextWindowButton();
  TextWindowButton(bool use, GameexeInterpretObject locationBox);
  virtual ~TextWindowButton();

  /// Returns the absolute screen coordinate of this button.
  int xLocation(TextWindow& window);
  int yLocation(TextWindow& window);

  /// Checks to see if this is a valid, used button
  bool isValid() const;

  /// Track the mouse position to see if we need to alter our state
  void setMousePosition(RLMachine& machine, TextWindow& window, 
                        int x, int y);

  bool handleMouseClick(RLMachine& machine, TextWindow& window, 
                        int x, int y, bool pressed);

  // 
  void render(RLMachine& machine, TextWindow& window,
              const boost::shared_ptr<Surface>& buttons, 
              int basePattern);

  /// Called when the button is pressed
  virtual void buttonPressed() {}

  /// Called by other execute() calls while the System object has its
  /// turn to do any updating
  virtual void execute() {}

  /// Called when the button is released
  virtual void buttonReleased() {}
};

// -----------------------------------------------------------------------

class ActionTextWindowButton : public TextWindowButton
{
public:
  typedef boost::function<void(void)> CallbackFunction;

private:
  CallbackFunction m_action;

public:
  ActionTextWindowButton(bool use, GameexeInterpretObject locationBox,
                         CallbackFunction action);
  ~ActionTextWindowButton();

  virtual void buttonReleased();
};

// -----------------------------------------------------------------------

class ActivationTextWindowButton : public TextWindowButton
{
public:
  typedef boost::function<void(void)> CallbackFunction;

private:
  CallbackFunction m_onStart;
  CallbackFunction m_onEnd;
  bool m_on;

public:
  ActivationTextWindowButton(bool use, GameexeInterpretObject locationBox,
                             CallbackFunction start,
                             CallbackFunction end);
  ~ActivationTextWindowButton();

  virtual void buttonReleased();
};

// -----------------------------------------------------------------------

class RepeatActionWhileHoldingWindowButton : public TextWindowButton
{
public:
  typedef boost::function<void(void)> CallbackFunction;

private:
  RLMachine& m_machine;
  CallbackFunction m_callback;
  bool m_heldDown;
  unsigned int m_lastInvocation;
  unsigned int m_timeBetweenInvocations;

public:
  RepeatActionWhileHoldingWindowButton(
    bool use, GameexeInterpretObject locationBox,
    RLMachine& machine,
    CallbackFunction callback,
    unsigned int timeBetweenInvocations);
  ~RepeatActionWhileHoldingWindowButton();

  virtual void buttonPressed();
  virtual void execute();
  virtual void buttonReleased();
};

// -----------------------------------------------------------------------

class ExbtnWindowButton : public TextWindowButton
{
private:
  RLMachine& m_machine;

  int m_scenario;
  int m_entrypoint;

public:
  ExbtnWindowButton(RLMachine& machine, 
                    bool use, GameexeInterpretObject locationBox,
                    GameexeInterpretObject toCall);
  ~ExbtnWindowButton();

  virtual void buttonReleased();
};

#endif
