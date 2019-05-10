// -*- Mode: C++; tab-width:2; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi:tw=80:et:ts=2:sts=2
//
// -----------------------------------------------------------------------
//
// This file is part of RLVM, a RealLive virtual machine clone.
//
// -----------------------------------------------------------------------
//
// Copyright (C) 2006, 2007 Elliot Glaysher
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

#include "machine/rlmodule.h"

#include <iomanip>
#include <iostream>
#include <utility>
#include <sstream>
#include <string>
#include <vector>

#include "libreallive/bytecode.h"
#include "machine/general_operations.h"
#include "machine/rloperation.h"
#include "utilities/exception.h"

// -----------------------------------------------------------------------
// RLMoudle
// -----------------------------------------------------------------------

RLModule::RLModule(const std::string& in_module_name,
                   int in_module_type,
                   int in_module_number)
    : property_list_(),
      module_type_(in_module_type),
      module_number_(in_module_number),
      module_name_(in_module_name) {}

RLModule::~RLModule() {}

int RLModule::PackOpcodeNumber(int opcode, unsigned char overload) {
  return ((int)opcode << 8) | overload;
}

void RLModule::UnpackOpcodeNumber(int packed_opcode,
                                  int& opcode,
                                  unsigned char& overload) {
  opcode = (packed_opcode >> 8);
  overload = packed_opcode & 0xFF;
}

void RLModule::AddOpcode(int opcode,
                         unsigned char overload,
                         const std::string& name,
                         RLOperation* op) {
  int packed_opcode = PackOpcodeNumber(opcode, overload);
  op->set_name(name);
  op->module_ = this;
#ifndef NDEBUG
  OpcodeMap::iterator it = stored_operations_.find(packed_opcode);

  if (it != stored_operations_.end()) {
    std::ostringstream oss;
    oss << "Duplicate opcode in " << *this << ": opcode " << opcode << ", "
        << int(overload);
    throw rlvm::Exception(oss.str());
  }
#endif
  stored_operations_.emplace(packed_opcode, std::unique_ptr<RLOperation>(op));
}

void RLModule::AddUnsupportedOpcode(int opcode,
                                    unsigned char overload,
                                    const std::string& name) {
  AddOpcode(opcode,
            overload,
            name,
            new UndefinedFunction(
                module_type_, module_number_, opcode, (int)overload));
}

void RLModule::SetProperty(int property, int value) {
  if (!property_list_) {
    property_list_.reset(new std::vector<std::pair<int, int>>);
  }

  // Modify the property if it already exists
  PropertyList::iterator it = FindProperty(property);
  if (it != property_list_->end()) {
    it->second = value;
    return;
  }

  property_list_->emplace_back(property, value);
}

bool RLModule::GetProperty(int property, int& value) const {
  if (property_list_) {
    PropertyList::iterator it = FindProperty(property);
    if (it != property_list_->end()) {
      value = it->second;
      return true;
    }
  }

  return false;
}

RLModule::PropertyList::iterator RLModule::FindProperty(int property) const {
  return find_if(property_list_->begin(),
                 property_list_->end(),
                 [&](Property& p) { return p.first == property; });
}

std::string RLModule::GetCommandName(RLMachine& machine,
                                     const libreallive::CommandElement& f) {
  OpcodeMap::iterator it =
      stored_operations_.find(PackOpcodeNumber(f.opcode(), f.overload()));
  std::string name;
  if (it != stored_operations_.end())
    name = it->second->name();
  return name;
}

void RLModule::DispatchFunction(RLMachine& machine,
                                const libreallive::CommandElement& f) {
  OpcodeMap::iterator it =
      stored_operations_.find(PackOpcodeNumber(f.opcode(), f.overload()));
  if (it != stored_operations_.end()) {
    try {
      if (machine.is_tracing_on()) {
        std::cerr << "(SEEN" << std::setw(4) << std::setfill('0')
                  << machine.SceneNumber()
                  << ")(Line " << std::setw(4) << std::setfill('0')
                  << machine.line_number() << "): " << it->second->name();
        libreallive::PrintParameterString(std::cerr,
                                          f.GetUnparsedParameters());
        std::cerr << std::endl;
      }
      it->second->DispatchFunction(machine, f);
    }
    catch (rlvm::Exception& e) {
      e.setOperation(it->second.get());
      throw;
    }
  } else {
    throw rlvm::UnimplementedOpcode(machine, f);
  }
}

std::ostream& operator<<(std::ostream& os, const RLModule& module) {
  os << "mod<" << module.module_name() << "," << module.module_type() << ":"
     << module.module_number() << ">";
  return os;
}
