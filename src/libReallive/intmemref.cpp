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

#include "intmemref.h"

#include <iostream>
#include <stdexcept>

using namespace std;
using std::ostream;

namespace libReallive {

// -----------------------------------------------------------------------

IntMemRef::IntMemRef(int bytecodeRep, int location)
  : memoryBank(bytecodeRep % 26),  accessType(bytecodeRep / 26),
	loc(location)
{
  if(memoryBank == INTZ_LOCATION_IN_BYTECODE)
	memoryBank = INTZ_LOCATION;
  else if(memoryBank == INTL_LOCATION_IN_BYTECODE)
	memoryBank = INTL_LOCATION;
}

// -----------------------------------------------------------------------

IntMemRef::IntMemRef(int bank, int type, int location)
  : memoryBank(bank), accessType(type), loc(location)
{
}

// -----------------------------------------------------------------------

IntMemRef::IntMemRef(char bankName, int location)
  : accessType(0), loc(location)
{
  if(bankName >= 'A' && bankName <= 'G')
	memoryBank = bankName - 'A';
  else if(bankName == 'Z')
	memoryBank = INTZ_LOCATION;
  else if(bankName == 'L')
	memoryBank = INTL_LOCATION;
  else
	throw std::runtime_error("Invalid memory bank name.");
}

// -----------------------------------------------------------------------

IntMemRef::IntMemRef(char bankName, const char* accessStr, int location)
  : accessType(0), loc(location)
{
  if(bankName >= 'A' && bankName <= 'G')
	memoryBank = bankName - 'A';
  else if(bankName == 'Z')
	memoryBank = INTZ_LOCATION;
  else if(bankName == 'L')
	memoryBank = INTL_LOCATION;
  else
	throw std::runtime_error("Invalid memory bank name.");

  if(strcmp(accessStr, "") == 0)
	accessType = 0;
  else if(strcmp(accessStr, "b") == 0)
	accessType = 1;
  else if(strcmp(accessStr, "2b") == 0)
	accessType = 2;
  else if(strcmp(accessStr, "4b") == 0)
	accessType = 3;
  else if(strcmp(accessStr, "8b") == 0)
	accessType = 4;
  else 
	throw std::runtime_error("Invalid access type string.");
}

// -----------------------------------------------------------------------

IntMemRef::~IntMemRef()
{}

}

// -----------------------------------------------------------------------

std::ostream& operator<<(std::ostream& oss, const libReallive::IntMemRef& memref)
{
  using namespace libReallive;

  oss << "int";

  int bank = memref.bank();
  if(bank >= INTA_LOCATION && bank <= INTG_LOCATION)
	oss << char('A' + memref.bank());
  else if(bank == INTZ_LOCATION)
	oss << 'Z';
  else if(bank == INTL_LOCATION)
	oss << 'L';
  else 
	oss << "{Invalid bank# " << bank << "}";

  if(bank == 0)
	;
  else if(bank == 1)
	oss << "b";
  else if(bank == 2)
	oss << "2b";
  else if(bank == 3)
	oss << "4b";
  else if(bank == 4)
	oss << "8b";

  oss << '[' << memref.location() << ']';

  return oss;
}
