// -*- Mode: C++; tab-width:2; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi:tw=80:et:ts=2:sts=2
//
// -----------------------------------------------------------------------
//
// This file is part of RLVM, a RealLive virtual machine clone.
//
// -----------------------------------------------------------------------
//
// Copyright (C) 2006, 2007, 2008 Elliot Glaysher
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

#include "machine/general_operations.h"

#include <string>
#include <vector>

#include "systems/base/system.h"
#include "systems/base/graphics_system.h"
#include "systems/base/text_system.h"
#include "machine/rlmachine.h"
#include "libreallive/gameexe.h"
#include "libreallive/bytecode.h"
#include "utilities/exception.h"

namespace getSystemObjImpl {

template <>
System& getSystemObj(RLMachine& machine) {
  return machine.system();
}

template <>
EventSystem& getSystemObj(RLMachine& machine) {
  return machine.system().event();
}

template <>
GraphicsSystem& getSystemObj(RLMachine& machine) {
  return machine.system().graphics();
}

template <>
TextSystem& getSystemObj(RLMachine& machine) {
  return machine.system().text();
}

template <>
SoundSystem& getSystemObj(RLMachine& machine) {
  return machine.system().sound();
}

template <>
CGMTable& getSystemObj(RLMachine& machine) {
  return machine.system().graphics().cgTable();
}

template <>
TextPage& getSystemObj(RLMachine& machine) {
  return machine.system().text().currentPage();
}

}  // namespace getSystemObjImpl

// -----------------------------------------------------------------------
// MultiDispatch
// -----------------------------------------------------------------------
MultiDispatch::MultiDispatch(RLOperation* op) : handler_(op) {}

MultiDispatch::~MultiDispatch() {}

void MultiDispatch::parseParameters(
    const std::vector<std::string>& input,
    libreallive::ExpressionPiecesVector& output) {
  for (auto const& parameter : input) {
    const char* src = parameter.c_str();
    output.push_back(libreallive::get_complex_param(src));
  }
}

/// @todo Port this up to the new expression handling code
void MultiDispatch::operator()(RLMachine& machine,
                               const libreallive::CommandElement& ff) {
  const libreallive::ExpressionPiecesVector& parameter_pieces =
      ff.getParameters();

  for (unsigned int i = 0; i < parameter_pieces.size(); ++i) {
    const libreallive::ExpressionPiecesVector& element =
        dynamic_cast<const libreallive::ComplexExpressionPiece&>(
            *parameter_pieces[i]).getContainedPieces();

    handler_->dispatch(machine, element);
  }

  machine.advanceInstructionPointer();
}

// -----------------------------------------------------------------------
// ReturnGameexeInt
// -----------------------------------------------------------------------
ReturnGameexeInt::ReturnGameexeInt(const std::string& full_key, int en)
    : full_key_name_(full_key), entry_(en) {}

int ReturnGameexeInt::operator()(RLMachine& machine) {
  Gameexe& gexe = machine.system().gameexe();
  std::vector<int> values = gexe(full_key_name_);
  if (static_cast<size_t>(entry_) < values.size()) {
    return values[entry_];
  } else {
    std::ostringstream oss;
    oss << "Could not access piece " << entry_ << " in Gameexe key \""
        << full_key_name_ << "\"";
    throw std::runtime_error(oss.str());
  }
}

// -----------------------------------------------------------------------
// InvokeSyscomAsOp
// -----------------------------------------------------------------------
InvokeSyscomAsOp::InvokeSyscomAsOp(const int syscom) : syscom_(syscom) {}

void InvokeSyscomAsOp::operator()(RLMachine& machine) {
  machine.system().invokeSyscom(machine, syscom_);
}

// -----------------------------------------------------------------------
// UndefinedFunction
// -----------------------------------------------------------------------
UndefinedFunction::UndefinedFunction(const std::string& name,
                                     int modtype,
                                     int module,
                                     int opcode,
                                     int overload)
    : name_(name),
      modtype_(modtype),
      module_(module),
      opcode_(opcode),
      overload_(overload) {}

void UndefinedFunction::dispatch(
    RLMachine& machine,
    const libreallive::ExpressionPiecesVector& parameters) {
  throw rlvm::UnimplementedOpcode(name_, modtype_, module_, opcode_, overload_);
}

void UndefinedFunction::dispatchFunction(RLMachine& machine,
                                         const libreallive::CommandElement& f) {
  throw rlvm::UnimplementedOpcode(machine, name_, f);
}

void UndefinedFunction::parseParameters(
    const std::vector<std::string>& input,
    libreallive::ExpressionPiecesVector& output) {
  throw rlvm::UnimplementedOpcode(name_, modtype_, module_, opcode_, overload_);
}

void UndefinedFunction::operator()(RLMachine& machine,
                                   const libreallive::CommandElement& f) {
  throw rlvm::UnimplementedOpcode(machine, name_, f);
}
