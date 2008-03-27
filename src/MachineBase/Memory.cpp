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

#include "MachineBase/Memory.hpp"
#include "MachineBase/RLMachine.hpp"
#include "libReallive/intmemref.h"
#include "Utilities.h"
#include <boost/assign/list_of.hpp>

// -----------------------------------------------------------------------

using boost::assign::list_of;
using namespace std;
using namespace libReallive;

// -----------------------------------------------------------------------

const IntegerBank_t LOCAL_INTEGER_BANKS =
  list_of(make_pair(INTB_LOCATION, 'A'))
  (make_pair(INTB_LOCATION, 'B'))
  (make_pair(INTC_LOCATION, 'C'))
  (make_pair(INTD_LOCATION, 'D'))
  (make_pair(INTE_LOCATION, 'E'))
  (make_pair(INTF_LOCATION, 'F'));

const IntegerBank_t GLOBAL_INTEGER_BANKS =
  list_of(make_pair(INTG_LOCATION, 'G'))
  (make_pair(INTZ_LOCATION, 'Z'));

// -----------------------------------------------------------------------
// GlobalMemory
// -----------------------------------------------------------------------
GlobalMemory::GlobalMemory()
{
  memset(intG, 0, sizeof(intG));
  memset(intZ, 0, sizeof(intZ));
}

// -----------------------------------------------------------------------
// LocalMemory
// -----------------------------------------------------------------------
LocalMemory::LocalMemory()
{
  memset(intA, 0, sizeof(intA));  
  memset(intB, 0, sizeof(intB));  
  memset(intC, 0, sizeof(intC));  
  memset(intD, 0, sizeof(intD));  
  memset(intE, 0, sizeof(intE));  
  memset(intF, 0, sizeof(intF));  

  memset(intL, 0, sizeof(intL));
}

// -----------------------------------------------------------------------

LocalMemory::LocalMemory(dont_initialize)
{
}

// -----------------------------------------------------------------------
// Memory
// -----------------------------------------------------------------------
Memory::Memory()
  : m_global(new GlobalMemory), m_local()
{
  connectIntVarPointers();
}

// -----------------------------------------------------------------------

Memory::Memory(RLMachine& machine, int slot)
  : m_global(machine.memory().m_global), m_local(dont_initialize())
{
  connectIntVarPointers();
}

// -----------------------------------------------------------------------

Memory::~Memory()
{
}

// -----------------------------------------------------------------------

void Memory::connectIntVarPointers()
{
  intVar[0] = m_local.intA;
  intVar[1] = m_local.intB;
  intVar[2] = m_local.intC;
  intVar[3] = m_local.intD;
  intVar[4] = m_local.intE;
  intVar[5] = m_local.intF;
  intVar[6] = m_global->intG;
  intVar[7] = m_global->intZ;
  intVar[8] = m_local.intL;
}

// -----------------------------------------------------------------------

const std::string& Memory::getStringValue(int type, int location) 
{
  if(location > (SIZE_OF_MEM_BANK -1))
      throw rlvm::Exception("Invalid range access in RLMachine::setStringValue");

  switch(type) {
  case STRK_LOCATION:
    if(location > 2)
      throw rlvm::Exception("Invalid range access on strK in RLMachine::setStringValue");
    return m_local.strK[location];
  case STRM_LOCATION: return m_global->strM[location];
  case STRS_LOCATION: return m_local.strS[location];
  default:
    throw rlvm::Exception("Invalid type in RLMachine::getStringValue");
  }
}

// -----------------------------------------------------------------------

void Memory::setStringValue(int type, int number, const std::string& value) 
{
  if(number > (SIZE_OF_MEM_BANK -1))
      throw rlvm::Exception("Invalid range access in RLMachine::setStringValue");

  switch(type) {
  case STRK_LOCATION:
    if(number > 2)
      throw rlvm::Exception("Invalid range access on strK in RLMachine::setStringValue");
    m_local.strK[number] = value;
    break;
  case STRM_LOCATION:
    m_global->strM[number] = value;
    break;
  case STRS_LOCATION: 
    m_local.strS[number] = value;
    break;
  default:
    throw rlvm::Exception("Invalid type in RLMachine::setStringValue");
  }     
}

// -----------------------------------------------------------------------


