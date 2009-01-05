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

#include "Precompiled.hpp"

// -----------------------------------------------------------------------

/**
 * @file   cp932toUnicode.cpp
 * @date   Sun Sep 17 14:10:35 2006
 */

#include <string>
#include <iostream>
#include <stdexcept>

#include "utf8.h"
#include "Systems/Base/SystemError.hpp"

#include "Encodings/codepage.h"

using namespace std;

// -----------------------------------------------------------------------

/**
 * Converts a CP932/Shift_JIS string into a wstring with Unicode
 * characters.
 *
 * If the string was not CP932, but actually another encoding
 * transformed such that it can be processed as CP932, this additional
 * transformation should be reversed here.  This technique is how
 * non-Japanese text is used in RealLive.  Transformations that might
 * potentially be encountered are:
 *
 *   0 - plain CP932 (no transformation)
 *   1 - CP936
 *   2 - CP1252 (also requires rlBabel support)
 *   3 - CP949
 *
 * These are the transformations applied by RLdev, and translation
 * tables can be found in the rlBabel source code.  There are also at
 * least two CP936 transformations used by the Key Fans Club and
 * possibly one or more other CP949 transformations, but details of
 * these are not publicly available.
 *
 * @param line Input string in CP932 encoding
 * @param transformation Additional encoding transformation
 * @return Equivalent string in Unicode
 */
wstring cp932toUnicode(const std::string& line, int transformation)
{
  return Cp::instance(transformation).ConvertString(line);
}

// -----------------------------------------------------------------------

string transformationName(int transformation) {
  switch(transformation) {
    case 0:
      return "Japanese (Cp932)";
    case 1:
      return "Chinese (Cp936)";
    case 2:
      return "Western";
    case 3:
      return "Korean (Cp949)";
    default:
      return "Unknown";
  }
}

// -----------------------------------------------------------------------

// All hankaku characters are <U+FF??>,
// while all zenkaku characters are <U+30??>
//
// hankaku characters that need translation are in the range 0xFF65 to
// 0x9F inclusive and continuous, so this table starts at 0xFF65.
char han2zen_table[] = {
  0xFB,   0xF2,   0xA1,   0xA3,   0xA5,   0xA7,   0xA9,   0xE3,
  0xE5,   0xE7,   0xC3,   0xFC,   0xA2,   0xA4,   0xA6,   0xA8,
  0xAA,   0xAB,   0xAD,   0xAF,   0xB1,   0xB3,   0xB5,   0xB7,
  0xB9,   0xBB,   0xBD,   0xBF,   0xC1,   0xC4,   0xC6,   0xC8,
  0xCA,   0xCB,   0xCC,   0xCD,   0xCE,   0xCF,   0xD2,   0xD5,
  0xD8,   0xDB,   0xDE,   0xDF,   0xE0,   0xE1,   0xE2,   0xE4,
  0xE6,   0xE8,   0xE9,   0xEA,   0xEB,   0xEC,   0xED,   0xEF,
  0xF3,   0x9B,   0x0C
};

/**
 * Converts a single half-width unicode character to its full-width
 * equivalent.
 *
 * @param input Input character
 * @return Converted character (or input character if no translation
 * neccessary).
 */
wchar_t hantozen_wchar(wchar_t input)
{
  if(input == 0x0020)
  {
    input = 0x3000;
  }
  else if(input >= '!' && input <= '~')
  {
    input += 0xFEE0;
  }
  // Need to build a conversion by reading the cp932 ucm file, and then building
  // a mapping between "^KATAKANA X" and "^HALFWIDTH KATAKANA X"
  else if(input >= 0xFF65 && input <= 0xFF9F)
  {
    input = (0x30 << 8) | han2zen_table[input - 0xFF65];
  }

  return input;
}

// -----------------------------------------------------------------------

/**
 * Converts half-width ASCII and katakana characters to their
 * full-width equivalents in a CP932 string.
 *
 * @param string Input strin
 * @return Output string with full-width characters.
 */
string hantozen_cp932(const std::string& string)
{
  // First convert the string to unicode so handling is easier.
  // (We can ignore any subsidiary transformation at this stage,
  // since RealLive always does.)
  wstring tmp = cp932toUnicode(string, 0);
  transform(tmp.begin(), tmp.end(), tmp.begin(), hantozen_wchar);
  throw rlvm::Exception("hantozen_cp932 unimplemented");
  //  return unicodetocp932(tmp);
}

