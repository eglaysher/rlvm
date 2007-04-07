// This file is part of RLVM, a RealLive virtual machine clone.
//
// -----------------------------------------------------------------------
//
// Copyright (C) 2006 El Riot
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

/**
 * @file   Module_Msg.cpp
 * @author Elliot Glaysher
 * @date   Sun Oct  1 22:18:39 2006
 * 
 * @brief  Implements many textout related operations.
 */

#include "Modules/Module_Msg.hpp"
#include "MachineBase/RLOperation.hpp"
#include "MachineBase/RLMachine.hpp"
#include "MachineBase/RLModule.hpp"
//#include "GeneralOperations.hpp"

#include "Systems/Base/System.hpp"
#include "Systems/Base/EventSystem.hpp"
#include "Systems/Base/GraphicsSystem.hpp"
#include "Systems/Base/TextSystem.hpp"
#include "Systems/Base/TextPage.hpp"
#include "Systems/Base/TextWindow.hpp"

#include <iostream>

using namespace std;

/**
 * @defgroup ModuleMessage The Message and Textout module (mod<0:3>).
 * @ingroup ModulesOpcodes
 *
 * @{
 */

// -----------------------------------------------------------------------
// Longop_pause
// -----------------------------------------------------------------------

Longop_pause::Longop_pause(RLMachine& machine)
  : NiceLongOperation(machine), m_isDone(false)
{
  machine.system().text().setInPauseState(true);
  machine.system().graphics().markScreenAsDirty();
  machine.system().event().addEventHandler(this);
}

// -----------------------------------------------------------------------

Longop_pause::~Longop_pause()
{
  machine().system().event().removeEventHandler(this);
  machine().system().text().setInPauseState(false);
}

// -------------------------------------------- [ EventHandler interface ]
void Longop_pause::mouseButtonStateChanged(MouseButton mouseButton, 
                                           bool pressed)
{
  EventSystem& es = machine().system().event();
  TextSystem& text = machine().system().text();

  if(pressed)
  {
    switch(mouseButton)
    {
    case MOUSE_LEFT:
      m_isDone = true;
      break;
    case MOUSE_WHEELUP:
      text.backPage(machine());
      break;
    case MOUSE_WHEELDOWN:
      if(text.isReadingBacklog())
        text.forwardPage(machine());
      break;
    }
  }
}

// -----------------------------------------------------------------------

void Longop_pause::keyStateChanged(KeyCode keyCode, bool pressed)
{
  EventSystem& es = machine().system().event();
  TextSystem& text = machine().system().text();

  if(pressed)
  {
    if(keyCode == RLKEY_RCTRL || keyCode == RLKEY_LCTRL)
      m_isDone = true;
    else if(keyCode == RLKEY_UP)
      text.backPage(machine());
    else if(keyCode == RLKEY_DOWN)
    {
      if(text.isReadingBacklog())
        text.forwardPage(machine());
    }
    else if(keyCode == RLKEY_RETURN)
      m_isDone = true;
  }
}

// -----------------------------------------------------------------------

bool Longop_pause::operator()(RLMachine& machine)
{
  if(m_isDone)
  {
    TextSystem& text = machine.system().text();
    TextPage& page = text.currentPage(machine);
    int windowNum = page.currentWindowNum();
    TextWindow& textWindow = text.textWindow(machine, windowNum);

    if(textWindow.actionOnPause())
    {
      machine.system().text().currentPage(machine).hardBrake();    
    }
    else
    {
      machine.system().text().newPage(machine);
    }
  }

  return m_isDone;
}

// -----------------------------------------------------------------------
// -----------------------------------------------------------------------

struct Msg_par : public RLOp_Void_Void {
  void operator()(RLMachine& machine) {
    TextPage& page = machine.system().text().currentPage(machine);
    page.resetIndentation();
    page.hardBrake();
  }
};

// -----------------------------------------------------------------------

/** 
 * Implements op<0:Msg:17, 0>, fun pause().
 * 
 * @todo This still isn't a real implementation of pause(), needs to
 * handle the window events when we have text windows.
 * @bug Does this work with ctrl()?
 */
struct Msg_pause : public RLOp_Void_Void {
  void operator()(RLMachine& machine) {
    machine.pushLongOperation(new Longop_pause(machine));
  }
};

// -----------------------------------------------------------------------

struct Msg_TextWindow : public RLOp_Void_1< DefaultIntValue_T< 0 > >
{
  void operator()(RLMachine& machine, int window)
  {
    machine.system().text().setDefaultWindow(window);
    machine.system().text().currentPage(machine).setWindow(window);
  }
};

// -----------------------------------------------------------------------

struct Msg_FontColour : public RLOp_Void_2< DefaultIntValue_T< 0 >,
                                            DefaultIntValue_T< 0 > >
{
  void operator()(RLMachine& machine, int textColorNum, int shadowColorNum)
  {
    machine.system().text().currentPage(machine).fontColour(textColorNum);
  }
};

// -----------------------------------------------------------------------

struct Msg_msgHide : public RLOp_Void_1< DefaultIntValue_T< 0 > >
{
  void operator()(RLMachine& machine, int unknown)
  {
//    machine.system().text().
  }
};

// -----------------------------------------------------------------------

struct Msg_br : public RLOp_Void_Void {
  void operator()(RLMachine& machine) {
    machine.system().text().currentPage(machine).hardBrake();    
  }
};

// -----------------------------------------------------------------------

MsgModule::MsgModule()
  : RLModule("Msg", 0, 003)
{
  addOpcode(3, 0, new Msg_par);
//  addOpcode(15, 0, /* spause3 */ );
  addOpcode(17, 0, new Msg_pause);

  addOpcode(102, 0, new Msg_TextWindow);
  addOpcode(102, 1, new Msg_TextWindow);

  addOpcode(105, 0, new Msg_FontColour);
  addOpcode(105, 1, new Msg_FontColour);
  addOpcode(105, 2, new Msg_FontColour);

  addOpcode(151, 0, new Msg_msgHide);

  addOpcode(201, 0, new Msg_br);

//  addOpcode(100, 0, );

//   addOpcode(101, 0, new Op_SetToIncoming(textSystem.fontSizeInPixels(),
//                                          textSystem));            
//   addOpcode(101, 1, new Op_ReturnValue(textSystem.fontSizeInPixels()));
//  addOpcode(102 ...)
//  addOpcode(103, 0, new Op_SetToIntConstant(textSystem, 
//                                            &TextSystem::setFastTextMode, 1));
//  addOpcode(104, 0, new Op_SetToIntConstant(textSystem,
//                                            &TextSystem::setFastTextMode, 0));

//   addOpcode(104, 0, new Op_SetToFalse(text.fastTextMode(),
//                                       textSystem));
//  addOpcode(105, 0
//  addOpcode(106
//  addOpcode
}

// @}
