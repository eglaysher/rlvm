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

#include "Precompiled.hpp"

// -----------------------------------------------------------------------

#include "Systems/Base/TextWindowButton.hpp"
#include "MachineBase/RLMachine.hpp"
#include "MachineBase/LongOperation.hpp"
#include "Systems/Base/Surface.hpp"
#include "Systems/Base/System.hpp"
#include "Systems/Base/SystemError.hpp"
#include "Systems/Base/EventSystem.hpp"
#include "Systems/Base/GraphicsSystem.hpp"
#include "Systems/Base/TextSystem.hpp"
#include "Systems/Base/TextWindow.hpp"
#include "Systems/Base/Rect.hpp"

#include <stdexcept>
#include <iostream>

using namespace std;

/**
 * Describes the state of a Waku button
 */
enum ButtonState
{
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

TextWindowButton::TextWindowButton()
  : state_(BUTTONSTATE_BUTTON_NOT_USED)
{
}

// -----------------------------------------------------------------------

TextWindowButton::TextWindowButton(bool useThisButton,
                                   GameexeInterpretObject locationBox)
  : state_(BUTTONSTATE_BUTTON_NOT_USED)
{
  if(useThisButton && locationBox.exists())
  {
    std::vector<int> z = locationBox;
    location_ = locationBox;
    state_ = BUTTONSTATE_NORMAL;
  }
}

// -----------------------------------------------------------------------

TextWindowButton::~TextWindowButton()
{
}

// -----------------------------------------------------------------------

Rect TextWindowButton::location(TextWindow& window)
{
  int type = location_.at(0);
  switch(type)
  {
  case 0:
    return Rect(Point(window.boxX1() + location_.at(1),
                      window.boxY1() + location_.at(2)),
                Size(location_.at(3), location_.at(4)));
  default:
    throw SystemError("Unsupported coordinate system");
  }
}

// -----------------------------------------------------------------------

bool TextWindowButton::isValid() const
{
  return state_ != BUTTONSTATE_BUTTON_NOT_USED && location_.size() == 5 &&
    !(location_[0] == 0 && location_[1] == 0 && location_[2] == 0 &&
      location_[3] == 0 && location_[4] == 0);
}

// -----------------------------------------------------------------------

void TextWindowButton::setMousePosition(
  RLMachine& machine, TextWindow& window, const Point& pos)
{
  if(isValid())
  {
    int origState = state_;
    bool inBox = location(window).contains(pos);

    if(inBox && state_ == BUTTONSTATE_NORMAL)
      state_ = BUTTONSTATE_HIGHLIGHTED;
    else if(!inBox && state_ == BUTTONSTATE_HIGHLIGHTED)
      state_ = BUTTONSTATE_NORMAL;
    else if(!inBox && state_ == BUTTONSTATE_PRESSED)
      state_ = BUTTONSTATE_NORMAL;

    if(origState != state_)
      machine.system().graphics().markScreenAsDirty(GUT_TEXTSYS);
  }
}

// -----------------------------------------------------------------------

bool TextWindowButton::handleMouseClick(
  RLMachine& machine, TextWindow& window, const Point& pos, bool pressed)
{
  if(isValid())
  {
    bool inBox = location(window).contains(pos);

    if(inBox)
    {
      // Perform any activation
      if(pressed)
      {
        state_ = BUTTONSTATE_PRESSED;
        buttonPressed();
      }
      else
      {
        state_ = BUTTONSTATE_HIGHLIGHTED;
        buttonReleased();
      }

      machine.system().graphics().markScreenAsDirty(GUT_TEXTSYS);

      return true;
    }
  }

  return false;
}

// -----------------------------------------------------------------------

void TextWindowButton::render(RLMachine& machine,
                              TextWindow& window,
                              const boost::shared_ptr<Surface>& buttons,
                              int basePattern)
{
  if(isValid())
  {
    Surface::GrpRect rect = buttons->getPattern(basePattern + state_);
    if(!(rect.rect.isEmpty()))
    {
      Rect dest = Rect(location(window).origin(), rect.rect.size());
      buttons->renderToScreen(rect.rect, dest, 255);
    }
  }
}


// -----------------------------------------------------------------------
// ActionTextWindowButton
// -----------------------------------------------------------------------

ActionTextWindowButton::ActionTextWindowButton(
  bool use, GameexeInterpretObject locationBox,
  CallbackFunction action)
  : TextWindowButton(use, locationBox), action_(action)
{
}

// -----------------------------------------------------------------------

ActionTextWindowButton::~ActionTextWindowButton()
{
}

// -----------------------------------------------------------------------

void ActionTextWindowButton::buttonReleased()
{
  action_();
}

// -----------------------------------------------------------------------
// ActivationTextWindowButton
// -----------------------------------------------------------------------

ActivationTextWindowButton::ActivationTextWindowButton(
  bool use, GameexeInterpretObject locationBox,
  CallbackFunction start,
  CallbackFunction end)
  : TextWindowButton(use, locationBox), on_start_(start), on_end_(end),
    on_(false)
{
}

// -----------------------------------------------------------------------

ActivationTextWindowButton::~ActivationTextWindowButton()
{
}

// -----------------------------------------------------------------------

void ActivationTextWindowButton::buttonReleased()
{
  if(on_)
  {
    on_end_();
    on_ = false;
    state_ = BUTTONSTATE_NORMAL;
  }
  else
  {
    on_start_();
    on_ = true;
    state_ = BUTTONSTATE_ACTIVATED;
  }
}

// -----------------------------------------------------------------------
// RepeatActionWhileHoldingWindowButton
// -----------------------------------------------------------------------

RepeatActionWhileHoldingWindowButton::RepeatActionWhileHoldingWindowButton(
  bool use, GameexeInterpretObject locationBox, RLMachine& machine,
  CallbackFunction callback, unsigned int timeBetweenInvocations)
  : TextWindowButton(use, locationBox), machine_(machine),
    callback_(callback), held_down_(false),
    time_between_invocations_(timeBetweenInvocations)
{
}

// -----------------------------------------------------------------------

RepeatActionWhileHoldingWindowButton::~RepeatActionWhileHoldingWindowButton()
{
}

// -----------------------------------------------------------------------

void RepeatActionWhileHoldingWindowButton::buttonPressed()
{
  held_down_ = true;

  callback_();
  last_invocation_ = machine_.system().event().getTicks();
}

// -----------------------------------------------------------------------

void RepeatActionWhileHoldingWindowButton::execute()
{
  if(held_down_)
  {
    unsigned int curTime = machine_.system().event().getTicks();

    if(last_invocation_ + time_between_invocations_ > curTime)
    {
      callback_();
      last_invocation_ = curTime;
    }
  }
}

// -----------------------------------------------------------------------

void RepeatActionWhileHoldingWindowButton::buttonReleased()
{
  held_down_ = false;
}

// -----------------------------------------------------------------------
// ExbtnWindowButton
// -----------------------------------------------------------------------

ExbtnWindowButton::ExbtnWindowButton(
  RLMachine& machine,
  bool use, GameexeInterpretObject locationBox,
  GameexeInterpretObject toCall)
  : TextWindowButton(use, locationBox), machine_(machine),
    scenario_(0), entrypoint_(0)
{
  if(locationBox.exists() && toCall.exists())
  {
    vector<int> farcall = toCall;
    scenario_ = farcall.at(0);
    entrypoint_ = farcall.at(1);
  }
}

// -----------------------------------------------------------------------

ExbtnWindowButton::~ExbtnWindowButton()
{
}

// -----------------------------------------------------------------------

struct RestoreTextSystemVisibility : public LongOperation
{
  bool operator()(RLMachine& machine)
  {
    machine.system().text().setSystemVisible(true);
    return true;
  }
};

// -----------------------------------------------------------------------

void ExbtnWindowButton::buttonReleased()
{
  /// Hide all text boxes when entering an Exbtn
  machine_.system().text().setSystemVisible(false);

  /// Push a LongOperation onto the stack which will restore
  /// visibility when we return from this Exbtn call
  machine_.pushLongOperation(new RestoreTextSystemVisibility);

  machine_.farcall(scenario_, entrypoint_);
}
