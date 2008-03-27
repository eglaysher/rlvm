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
  : m_state(BUTTONSTATE_BUTTON_NOT_USED)  
{
}

// -----------------------------------------------------------------------

TextWindowButton::TextWindowButton(bool useThisButton,
                                   GameexeInterpretObject locationBox)
  : m_state(BUTTONSTATE_BUTTON_NOT_USED)
{
  if(useThisButton && locationBox.exists())
  {
    std::vector<int> z = locationBox;
    m_location = locationBox;
    m_state = BUTTONSTATE_NORMAL;
  }
}

// -----------------------------------------------------------------------

TextWindowButton::~TextWindowButton()
{
}

// -----------------------------------------------------------------------

int TextWindowButton::xLocation(TextWindow& window)
{
  int type = m_location.at(0);
  switch(type)
  {
  case 0:
    return window.boxX1() + m_location.at(1);
  default:
    throw SystemError("Unsupported coordinate system"); 
  }
}

// -----------------------------------------------------------------------

int TextWindowButton::yLocation(TextWindow& window)
{
  int type = m_location.at(0);
  switch(type)
  {
  case 0:
    return window.boxY1() + m_location.at(2);
  default:
    throw SystemError("Unsupported coordinate system"); 
  }
}

// -----------------------------------------------------------------------

bool TextWindowButton::isValid() const
{
  return m_state != BUTTONSTATE_BUTTON_NOT_USED && m_location.size() == 5 &&
    !(m_location[0] == 0 && m_location[1] == 0 && m_location[2] == 0 &&
      m_location[3] == 0 && m_location[4] == 0);
}

// -----------------------------------------------------------------------

void TextWindowButton::setMousePosition(
  RLMachine& machine, TextWindow& window, int x, int y)
{
  if(isValid())
  {
    int origState = m_state;
    
    int x1 = xLocation(window);
    int x2 = x1 + m_location.at(3);
    int y1 = yLocation(window);
    int y2 = y1 + m_location.at(4);

    bool inBox = x >= x1 && x < x2 && y >= y1 && y < y2;

    if(inBox && m_state == BUTTONSTATE_NORMAL)
      m_state = BUTTONSTATE_HIGHLIGHTED;
    else if(!inBox && m_state == BUTTONSTATE_HIGHLIGHTED)
        m_state = BUTTONSTATE_NORMAL;
    else if(!inBox && m_state == BUTTONSTATE_PRESSED)
      m_state = BUTTONSTATE_NORMAL;

    if(origState != m_state)
      machine.system().graphics().markScreenAsDirty(GUT_TEXTSYS);
  }
}

// -----------------------------------------------------------------------

bool TextWindowButton::handleMouseClick(
  RLMachine& machine, TextWindow& window, int x, int y, bool pressed)
{
  if(isValid())
  {
    int x1 = xLocation(window);
    int x2 = x1 + m_location.at(3);
    int y1 = yLocation(window);
    int y2 = y1 + m_location.at(4);

    bool inBox = x >= x1 && x < x2 && y >= y1 && y < y2;

    if(inBox)
    {
      // Perform any activation 
      if(pressed)
      {
        m_state = BUTTONSTATE_PRESSED;
        buttonPressed();
      }
      else
      {
        m_state = BUTTONSTATE_HIGHLIGHTED;
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
    Surface::GrpRect rect = buttons->getPattern(basePattern + m_state);
    if(!(rect.x1 == 0 && rect.y1 == 0 && rect.x2 == 0 && rect.y2 == 0))
    {
      int destX = xLocation(window);
      int destY = yLocation(window);
      int width = rect.x2 - rect.x1;
      int height = rect.y2 - rect.y1;

      buttons->renderToScreen(
        rect.x1, rect.y1, rect.x2, rect.y2,
        destX, destY, destX + width, destY + height,
        255);
    }
  }
}


// -----------------------------------------------------------------------
// ActionTextWindowButton
// -----------------------------------------------------------------------

ActionTextWindowButton::ActionTextWindowButton(
  bool use, GameexeInterpretObject locationBox,
  CallbackFunction action)
  : TextWindowButton(use, locationBox), m_action(action)
{
}

// -----------------------------------------------------------------------

ActionTextWindowButton::~ActionTextWindowButton()
{
}

// -----------------------------------------------------------------------

void ActionTextWindowButton::buttonReleased()
{
  m_action();
}

// -----------------------------------------------------------------------
// ActivationTextWindowButton
// -----------------------------------------------------------------------

ActivationTextWindowButton::ActivationTextWindowButton(
  bool use, GameexeInterpretObject locationBox,
  CallbackFunction start,
  CallbackFunction end)
  : TextWindowButton(use, locationBox), m_onStart(start), m_onEnd(end),
    m_on(false)
{
}

// -----------------------------------------------------------------------

ActivationTextWindowButton::~ActivationTextWindowButton()
{
}

// -----------------------------------------------------------------------

void ActivationTextWindowButton::buttonReleased()
{
  if(m_on)
  {
    m_onEnd();
    m_on = false;
    m_state = BUTTONSTATE_NORMAL;
  }
  else
  {
    m_onStart();
    m_on = true;
    m_state = BUTTONSTATE_ACTIVATED;
  }
}

// -----------------------------------------------------------------------
// RepeatActionWhileHoldingWindowButton
// -----------------------------------------------------------------------

RepeatActionWhileHoldingWindowButton::RepeatActionWhileHoldingWindowButton(
  bool use, GameexeInterpretObject locationBox, RLMachine& machine,
  CallbackFunction callback, unsigned int timeBetweenInvocations)
  : TextWindowButton(use, locationBox), m_machine(machine),
    m_callback(callback), m_heldDown(false),
    m_timeBetweenInvocations(timeBetweenInvocations)
{
}

// -----------------------------------------------------------------------

RepeatActionWhileHoldingWindowButton::~RepeatActionWhileHoldingWindowButton()
{
}

// -----------------------------------------------------------------------

void RepeatActionWhileHoldingWindowButton::buttonPressed()
{
  m_heldDown = true;

  m_callback();
  m_lastInvocation = m_machine.system().event().getTicks();
}

// -----------------------------------------------------------------------

void RepeatActionWhileHoldingWindowButton::execute()
{
  if(m_heldDown)
  {
    unsigned int curTime = m_machine.system().event().getTicks();

    if(m_lastInvocation + m_timeBetweenInvocations > curTime)
    {
      m_callback();
      m_lastInvocation = curTime;
    }
  }
}

// -----------------------------------------------------------------------

void RepeatActionWhileHoldingWindowButton::buttonReleased()
{
  m_heldDown = false;
}

// -----------------------------------------------------------------------
// ExbtnWindowButton
// -----------------------------------------------------------------------

ExbtnWindowButton::ExbtnWindowButton(
  RLMachine& machine,
  bool use, GameexeInterpretObject locationBox,
  GameexeInterpretObject toCall)
  : TextWindowButton(use, locationBox), m_machine(machine),
    m_scenario(0), m_entrypoint(0)
{
  if(locationBox.exists() && toCall.exists())
  {
    vector<int> farcall = toCall;
    m_scenario = farcall.at(0);
    m_entrypoint = farcall.at(1);
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
  m_machine.system().text().setSystemVisible(false);

  /// Push a LongOperation onto the stack which will restore
  /// visibility when we return from this Exbtn call
  m_machine.pushLongOperation(new RestoreTextSystemVisibility);

  m_machine.farcall(m_scenario, m_entrypoint);
}
