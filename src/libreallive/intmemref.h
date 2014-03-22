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

#ifndef SRC_LIBREALLIVE_INTMEMREF_H_
#define SRC_LIBREALLIVE_INTMEMREF_H_

#include <iosfwd>

namespace libreallive {

const int INTA_LOCATION = 0;
const int INTB_LOCATION = 1;
const int INTC_LOCATION = 2;
const int INTD_LOCATION = 3;
const int INTE_LOCATION = 4;
const int INTF_LOCATION = 5;
const int INTG_LOCATION = 6;

const int INTZ_LOCATION = 7;
const int INTL_LOCATION = 8;

// -----------------------------------------------------------------------

const int INTZ_LOCATION_IN_BYTECODE = 25;
const int INTL_LOCATION_IN_BYTECODE = 11;

// -----------------------------------------------------------------------

const int STRK_LOCATION = 0x0A;
const int STRM_LOCATION = 0x0C;
const int STRS_LOCATION = 0x12;

// -----------------------------------------------------------------------

inline bool isStringLocation(const int type) {
  return type == STRS_LOCATION || type == STRK_LOCATION ||
      type == STRM_LOCATION;
}

// -----------------------------------------------------------------------

// References a piece of integer memory.
class IntMemRef {
 public:
  IntMemRef(int bytecodeRep, int location);
  IntMemRef(int bank, int type, int location);

  IntMemRef(char bankName, int location);
  IntMemRef(char bankName, const char* accessStr, int location);
  ~IntMemRef();

  int bank() const { return memoryBank; }
  int type() const { return accessType; }
  int location() const { return loc;    }

 private:
  // Which piece of memory to operate on.
  int memoryBank;

  // How to access
  int accessType;

  // The memory location to
  int loc;
};

}  // namespace libreallive

std::ostream& operator<<(std::ostream& oss,
                         const libreallive::IntMemRef& memref);

#endif  // SRC_LIBREALLIVE_INTMEMREF_H_
