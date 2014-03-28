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

#include "libreallive/intmemref.h"

#include <cstring>
#include <ostream>
#include <stdexcept>

namespace libreallive {

// -----------------------------------------------------------------------

IntMemRef::IntMemRef(int bytecode_rep, int location)
    : memory_bank_(bytecode_rep % 26),
      access_type_(bytecode_rep / 26),
      location_(location) {
  if (memory_bank_ == INTZ_LOCATION_IN_BYTECODE)
    memory_bank_ = INTZ_LOCATION;
  else if (memory_bank_ == INTL_LOCATION_IN_BYTECODE)
    memory_bank_ = INTL_LOCATION;
}

// -----------------------------------------------------------------------

IntMemRef::IntMemRef(int bank, int type, int location)
    : memory_bank_(bank), access_type_(type), location_(location) {}

// -----------------------------------------------------------------------

IntMemRef::IntMemRef(char bank_name, int location)
    : access_type_(0), location_(location) {
  if (bank_name >= 'A' && bank_name <= 'G')
    memory_bank_ = bank_name - 'A';
  else if (bank_name == 'Z')
    memory_bank_ = INTZ_LOCATION;
  else if (bank_name == 'L')
    memory_bank_ = INTL_LOCATION;
  else
    throw std::runtime_error("Invalid memory bank name.");
}

// -----------------------------------------------------------------------

IntMemRef::IntMemRef(char bank_name, const char* access_str, int location)
    : access_type_(0), location_(location) {
  if (bank_name >= 'A' && bank_name <= 'G')
    memory_bank_ = bank_name - 'A';
  else if (bank_name == 'Z')
    memory_bank_ = INTZ_LOCATION;
  else if (bank_name == 'L')
    memory_bank_ = INTL_LOCATION;
  else
    throw std::runtime_error("Invalid memory bank name.");

  if (strcmp(access_str, "") == 0)
    access_type_ = 0;
  else if (strcmp(access_str, "b") == 0)
    access_type_ = 1;
  else if (strcmp(access_str, "2b") == 0)
    access_type_ = 2;
  else if (strcmp(access_str, "4b") == 0)
    access_type_ = 3;
  else if (strcmp(access_str, "8b") == 0)
    access_type_ = 4;
  else
    throw std::runtime_error("Invalid access type string.");
}

// -----------------------------------------------------------------------

IntMemRef::~IntMemRef() {}

}  // namespace libreallive

// -----------------------------------------------------------------------

std::ostream& operator<<(std::ostream& oss,
                         const libreallive::IntMemRef& memref) {
  oss << "int";

  int bank = memref.bank();
  if (bank >= libreallive::INTA_LOCATION && bank <= libreallive::INTG_LOCATION)
    oss << char('A' + memref.bank());
  else if (bank == libreallive::INTZ_LOCATION)
    oss << 'Z';
  else if (bank == libreallive::INTL_LOCATION)
    oss << 'L';
  else
    oss << "{Invalid bank# " << bank << "}";

  if (bank == 0) {
  } else if (bank == 1) {
    oss << "b";
  } else if (bank == 2) {
    oss << "2b";
  } else if (bank == 3) {
    oss << "4b";
  } else if (bank == 4) {
    oss << "8b";
  }

  oss << '[' << memref.location() << ']';

  return oss;
}
