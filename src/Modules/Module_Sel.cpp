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
  : EventHandler(machine), machine_(machine),
    textWindow(machine.system().text().currentWindow(machine)),
    return_value_(-1)
{
  machine.system().text().setInSelectionMode(true);
  textWindow.setVisible(true);
  textWindow.startSelectionMode();
  textWindow.setSelectionCallback(
    bind(&Sel_LongOperation::selected, this, _1));

  const vector<SelectElement::Param>& params = commandElement.getRawParams();
  for (unsigned int i = 0; i < params.size(); ++i) {
    std::string utf8str = cp932toUTF8(params[i].text,
                                      machine.getTextEncoding());
    textWindow.addSelectionItem(utf8str);
  }

  machine.system().graphics().markScreenAsDirty(GUT_TEXTSYS);
}

// -----------------------------------------------------------------------

Sel_LongOperation::~Sel_LongOperation()
{
  textWindow.endSelectionMode();
  machine_.system().text().setInSelectionMode(false);
}

// -----------------------------------------------------------------------

void Sel_LongOperation::selected(int num)
{
  return_value_ = num;
}

// -----------------------------------------------------------------------

bool Sel_LongOperation::operator()(RLMachine& machine)
{
  if(return_value_ != -1)
  {
    machine.setStoreRegister(return_value_);
    return true;
  }
  else
    return false;
}

// -------------------------------------------- [ EventHandler interface ]
void Sel_LongOperation::mouseMotion(const Point& pos)
{
  // Tell the text system about the move
  machine_.system().text().setMousePosition(machine_, pos);
}

// -----------------------------------------------------------------------

bool Sel_LongOperation::mouseButtonStateChanged(MouseButton mouseButton,
                                                bool pressed)
{
  EventSystem& es = machine_.system().event();

  switch(mouseButton)
  {
  case MOUSE_LEFT:
  {
    Point pos = es.getCursorPos();
    machine_.system().text().handleMouseClick(machine_, pos, pressed);
    return true;
    break;
  }
  case MOUSE_RIGHT:
  {
    if (pressed) {
      machine_.system().showSyscomMenu(machine_);
      return true;
    }
    break;
  }
  default:
    break;
  }

  return false;
}

// -----------------------------------------------------------------------

struct Sel_select : public RLOp_SpecialCase
{
  void operator()(RLMachine& machine, const CommandElement& ce)
  {
    if(machine.shouldSetSelcomSavepoint())
      machine.markSavepoint();

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
