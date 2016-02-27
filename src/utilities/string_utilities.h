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
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
//
// -----------------------------------------------------------------------

#ifndef SRC_UTILITIES_STRING_UTILITIES_H_
#define SRC_UTILITIES_STRING_UTILITIES_H_

#include <cstdint>
#include <functional>
#include <string>

// Converts a CP932/Shift_JIS string into a wstring with Unicode
// characters.
//
// If the string was not CP932, but actually another encoding
// transformed such that it can be processed as CP932, this additional
// transformation should be reversed here.  This technique is how
// non-Japanese text is used in RealLive.  Transformations that might
// potentially be encountered are:
//
//   0 - plain CP932 (no transformation)
//   1 - CP936
//   2 - CP1252 (also requires rlBabel support)
//   3 - CP949
//
// These are the transformations applied by RLdev, and translation
// tables can be found in the rlBabel source code.  There are also at
// least two CP936 transformations used by the Key Fans Club and
// possibly one or more other CP949 transformations, but details of
// these are not publicly available.
std::wstring cp932toUnicode(const std::string& line, int transformation);

// String representation of the transformation name.
std::string TransformationName(int transformation);

// Converts a UTF-16 string to a UTF-8 one.
std::string UnicodeToUTF8(const std::wstring& widestring);

// For convenience. Combines the two above functions.
std::string cp932toUTF8(const std::string& line, int transformation);

// Returns true if codepoint is either of the Japanese quote marks or '('.
bool IsOpeningQuoteMark(int codepoint);

// Returns whether |codepoint| is part of a word that should be wrapped if we
// go over the right margin.
bool IsWrappingRomanCharacter(int codepoint);

// Returns whether the unicode |codepoint| is a piece of breaking punctuation.
bool IsKinsoku(int codepoint);

// Returns the unicode codpoint for the next UTF-8 character in |c|.
int Codepoint(const std::string& c);

// Checks to see if the byte c is the first byte of a two byte
// character. RealLive encodes its strings in Shift_JIS, so we have to
// deal with this character encoding.
inline bool shiftjis_lead_byte(const char c) {
  return (c >= 0x81 && c <= 0x9f) || (c >= 0xe0 && c <= 0xfc);
}

// Advanced the Shift_JIS character string c by one char.
void AdvanceOneShiftJISChar(const char*& c);

// Copies a single Shift_JIS character into output and advances the string.
void CopyOneShiftJisCharacter(const char*& str, std::string& output);

// If there is currently a two byte fullwidth Latin capital letter at the
// current point in str (a shift_jis string), then convert it to ASCII, copy it
// to output and return true. Returns false otherwise.
bool ReadFullwidthLatinLetter(const char*& str, std::string& output);

// Adds a character to the string |output|. Two byte characters are encoded in
// an uint16_t.
void AddShiftJISChar(uint16_t c, std::string& output);

// Feeds each two consecutive pair of characters to |fun|.
void PrintTextToFunction(
    std::function<bool(const std::string& c, const std::string& nextChar)> fun,
    const std::string& charsToPrint,
    const std::string& nextCharForFinal);

// Removes quotes from the beginning and end of the string.
std::string RemoveQuotes(const std::string& quotedString);

#endif  // SRC_UTILITIES_STRING_UTILITIES_H_
