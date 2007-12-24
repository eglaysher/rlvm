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

RLModule::RLModule(const std::string& inModuleName, int inModuleType, 
                   int inModuleNumber)
  : m_moduleType(inModuleType), m_moduleNumber(inModuleNumber), 
    m_moduleName(inModuleName) 
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

void RLModule::unpackOpcodeNumber(int packedOpcode, int& opcode, unsigned char& overload)
{
  opcode = (packedOpcode >> 8);
  overload = packedOpcode & 0xFF;
}

// -----------------------------------------------------------------------

void RLModule::addOpcode(int opcode, unsigned char overload, RLOperation* op)
{
  ostringstream oss;
  oss << "opcode<" << m_moduleType << ":" << m_moduleNumber << ":" 
      << opcode << ", " << overload << ">";

  addOpcode(opcode, overload, oss.str(), op);
}

// -----------------------------------------------------------------------

void RLModule::addOpcode(int opcode, unsigned char overload, 
                         const std::string& name, RLOperation* op) 
{
  int packedOpcode = packOpcodeNumber(opcode, overload);  
  op->setName(name);
  storedOperations.insert(packedOpcode, op);
}

// -----------------------------------------------------------------------

void RLModule::addUnsupportedOpcode(int opcode, unsigned char overload, 
                                    const std::string& name)
{
  addOpcode(opcode, overload, 
            new UndefinedFunction(name, m_moduleType, m_moduleNumber, opcode, 
                                  (int)overload));
}

// -----------------------------------------------------------------------

void RLModule::dispatchFunction(RLMachine& machine, const CommandElement& f) 
{
  OpcodeMap::iterator it = storedOperations.find(packOpcodeNumber(f.opcode(), f.overload()));
  if(it != storedOperations.end()) {
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
