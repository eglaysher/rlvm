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

#include "Modules/Module_Sel.hpp"
#include "Modules/cp932toUnicode.hpp"

#include "MachineBase/RLMachine.hpp"
#include "MachineBase/RLOperation.hpp"

#include "Systems/Base/System.hpp"
#include "Systems/Base/GraphicsSystem.hpp"
#include "Systems/Base/TextSystem.hpp"
#include "Systems/Base/TextWindow.hpp"
#include "Systems/Base/EventSystem.hpp"

#include "libReallive/bytecode.h"

#include <vector>
#include <iostream>

#include <boost/bind.hpp>

using boost::bind;
using std::cin;
using std::cerr;
using std::endl;
using std::vector;
using libReallive::SelectElement;
using libReallive::CommandElement;

// -----------------------------------------------------------------------

Sel_LongOperation::Sel_LongOperation(
  RLMachine& machine,
  const libReallive::SelectElement& commandElement)
  : m_machine(machine), EventHandler(machine),
    textWindow(machine.system().text().currentWindow(machine)),
    m_returnValue(-1)
{
  textWindow.startSelectionMode();
  textWindow.setSelectionCallback(
    bind(&Sel_LongOperation::selected, this, _1));

  cerr << "---------" << endl;
  const vector<SelectElement::Param>& params = commandElement.getRawParams();
  for(int i = 0; i < params.size(); ++i)
  {
    std::string utf8str = cp932toUTF8(params[i].text, 
                                      machine.getTextEncoding());
    cerr << i << ": " << utf8str << endl;
    textWindow.addSelectionItem(machine, utf8str);
  }
  cerr << "---------" << endl;
  machine.system().graphics().markScreenForRefresh();
}

// -----------------------------------------------------------------------

Sel_LongOperation::~Sel_LongOperation()
{
  textWindow.endSelectionMode();
}

// -----------------------------------------------------------------------

void Sel_LongOperation::selected(int num)
{
  m_returnValue = num;
}

// -----------------------------------------------------------------------

bool Sel_LongOperation::operator()(RLMachine& machine) 
{
  if(m_returnValue != -1)
  {
    machine.setStoreRegister(m_returnValue);
    return true;
  }
  else 
    return false;
}

// -------------------------------------------- [ EventHandler interface ]
void Sel_LongOperation::mouseMotion(int x, int y)
{
  // Tell the text system about the move
  m_machine.system().text().setMousePosition(m_machine, x, y);
}

// -----------------------------------------------------------------------

void Sel_LongOperation::mouseButtonStateChanged(MouseButton mouseButton, 
                                                bool pressed)
{
  EventSystem& es = m_machine.system().event();
  TextSystem& text = m_machine.system().text();

  switch(mouseButton)
  {
  case MOUSE_LEFT:
  {
    int x, y;
    es.getCursorPos(x, y);
    m_machine.system().text().handleMouseClick(m_machine, x, y, pressed);
    break;
  case MOUSE_RIGHT:
    cerr << "Doesn't handle syscom!" << endl;
    break;
  }
  }
}
 
// -----------------------------------------------------------------------

struct Sel_select : public RLOp_SpecialCase
{
  void operator()(RLMachine& machine, const CommandElement& ce)
  {
    const SelectElement& element = dynamic_cast<const SelectElement&>(ce);
    machine.pushLongOperation(new Sel_LongOperation(machine, element));
    machine.advanceInstructionPointer();
  }
};

// -----------------------------------------------------------------------

SelModule::SelModule()
  : RLModule("Sel", 0, 2)
{
  addOpcode(1, 0, new Sel_select);
}
