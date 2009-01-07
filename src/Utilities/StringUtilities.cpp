// -*- Mode: C++; tab-width:2; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi:tw=80:et:ts=2:sts=2
//
// -----------------------------------------------------------------------
//
// This file is part of RLVM, a RealLive virtual machine clone.
//
// -----------------------------------------------------------------------
//
// Copyright (C) 2008 Elliot Glaysher
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

#include "Utilities.h"
#include "Utilities/StringUtilities.hpp"

// -----------------------------------------------------------------------

void advanceOneShiftJISChar(const char*& c)
{
  if(shiftjis_lead_byte(c[0]))
  {
    if(c[1] == '\0')
      throw rlvm::Exception("Malformed Shift_JIS string!");
    else
      c += 2;
  }
  else
    c += 1;
}

// -----------------------------------------------------------------------

void copyOneShiftJisCharacter(const char*& str, std::string& output)
{
  if(shiftjis_lead_byte(str[0]))
  {
    if(str[1] == '\0')
      throw rlvm::Exception("Malformed Shift_JIS string!");
    else {
      output += *str++;
      output += *str++;
    }
  }
  else
    output += *str++;
}

// -----------------------------------------------------------------------

bool readFullwidthLatinLetter(const char*& str, std::string& output)
{
  // The fullwidth uppercase latin characters are 0x8260 through 0x8279.
  if (str[0] == 0x82) {
    if (str[1] == 0) {
      throw rlvm::Exception("Malformed Shift_JIS string!");
    } else if (str[1] >= 0x60 && str[1] <= 0x79) {
      char printable = str[1] - 0x1F;
      output += printable;
      str += 2;
      return true;
    }
  }

  return false;
}

// -----------------------------------------------------------------------

void addShiftJISChar(unsigned short c, std::string& output) {
  if (c > 0xFF)
    output += (c >> 8);
  output += (c & 0xFF);
}
