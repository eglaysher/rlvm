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
#include "Utilities/Exception.hpp"
#include "Utilities/StringUtilities.hpp"
#include "libReallive/intmemref.h"
#include "libReallive/gameexe.h"
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
GlobalMemory::GlobalMemory() {
  memset(intG, 0, sizeof(intG));
  memset(intZ, 0, sizeof(intZ));
}

// -----------------------------------------------------------------------
// LocalMemory
// -----------------------------------------------------------------------
LocalMemory::LocalMemory() {
  reset();
}

// -----------------------------------------------------------------------

LocalMemory::LocalMemory(dont_initialize) {
}

// -----------------------------------------------------------------------

void LocalMemory::reset() {
  memset(intA, 0, sizeof(intA));
  memset(intB, 0, sizeof(intB));
  memset(intC, 0, sizeof(intC));
  memset(intD, 0, sizeof(intD));
  memset(intE, 0, sizeof(intE));
  memset(intF, 0, sizeof(intF));

  memset(intL, 0, sizeof(intL));

  for (int i = 0; i < SIZE_OF_MEM_BANK; ++i)
    strS[i].clear();
  for (int i = 0; i < 3; ++i)
    strK[i].clear();
  for (int i = 0; i < SIZE_OF_NAME_BANK; ++i)
    local_names[i].clear();
}

// -----------------------------------------------------------------------
// Memory
// -----------------------------------------------------------------------
Memory::Memory(Gameexe& gameexe)
  : global_(new GlobalMemory), local_() {
  connectIntVarPointers();

  initializeDefaultValues(gameexe);
}

// -----------------------------------------------------------------------

Memory::Memory(RLMachine& machine, int slot)
  : global_(machine.memory().global_), local_(dont_initialize()) {
  connectIntVarPointers();
}

// -----------------------------------------------------------------------

Memory::~Memory() {
}

// -----------------------------------------------------------------------

void Memory::connectIntVarPointers() {
  int_var[0] = local_.intA;
  int_var[1] = local_.intB;
  int_var[2] = local_.intC;
  int_var[3] = local_.intD;
  int_var[4] = local_.intE;
  int_var[5] = local_.intF;
  int_var[6] = global_->intG;
  int_var[7] = global_->intZ;
  int_var[8] = local_.intL;
}

// -----------------------------------------------------------------------

const std::string& Memory::getStringValue(int type, int location) {
  if (location > (SIZE_OF_MEM_BANK -1))
    throw rlvm::Exception("Invalid range access in RLMachine::set_string_value");

  switch (type) {
  case STRK_LOCATION:
    if (location > 2)
      throw rlvm::Exception("Invalid range access on strK in RLMachine::set_string_value");
    return local_.strK[location];
  case STRM_LOCATION: return global_->strM[location];
  case STRS_LOCATION: return local_.strS[location];
  default:
    throw rlvm::Exception("Invalid type in RLMachine::get_string_value");
  }
}

// -----------------------------------------------------------------------

void Memory::setStringValue(int type, int number, const std::string& value) {
  if (number > (SIZE_OF_MEM_BANK -1))
      throw rlvm::Exception("Invalid range access in RLMachine::set_string_value");

  switch (type) {
  case STRK_LOCATION:
    if (number > 2)
      throw rlvm::Exception("Invalid range access on strK in RLMachine::set_string_value");
    local_.strK[number] = value;
    break;
  case STRM_LOCATION:
    global_->strM[number] = value;
    break;
  case STRS_LOCATION:
    local_.strS[number] = value;
    break;
  default:
    throw rlvm::Exception("Invalid type in RLMachine::set_string_value");
  }
}

// -----------------------------------------------------------------------

void Memory::checkNameIndex(int index, const std::string& name) const {
  if (index > (SIZE_OF_NAME_BANK - 1)) {
    ostringstream oss;
    oss << "Invalid index " << index << " in " << name;
    throw rlvm::Exception(oss.str());
  }
}

// -----------------------------------------------------------------------

void Memory::setName(int index, const std::string& name) {
  checkNameIndex(index, "Memory::set_name");
  global_->global_names[index] = name;
}

// -----------------------------------------------------------------------

const std::string& Memory::getName(int index) const {
  checkNameIndex(index, "Memory::get_name");
  return global_->global_names[index];
}

// -----------------------------------------------------------------------

void Memory::setLocalName(int index, const std::string& name) {
  checkNameIndex(index, "Memory::set_local_name");
  local_.local_names[index] = name;
}

// -----------------------------------------------------------------------

const std::string& Memory::getLocalName(int index) const {
  checkNameIndex(index, "Memory::set_local_name");
  return local_.local_names[index];
}

// -----------------------------------------------------------------------

bool Memory::hasBeenRead(int scenario, int kidoku) const {
  std::map<int, boost::dynamic_bitset<> >::const_iterator it =
    global_->kidoku_data.find(scenario);

  if ((it != global_->kidoku_data.end()) &&
      (static_cast<size_t>(kidoku) < it->second.size()))
    return it->second.test(kidoku);

  return false;
}

// -----------------------------------------------------------------------

void Memory::recordKidoku(int scenario, int kidoku) {
  boost::dynamic_bitset<>& bitset = global_->kidoku_data[scenario];
  if (bitset.size() <= static_cast<size_t>(kidoku))
    bitset.resize(kidoku + 1, false);

  bitset[kidoku] = true;
}

// -----------------------------------------------------------------------

/* static */
int Memory::ConvertLetterIndexToInt(const std::string& value) {
  int total = 0;

  if (value.size() == 1) {
    total += (value[0] - 'A');
  } else if (value.size() == 2) {
    total += 26 * ((value[0] - 'A') + 1);
    total += (value[1] - 'A');
  } else {
    throw rlvm::Exception("Invalid value in convert_name_var!");
  }

  return total;
}

// -----------------------------------------------------------------------

void Memory::initializeDefaultValues(Gameexe& gameexe) {
  // Note: We ignore the \#NAME_MAXLEN variable because manual allocation is
  // error prone and for losers.
  GameexeFilteringIterator end = gameexe.filtering_end();
  for (GameexeFilteringIterator it = gameexe.filtering_begin("NAME.");
      it != end; ++it) {
    try {
      setName(ConvertLetterIndexToInt(it->key_parts().at(1)),
              removeQuotes(it->to_string()));
    } catch(...) {
      cerr << "WARNING: Invalid format for key " << it->key() << endl;
    }
  }

  for (GameexeFilteringIterator it = gameexe.filtering_begin("LOCALNAME.");
      it != end; ++it) {
    try {
      setLocalName(ConvertLetterIndexToInt(it->key_parts().at(1)),
                   removeQuotes(it->to_string()));
    } catch(...) {
      cerr << "WARNING: Invalid format for key " << it->key() << endl;
    }
  }
}
