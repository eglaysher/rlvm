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
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
//
// -----------------------------------------------------------------------

#include "Precompiled.hpp"

// -----------------------------------------------------------------------

/**
 * @file   RLModule.cpp
 * @author Elliot Glaysher
 * @date   Sat Oct  7 11:14:14 2006
 *
 * @brief  Definition of RLModule
 */

#include "MachineBase/RLModule.hpp"
#include "MachineBase/RLOperation.hpp"
#include "MachineBase/GeneralOperations.hpp"

#include "libReallive/bytecode.h"

#include <sstream>

using namespace std;
using namespace libReallive;

// -----------------------------------------------------------------------
// RLMoudle
// -----------------------------------------------------------------------

RLModule::RLModule(const std::string& in_module_name, int in_module_type,
                   int in_module_number)
  : module_type_(in_module_type), module_number_(in_module_number),
    module_name_(in_module_name)
{}

// -----------------------------------------------------------------------

RLModule::~RLModule()
{}

// -----------------------------------------------------------------------

int RLModule::packOpcodeNumber(int opcode, unsigned char overload)
{
  return ((int)opcode << 8) | overload;
}

// -----------------------------------------------------------------------

void RLModule::unpackOpcodeNumber(int packed_opcode, int& opcode, unsigned char& overload)
{
  opcode = (packed_opcode >> 8);
  overload = packed_opcode & 0xFF;
}

// -----------------------------------------------------------------------

void RLModule::addOpcode(int opcode, unsigned char overload, RLOperation* op)
{
  addOpcode(opcode, overload, "", op);
}

// -----------------------------------------------------------------------

void RLModule::addOpcode(int opcode, unsigned char overload,
                         const char* name, RLOperation* op)
{
  int packed_opcode = packOpcodeNumber(opcode, overload);
  op->setName(name);
  stored_operations.insert(packed_opcode, op);
}

// -----------------------------------------------------------------------

void RLModule::addUnsupportedOpcode(int opcode, unsigned char overload,
                                    const std::string& name)
{
  addOpcode(opcode, overload,
            new UndefinedFunction(name, module_type_, module_number_, opcode,
                                  (int)overload));
}

// -----------------------------------------------------------------------

void RLModule::dispatchFunction(RLMachine& machine, const CommandElement& f)
{
  OpcodeMap::iterator it = stored_operations.find(packOpcodeNumber(f.opcode(), f.overload()));
  if(it != stored_operations.end()) {
    it->second->dispatchFunction(machine, f);
  } else {
    throw rlvm::UnimplementedOpcode(f.modtype(), f.module(), f.opcode(),
                                    f.overload());
  }
}

// -----------------------------------------------------------------------

std::ostream& operator<<(std::ostream& os, const RLModule& module)
{
  os << "mod<" << module.moduleName() << "," << module.moduleType()
     << ":" << module.moduleNumber() << ">";
  return os;
}
