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
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
//
// -----------------------------------------------------------------------

#include "Modules/Module_Sel.hpp"

#include <vector>
#include <iterator>
#include <boost/bind.hpp>
#include <string>

#include "LongOperations/ButtonObjectSelectLongOperation.hpp"
#include "LongOperations/SelectLongOperation.hpp"
#include "MachineBase/RLMachine.hpp"
#include "MachineBase/RLOperation.hpp"
#include "Systems/Base/EventSystem.hpp"
#include "Systems/Base/GraphicsSystem.hpp"
#include "Systems/Base/System.hpp"
#include "Systems/Base/TextSystem.hpp"
#include "Systems/Base/TextWindow.hpp"
#include "Utilities/StringUtilities.hpp"
#include "libReallive/bytecode.h"
#include "libReallive/gameexe.h"

using boost::bind;
using libReallive::SelectElement;
using libReallive::CommandElement;

namespace {

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

struct ClearAndRestoreWindow : public LongOperation {
  int to_restore_;
  ClearAndRestoreWindow(int in) : to_restore_(in) {}

  bool operator()(RLMachine& machine) {
    machine.system().text().hideAllTextWindows();
    machine.system().text().setActiveWindow(to_restore_);
    return true;
  }
};

struct Sel_select_w : public RLOp_SpecialCase {
  // Prevent us from trying to parse the parameters to the CommandElement as
  // RealLive expressions (because they are not).
  virtual void parseParameters(
    const std::vector<std::string>& input,
    boost::ptr_vector<libReallive::ExpressionPiece>& output) {}

  void operator()(RLMachine& machine, const CommandElement& ce) {
    if (machine.shouldSetSelcomSavepoint())
      machine.markSavepoint();

    const SelectElement& element = dynamic_cast<const SelectElement&>(ce);

    // Sometimes the RL bytecode will override DEFAULT_SEL_WINDOW.
    int window = machine.system().gameexe()("DEFAULT_SEL_WINDOW").to_int(-1);
    libReallive::ExpressionElement window_exp = element.window();
    int computed = window_exp.valueOnly(machine);
    if (computed != -1)
      window = computed;

    // Restore the previous text state after the select operation completes.
    TextSystem& text = machine.system().text();
    int active_window = text.activeWindow();
    text.hideAllTextWindows();
    text.setActiveWindow(window);
    machine.pushLongOperation(new ClearAndRestoreWindow(active_window));

    machine.pushLongOperation(new NormalSelectLongOperation(machine, element));
    machine.advanceInstructionPointer();
  }
};

struct Sel_select_objbtn : public RLOp_Void_1<IntConstant_T> {
  void operator()(RLMachine& machine, int group) {
    if (machine.shouldSetSelcomSavepoint())
      machine.markSavepoint();

    machine.pushLongOperation(
        new ButtonObjectSelectLongOperation(machine, group));
  }
};

struct Sel_select_objbtn_cancel_0 : public RLOp_Void_1<IntConstant_T> {
  void operator()(RLMachine& machine, int group) {
    if (machine.shouldSetSelcomSavepoint())
      machine.markSavepoint();

    ButtonObjectSelectLongOperation* obj =
        new ButtonObjectSelectLongOperation(machine, group);
    obj->set_cancelable();
    machine.pushLongOperation(obj);
  }
};


struct Sel_select_objbtn_cancel_1
    : public RLOp_Void_2<IntConstant_T, IntConstant_T> {
  void operator()(RLMachine& machine, int group, int se) {
    if (machine.shouldSetSelcomSavepoint())
      machine.markSavepoint();

    ButtonObjectSelectLongOperation* obj =
        new ButtonObjectSelectLongOperation(machine, group);
    obj->set_cancelable();
    machine.pushLongOperation(obj);
  }
};

// Our system doesn't need an explicit initialize.
struct objbtn_init_0 : public RLOp_Void_1<IntConstant_T> {
  void operator()(RLMachine& machine, int ignored) {}
};

struct objbtn_init_1 : public RLOp_Void_Void {
  void operator()(RLMachine& machine) {}
};

}  // namespace

SelModule::SelModule()
    : RLModule("Sel", 0, 2) {
  addOpcode(0, 0, "select_w", new Sel_select_w);
  addOpcode(1, 0, "select", new Sel_select);
  addOpcode(2, 0, "select_s2", new Sel_select_s);
  addOpcode(3, 0, "select_s", new Sel_select_s);
  addOpcode(4, 0, "select_objbtn", new Sel_select_objbtn);
  addOpcode(14, 0, "select_objbtn_cancel", new Sel_select_objbtn_cancel_0);
  addOpcode(14, 1, "select_objbtn_cancel", new Sel_select_objbtn_cancel_1);

  addOpcode(20, 0, "objbtn_init", new objbtn_init_0);
  addOpcode(20, 1, "objbtn_init", new objbtn_init_1);
}
