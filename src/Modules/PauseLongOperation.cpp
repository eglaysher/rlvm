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

#include "Precompiled.hpp"

// -----------------------------------------------------------------------

#include "Modules/PauseLongOperation.hpp"
#include "MachineBase/RLMachine.hpp"
#include "libReallive/gameexe.h"

#include "Systems/Base/System.hpp"
#include "Systems/Base/EventSystem.hpp"
#include "Systems/Base/GraphicsSystem.hpp"
#include "Systems/Base/TextSystem.hpp"
#include "Systems/Base/TextPage.hpp"
#include "Systems/Base/TextWindow.hpp"

#include <vector>

using namespace std;

// -----------------------------------------------------------------------
// PauseLongOperation
// -----------------------------------------------------------------------

PauseLongOperation::PauseLongOperation(RLMachine& imachine)
  : EventHandler(imachine), machine(imachine), m_isDone(false)
{
  TextSystem& text = machine.system().text();
  EventSystem& event = machine.system().event();

  // Initialize Auto Mode (in case it's activated, or in case it gets
  // activated)
  int numChars =
    text.currentPage(machine).numberOfCharsOnPage();
  m_automodeTime = text.getAutoTime(numChars);
  m_startTime = event.getTicks();

  machine.system().graphics().markScreenForRefresh();

  // We undo this in the destructor
  text.setInPauseState(true);
}

// -----------------------------------------------------------------------

PauseLongOperation::~PauseLongOperation()
{
  machine.system().text().setInPauseState(false);
}

// -----------------------------------------------------------------------

/** 
 * @todo This code has some issues; we don't handle non custom SYSCOM
 *       calls, we don't disable parts of rendering (such as the key
 *       cursor). Still, for the most pat this works.
 */
void PauseLongOperation::handleSyscomCall()
{
  Gameexe& gexe = machine.system().gameexe();

  if(gexe("CANCELCALL_MOD") == 1)
  {
    vector<int> cancelcall = gexe("CANCELCALL");
    machine.farcall(cancelcall.at(0), cancelcall.at(1));
  }
  else
  {
    cerr << "(We don't deal with non-custom SYSCOM calls yet.)" << endl;
  }
}

// -------------------------------------------- [ EventHandler interface ]
void PauseLongOperation::mouseMotion(int x, int y)
{
  // Tell the text system about the move
  machine.system().text().setMousePosition(machine, x, y);
}

// -----------------------------------------------------------------------

void PauseLongOperation::mouseButtonStateChanged(MouseButton mouseButton, 
                                                 bool pressed)
{
  GraphicsSystem& graphics = machine.system().graphics();
  EventSystem& es = machine.system().event();

  TextSystem& text = machine.system().text();

  switch(mouseButton)
  {
  case MOUSE_LEFT:
  {
    int x, y;
    es.getCursorPos(x, y);
    // Only unhide the interface on release of the left mouse button
    if(graphics.interfaceHidden())
    {
      if(!pressed)
        graphics.toggleInterfaceHidden();
    }
    else if(!machine.system().text().handleMouseClick(machine, x, y, pressed))
    {
      if(pressed)
      {
        if(text.isReadingBacklog())
        {
          // Move back to the main page.
          text.stopReadingBacklog();
        }
        else
        {
          m_isDone = true;
        }
      }
    }
    break;
  }
  case MOUSE_RIGHT:
    if(pressed)
      handleSyscomCall();
    break;
  case MOUSE_WHEELUP:
    if(pressed)
      text.backPage(machine);
    break;
  case MOUSE_WHEELDOWN:
    if(pressed)
      text.forwardPage(machine);
    break;
  default:
    break;
  }
}

// -----------------------------------------------------------------------

void PauseLongOperation::keyStateChanged(KeyCode keyCode, bool pressed)
{
  GraphicsSystem& graphics = machine.system().graphics();
  TextSystem& text = machine.system().text();
  
  if(pressed)
  {
    if(graphics.interfaceHidden())
    {
      graphics.toggleInterfaceHidden();
    }
    else
    {
      bool ctrlKeySkips = text.ctrlKeySkip();

      if(ctrlKeySkips && 
         (keyCode == RLKEY_RCTRL || keyCode == RLKEY_LCTRL))
      {
        m_isDone = true;
      }
      else if(keyCode == RLKEY_SPACE)
        graphics.toggleInterfaceHidden();
      else if(keyCode == RLKEY_UP)
        text.backPage(machine);
      else if(keyCode == RLKEY_DOWN)
        text.forwardPage(machine);
      else if(keyCode == RLKEY_RETURN)
        m_isDone = true;
    }
  }
}

// -----------------------------------------------------------------------

bool PauseLongOperation::operator()(RLMachine& machine)
{
  // Check to see if we're done because of the auto mode timer
  if(machine.system().text().autoMode())
  {
    unsigned int curTime = machine.system().event().getTicks();
    if(m_startTime + m_automodeTime < curTime)
      m_isDone = true;
  }

  return m_isDone;
}

// -----------------------------------------------------------------------

void PauseLongOperation::looseFocus()
{
  machine.system().event().removeEventHandler(this);
}

// -----------------------------------------------------------------------

void PauseLongOperation::gainFocus()
{
  machine.system().event().addEventHandler(this);
}

// -----------------------------------------------------------------------
// NewPageAfterLongop
// -----------------------------------------------------------------------
NewPageAfterLongop::NewPageAfterLongop(LongOperation* inOp)
  : PerformAfterLongOperationDecorator(inOp)
{}

// -----------------------------------------------------------------------

NewPageAfterLongop::~NewPageAfterLongop()
{}

// -----------------------------------------------------------------------

void NewPageAfterLongop::performAfterLongOperation(RLMachine& machine)
{
  TextSystem& text = machine.system().text();
  text.snapshot(machine);
  text.currentWindow(machine).clearWin();
  text.newPageOnWindow(machine, text.activeWindow());
}

// -----------------------------------------------------------------------
// HardBrakeAfterLongop
// -----------------------------------------------------------------------
HardBrakeAfterLongop::HardBrakeAfterLongop(LongOperation* inOp)
  : PerformAfterLongOperationDecorator(inOp)
{}

// -----------------------------------------------------------------------

HardBrakeAfterLongop::~HardBrakeAfterLongop()
{}

// -----------------------------------------------------------------------

void HardBrakeAfterLongop::performAfterLongOperation(RLMachine& machine)
{
  machine.system().text().currentPage(machine).hardBrake();
}

