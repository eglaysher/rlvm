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

#include "MachineBase/RLMachine.hpp"
#include "MachineBase/RLOperation.hpp"

#include "Systems/Base/System.hpp"
#include "Systems/Base/GraphicsSystem.hpp"
#include "Systems/Base/TextSystem.hpp"
#include "Systems/Base/TextWindow.hpp"
#include "Systems/Base/EventSystem.hpp"
#include "Utilities/StringUtilities.hpp"

#include "libReallive/bytecode.h"

#include <vector>
#include <iostream>
#include <iterator>
#include <boost/bind.hpp>

using boost::bind;
using std::cin;
using std::cerr;
using std::distance;
using std::endl;
using std::vector;
using libReallive::SelectElement;
using libReallive::CommandElement;

// -----------------------------------------------------------------------

Sel_LongOperation::Sel_LongOperation(
  RLMachine& machine,
  const libReallive::SelectElement& commandElement)
  : machine_(machine),
    text_window_(machine.system().text().currentWindow()),
    return_value_(-1)
{
  machine.system().text().setInSelectionMode(true);
  text_window_->setVisible(true);
  text_window_->startSelectionMode();
  text_window_->setSelectionCallback(
    bind(&Sel_LongOperation::selected, this, _1));

  const vector<SelectElement::Param>& params = commandElement.getRawParams();
  for (unsigned int i = 0; i < params.size(); ++i) {
    std::string evaluated_native =
      libReallive::evaluatePRINT(machine, params[i].text);
    std::string utf8str = cp932toUTF8(evaluated_native,
                                      machine.getTextEncoding());

    options_.push_back(utf8str);
    text_window_->addSelectionItem(utf8str);
  }

  machine.system().graphics().markScreenAsDirty(GUT_TEXTSYS);
}

// -----------------------------------------------------------------------

Sel_LongOperation::~Sel_LongOperation()
{
  text_window_->endSelectionMode();
  machine_.system().text().setInSelectionMode(false);
}

// -----------------------------------------------------------------------

void Sel_LongOperation::selected(int num)
{
  return_value_ = num;
}

// -----------------------------------------------------------------------

bool Sel_LongOperation::selectOption(const std::string& str) {
  std::vector<std::string>::iterator it =
    find(options_.begin(), options_.end(), str);

  if (it != options_.end()) {
    selected(distance(options_.begin(), it));
    return true;
  }

  return false;
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

void Sel_LongOperation::mouseMotion(const Point& pos)
{
  // Tell the text system about the move
  machine_.system().text().setMousePosition(pos);
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
  // Prevent us from trying to parse the parameters to the CommandElement as
  // RealLive expressions (because they are not).
  virtual void parseParameters(
    const std::vector<std::string>& input,
    boost::ptr_vector<libReallive::ExpressionPiece>& output) {}

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
  addOpcode(1, 0, "select", new Sel_select);

  // TODO: These are wrong! These have different implementations which do more
  // graphical fun. Refer to the rldev manual once I get off my lazy ass to
  // implement them!
  addOpcode(0, 0, "select_w", new Sel_select);
  addOpcode(2, 0, "select_s2", new Sel_select);
  addOpcode(3, 0, "select_s", new Sel_select);
}