// -----------------------------------------------------------------------
// All hankaku characters are <U+FF??>,
// while all zenkaku characters are <U+30??>
char zen2han_table[] = {
 0x9E,  0x00,  0x00,  0x00,  0x00,  0x00,  0x67,  0x71,
 0x68,  0x72,  0x69,  0x73,  0x6A,  0x74,  0x6B,  0x75,
 0x76,  0x00,  0x77,  0x00,  0x78,  0x00,  0x79,  0x00,
 0x7A,  0x00,  0x7B,  0x00,  0x7C,  0x00,  0x7D,  0x00,
 0x7E,  0x00,  0x7F,  0x00,  0x80,  0x00,  0x81,  0x00,
 0x6F,  0x82,  0x00,  0x83,  0x00,  0x84,  0x00,  0x85,
 0x86,  0x87,  0x88,  0x89,  0x8A,  0x00,  0x00,  0x8B,
 0x00,  0x00,  0x8C,  0x00,  0x00,  0x8D,  0x00,  0x00,
 0x8E,  0x00,  0x00,  0x8F,  0x90,  0x91,  0x92,  0x93,
 0x6C,  0x94,  0x6D,  0x95,  0x6E,  0x96,  0x97,  0x98,
 0x99,  0x9A,  0x9B,  0x00,  0x9C,  0x00,  0x00,  0x66,
 0x9D,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,
 0x65,  0x70
};

/**
 * Converts a single full-width unicode character to its half-width
 * equivalent.
 *
 * @param input Input character
 * @return Converted character (or input character if no translation
 * neccessary).
 */
wchar_t zentohan_wchar(wchar_t input)
{
  if(input >= 0xFF01 && input <= 0xFF5E)
  {
    input -= 0xFEE0;
  }
  else if(input == 0x3000)
  {
    input = 0x0020;
  }
  // Need to build a conversion by reading the cp932 ucm file, and then building
  // a mapping between "^KATAKANA X" and "^HALFWIDTH KATAKANA X"
  // d12443 - d12541
  else if(input >= 0x309B && input <= 0x30FC)
  {
    char han = zen2han_table[input - 0x309B];
    if(han)
      input = (0xFF << 8) | han;
  }

  return input;
}

// -----------------------------------------------------------------------

string zentohan_cp932(const std::string& string)
{
  wstring tmp = cp932toUnicode(string, 0);
  transform(tmp.begin(), tmp.end(), tmp.begin(), zentohan_wchar);
  throw rlvm::Exception("zentohan_cp932 unimplemented");
  //  return unicodetocp932(tmp);
}

// -----------------------------------------------------------------------

std::string unicodeToUTF8(const std::wstring& widestring)
{
  string out;
  utf8::utf16to8(widestring.begin(), widestring.end(),
                 back_inserter(out));

  return out;
}

// -----------------------------------------------------------------------

std::string cp932toUTF8(const std::string& line, int transformation)
{
  std::wstring ws = cp932toUnicode(line, transformation);
  return unicodeToUTF8(ws);
}

// -----------------------------------------------------------------------

bool isKinsoku(int codepoint)
{
  static const int matchingCodepoints[] =
    { 0x0021, 0x0022, 0x0027, 0x0029, 0x002c, 0x002e, 0x003a,
      0x003b, 0x003e, 0x003f, 0x005d, 0x007d, 0x2019, 0x201d,
      0x2025, 0x2026, 0x3001, 0x3002, 0x3009, 0x300b, 0x300d,
      0x300f, 0x3011, 0x301f, 0x3041, 0x3043, 0x3045, 0x3047,
      0x3049, 0x3063, 0x3083, 0x3085, 0x3087, 0x308e, 0x30a1,
      0x30a3, 0x30a5, 0x30a7, 0x30a9, 0x30c3, 0x30e3, 0x30e5,
      0x30e7, 0x30ee, 0x30f5, 0x30f6, 0x30fb, 0x30fc, 0xff01,
      0xff09, 0xff0c, 0xff0e, 0xff1a, 0xff1b, 0xff1f, 0xff3d,
      0xff5d, 0xff5e, 0xff61, 0xff63, 0xff64, 0xff65, 0xff67,
      0xff68, 0xff69, 0xff6a, 0xff6b, 0xff6c, 0xff6d, 0xff6e,
      0xff6f, 0xff70, 0xff9e, 0xff9f, 0x0 };

  for(int i = 0; matchingCodepoints[i] != 0x0; ++i)
    if(matchingCodepoints[i] == codepoint)
      return true;

  return false;
}

// -----------------------------------------------------------------------

int codepoint(const std::string& c)
{
  if(c == "")
    return 0;
  else
  {
    std::string::const_iterator it = c.begin();
    return utf8::next(it, c.end());
  }
}
