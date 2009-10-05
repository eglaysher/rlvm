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

#include "LongOperations/SelectLongOperation.hpp"
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
#include <iterator>
#include <boost/bind.hpp>
#include <string>

using boost::bind;
using libReallive::SelectElement;
using libReallive::CommandElement;

// -----------------------------------------------------------------------

struct Sel_select : public RLOp_SpecialCase {
  // Prevent us from trying to parse the parameters to the CommandElement as
  // RealLive expressions (because they are not).
  virtual void parseParameters(
    const std::vector<std::string>& input,
    boost::ptr_vector<libReallive::ExpressionPiece>& output) {}

  void operator()(RLMachine& machine, const CommandElement& ce) {
    if (machine.shouldSetSelcomSavepoint())
      machine.markSavepoint();

    const SelectElement& element = dynamic_cast<const SelectElement&>(ce);
    machine.pushLongOperation(new NormalSelectLongOperation(machine, element));
    machine.advanceInstructionPointer();
  }
};

// -----------------------------------------------------------------------

struct Sel_select_s : public RLOp_SpecialCase {
  // Prevent us from trying to parse the parameters to the CommandElement as
  // RealLive expressions (because they are not).
  virtual void parseParameters(
    const std::vector<std::string>& input,
    boost::ptr_vector<libReallive::ExpressionPiece>& output) {}

  void operator()(RLMachine& machine, const CommandElement& ce) {
    if (machine.shouldSetSelcomSavepoint())
      machine.markSavepoint();

    const SelectElement& element = dynamic_cast<const SelectElement&>(ce);
    machine.pushLongOperation(
        new ButtonSelectLongOperation(machine, element, 0));
    machine.advanceInstructionPointer();
  }
};

// -----------------------------------------------------------------------

SelModule::SelModule()
    : RLModule("Sel", 0, 2) {
  addOpcode(1, 0, "select", new Sel_select);

  // TODO: These are wrong! These have different implementations which do more
  // graphical fun. Refer to the rldev manual once I get off my lazy ass to
  // implement them!
  addOpcode(0, 0, "select_w", new Sel_select);
  addOpcode(2, 0, "select_s2", new Sel_select_s);
  addOpcode(3, 0, "select_s", new Sel_select_s);
}
