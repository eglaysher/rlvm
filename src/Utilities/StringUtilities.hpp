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

#ifndef __StringUtilities_hpp__
#define __StringUtilities_hpp__

#include <string>

/**
 * Checks to see if the byte c is the first byte of a two byte
 * character. RealLive encodes its strings in Shift_JIS, so we have to
 * deal with this character encoding.
 */
inline bool shiftjis_lead_byte(const char c)
{
  return (c >= 0x81 && c <= 0x9f) || (c >= 0xe0 && c <= 0xfc);
}

/**
 * Advanced the Shift_JIS character string c by one char.
 *
 * @param c Pointer to the current character in the string
 */
void advanceOneShiftJISChar(const char*& c);

/**
 * Copies a single Shift_JIS character into output and advances the string.
 *
 * @param str The input character string
 * @param output The output std::string
 */
void copyOneShiftJisCharacter(const char*& str, std::string& output);

/**
 * If there is currently a two byte fullwidth Latin capital letter at the
 * current point in str (a shift_jis string), then convert it to ASCII, copy it
 * to output and return true. Returns false otherwise.
 */
bool readFullwidthLatinLetter(const char*& str, std::string& output);

#endif
