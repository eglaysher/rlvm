// This file is part of RLVM, a RealLive virtual machine clone.
//
// -----------------------------------------------------------------------
//
// Copyright (C) 2007 El Riot
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

PauseLongOperation::PauseLongOperation(RLMachine& machine)
  : NiceLongOperation(machine), m_isDone(false)
{
  TextSystem& text = machine.system().text();
  EventSystem& event = machine.system().event();

  // Initialize Auto Mode (in case it's activated, or in case it gets
  // activated)
  int numChars =
    text.currentPage(machine).numberOfCharsOnPage();
  m_automodeTime = text.getAutoTime(numChars);
  m_startTime = event.getTicks();

  machine.system().graphics().markScreenAsDirty();

  // We undo this in the destructor
  text.setInPauseState(true);
  event.addEventHandler(this);
}

// -----------------------------------------------------------------------

PauseLongOperation::~PauseLongOperation()
{
  machine().system().event().removeEventHandler(this);
  machine().system().text().setInPauseState(false);
}

// -----------------------------------------------------------------------

/** 
 * @todo This is fairly buggy right now. I wanted to support the
 *       CLANNAD menu, but there are three problems. 1) I don't handle
 *       everything necessary to display the menu properly. 2) The
 *       code in CLANNAD's SEEN.TXT doesn't return properly. 3) Even
 *       if it did, THIS C++ code doesn't return properly.
 *  
 *       To really fix that final one, I'm going to have to make
 *       Longoperations and Reallive stack frames use the same call
 *       stack instead of the hacktastic separate stacks I'm doing
 *       now.
 */
void PauseLongOperation::handleSyscomCall()
{
  Gameexe& gexe = machine().system().gameexe();

  if(gexe("CANCELCALL_MOD") == 1)
  {
    vector<int> cancelcall = gexe("CANCELCALL");
    machine().farcall(cancelcall.at(0), cancelcall.at(1));
    m_isDone = true;
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
  machine().system().text().setMousePosition(machine(), x, y);
}

// -----------------------------------------------------------------------

void PauseLongOperation::mouseButtonStateChanged(MouseButton mouseButton, 
                                           bool pressed)
{
  EventSystem& es = machine().system().event();
  TextSystem& text = machine().system().text();

  switch(mouseButton)
  {
  case MOUSE_LEFT:
  {
    int x, y;
    es.getCursorPos(x, y);
    if(!machine().system().text().handleMouseClick(machine(), x, y, pressed))
    {
      if(pressed)
        m_isDone = true;
    }
    break;
  }
  case MOUSE_RIGHT:
    if(pressed)
      handleSyscomCall();
    break;
  case MOUSE_WHEELUP:
    if(pressed)
      text.backPage(machine());
    break;
  case MOUSE_WHEELDOWN:
    if(pressed)
      text.forwardPage(machine());
    break;
  }
}

// -----------------------------------------------------------------------

void PauseLongOperation::keyStateChanged(KeyCode keyCode, bool pressed)
{
  EventSystem& es = machine().system().event();
  TextSystem& text = machine().system().text();

  if(pressed)
  {
    bool ctrlKeySkips = text.ctrlKeySkip();

    if(ctrlKeySkips && 
       (keyCode == RLKEY_RCTRL || keyCode == RLKEY_LCTRL))
    {
      m_isDone = true;
    }
    else if(keyCode == RLKEY_UP)
      text.backPage(machine());
    else if(keyCode == RLKEY_DOWN)
      text.forwardPage(machine());
    else if(keyCode == RLKEY_RETURN)
      m_isDone = true;
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
  machine.system().text().newPage(machine);
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

