// -*- Mode: C++; tab-width:2; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi:tw=80:et:ts=2:sts=2
//
// -----------------------------------------------------------------------
//
// This file is part of RLVM, a RealLive virtual machine clone.
//
// -----------------------------------------------------------------------
//
// Copyright (C) 2009 Elliot Glaysher
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
// -----------------------------------------------------------------------

#include "Precompiled.hpp"

// -----------------------------------------------------------------------

#include "LongOperations/SelectLongOperation.hpp"

#include <vector>
#include <string>
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>

#include "MachineBase/LongOperation.hpp"
#include "Systems/Base/EventListener.hpp"
#include "Systems/Base/EventSystem.hpp"
#include "Systems/Base/GraphicsSystem.hpp"
#include "Systems/Base/Renderable.hpp"
#include "Systems/Base/System.hpp"
#include "Systems/Base/TextSystem.hpp"
#include "Systems/Base/TextWindow.hpp"
#include "Utilities/StringUtilities.hpp"
#include "libReallive/bytecode.h"

using boost::bind;
using boost::shared_ptr;
using std::string;
using std::vector;
using std::distance;
using libReallive::SelectElement;
using libReallive::CommandElement;

// -----------------------------------------------------------------------
// SelectLongOperation
// -----------------------------------------------------------------------
SelectLongOperation::SelectLongOperation(RLMachine& machine,
                                         const SelectElement& commandElement)
    : return_value_(-1) {
  const vector<SelectElement::Param>& params = commandElement.getRawParams();
  for (unsigned int i = 0; i < params.size(); ++i) {
    std::string evaluated_native =
        libReallive::evaluatePRINT(machine, params[i].text);
    std::string utf8str = cp932toUTF8(evaluated_native,
                                      machine.getTextEncoding());

    options_.push_back(utf8str);
  }
}

// -----------------------------------------------------------------------

void SelectLongOperation::selected(int num) {
  return_value_ = num;
}

// -----------------------------------------------------------------------

bool SelectLongOperation::selectOption(const std::string& str) {
  std::vector<std::string>::iterator it =
    find(options_.begin(), options_.end(), str);

  if (it != options_.end()) {
    selected(distance(options_.begin(), it));
    return true;
  }

  return false;
}

// -----------------------------------------------------------------------

bool SelectLongOperation::operator()(RLMachine& machine) {
  if (return_value_ != -1) {
    machine.setStoreRegister(return_value_);
    return true;
  } else {
    return false;
  }
}

// -----------------------------------------------------------------------
// NormalSelectLongOperation
// -----------------------------------------------------------------------
NormalSelectLongOperation::NormalSelectLongOperation(
    RLMachine& machine,
    const libReallive::SelectElement& commandElement)
    : SelectLongOperation(machine, commandElement),
      machine_(machine),
      text_window_(machine.system().text().currentWindow()) {
  machine.system().text().setInSelectionMode(true);
  text_window_->setVisible(true);
  text_window_->startSelectionMode();
  text_window_->setSelectionCallback(
    bind(&NormalSelectLongOperation::selected, this, _1));

  for (size_t i = 0; i < options_.size(); ++i) {
    text_window_->addSelectionItem(options_[i]);
  }

  machine.system().graphics().markScreenAsDirty(GUT_TEXTSYS);
}

// -----------------------------------------------------------------------

NormalSelectLongOperation::~NormalSelectLongOperation() {
  text_window_->endSelectionMode();
  machine_.system().text().setInSelectionMode(false);
}

// -----------------------------------------------------------------------

void NormalSelectLongOperation::mouseMotion(const Point& pos) {
  // Tell the text system about the move
  machine_.system().text().setMousePosition(pos);
}

// -----------------------------------------------------------------------

bool NormalSelectLongOperation::mouseButtonStateChanged(MouseButton mouseButton,
                                                        bool pressed) {
  EventSystem& es = machine_.system().event();

  switch (mouseButton) {
    case MOUSE_LEFT: {
      Point pos = es.getCursorPos();
      machine_.system().text().handleMouseClick(machine_, pos, pressed);
      return true;
      break;
    }
    case MOUSE_RIGHT: {
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
// ButtonSelectLongOperation
// -----------------------------------------------------------------------
ButtonSelectLongOperation::ButtonSelectLongOperation(
    RLMachine& machine,
    const libReallive::SelectElement& commandElement)
    : SelectLongOperation(machine, commandElement) {
}

// -----------------------------------------------------------------------

ButtonSelectLongOperation::~ButtonSelectLongOperation() {
}

// -----------------------------------------------------------------------

void ButtonSelectLongOperation::mouseMotion(const Point&) {
}

// -----------------------------------------------------------------------

bool ButtonSelectLongOperation::mouseButtonStateChanged(
    MouseButton mouseButton, bool pressed) {
}

// -----------------------------------------------------------------------

void ButtonSelectLongOperation::render(std::ostream* tree) {
}
