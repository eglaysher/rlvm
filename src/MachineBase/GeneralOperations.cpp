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
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
//
// -----------------------------------------------------------------------

#include "Precompiled.hpp"

// -----------------------------------------------------------------------

#include "MachineBase/GeneralOperations.hpp"

#include "Systems/Base/System.hpp"
#include "Systems/Base/GraphicsSystem.hpp"
#include "MachineBase/RLMachine.hpp"
#include "libReallive/gameexe.h"
#include "libReallive/bytecode.h"
#include "Utilities/Exception.hpp"

#include <boost/ptr_container/ptr_vector.hpp>

using namespace std;
using namespace boost;
using namespace libReallive;


namespace getSystemObjImpl {

template<>
System& getSystemObj(RLMachine& machine) {
  return machine.system();
}

template<>
EventSystem& getSystemObj(RLMachine& machine) {
  return machine.system().event();
}

template<>
GraphicsSystem& getSystemObj(RLMachine& machine) {
  return machine.system().graphics();
}

template<>
TextSystem& getSystemObj(RLMachine& machine) {
  return machine.system().text();
}

template<>
SoundSystem& getSystemObj(RLMachine& machine) {
  return machine.system().sound();
}

template<>
CGMTable& getSystemObj(RLMachine& machine) {
  return machine.system().graphics().cgTable();
}

}

// -----------------------------------------------------------------------

MultiDispatch::MultiDispatch(RLOperation* op)
    : handler_(op) {
}

// -----------------------------------------------------------------------

MultiDispatch::~MultiDispatch() {
}

// -----------------------------------------------------------------------

void MultiDispatch::parseParameters(
    const std::vector<std::string>& input,
    boost::ptr_vector<ExpressionPiece>& output) {
  for (vector<string>::const_iterator it = input.begin(); it != input.end();
       ++it)
  {
    const char* src = it->c_str();
    output.push_back(get_complex_param(src));
  }
}

// -----------------------------------------------------------------------

/// @todo Port this up to the new expression handling code
void MultiDispatch::operator()(
    RLMachine& machine,
    const libReallive::CommandElement& ff)
{
  const ptr_vector<ExpressionPiece>& parameter_pieces = ff.getParameters();

  for(unsigned int i = 0; i < parameter_pieces.size(); ++i) {
    const ptr_vector<ExpressionPiece>& element =
        dynamic_cast<const ComplexExpressionPiece&>(parameter_pieces[i]).
        getContainedPieces();

    // @todo Do whatever is needed to get this part working...
    //     if(!handler->checkTypes(machine, element)) {
    //       throw Error("Expected type mismatch in parameters in MultiDispatch.");
    //     }

    handler_->dispatch(machine, element);
  }

  machine.advanceInstructionPointer();
}

// -------------------------------------------------- [ ReturnGameexeInt ]

ReturnGameexeInt::ReturnGameexeInt(const std::string& full_key, int en)
    : full_key_name_(full_key), entry_(en) {
}

// -----------------------------------------------------------------------

int ReturnGameexeInt::operator()(RLMachine& machine) {
  Gameexe& gexe = machine.system().gameexe();
  vector<int> values = gexe(full_key_name_);
  if (static_cast<size_t>(entry_) < values.size()) {
    return values[entry_];
  } else {
    ostringstream oss;
    oss << "Could not access piece " << entry_ << " in Gameexe key \""
        << full_key_name_ << "\"";
    throw std::runtime_error(oss.str());
  }
}

// -------------------------------------------------- [ InvokeSyscomAsOp ]

InvokeSyscomAsOp::InvokeSyscomAsOp(const int syscom)
    : syscom_(syscom) {
}

// -----------------------------------------------------------------------

void InvokeSyscomAsOp::operator()(RLMachine& machine) {
  return machine.system().invokeSyscom(machine, syscom_);
}

// ------------------------------------------------- [ UndefinedFunction ]

UndefinedFunction::UndefinedFunction(
    const std::string& name,
    int modtype, int module, int opcode, int overload)
    : name_(name), modtype_(modtype), module_(module), opcode_(opcode),
      overload_(overload) {
}

// -----------------------------------------------------------------------

void UndefinedFunction::operator()(RLMachine&,
                                   const libReallive::CommandElement&) {
  throw rlvm::UnimplementedOpcode(name_, modtype_, module_, opcode_, overload_);
}
