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
#include "Modules/cp932toUnicode.hpp"

#include <string>
#include <iostream>
#include <stdexcept>

#include "utf8.h"
#include "Systems/Base/SystemError.hpp"

#include "Encodings/codepage.h"
#include "Utilities/StringUtilities.hpp"

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

unsigned short lower_hantozen_table[] = {
  0x8140, // (32) 'SPACE' => 'IDEOGRAPH SPACE'
  0x8149, // (33) 'EXCLAMATION MARK' => 'FULLWIDTH EXCLAMATION MARK'
  0xeefc, // (34) 'QUOTATION MARK' => 'FULLWIDTH QUOTATION MARK'
  0x8194, // (35) 'NUMBER SIGN' => 'FULLWIDTH NUMBER SIGN'
  0x8190, // (36) 'DOLLAR SIGN' => 'FULLWIDTH DOLLAR SIGN'
  0x8193, // (37) 'PERCENT SIGN' => 'FULLWIDTH PERCENT SIGN'
  0x8195, // (38) 'AMPERSAND' => 'FULLWIDTH AMPERSAND'
  0xeefb, // (39) 'APOSTROPHE' => 'FULLWIDTH APOSTROPHE'
  0x8169, // (40) 'LEFT PARENTHESIS' => 'FULLWIDTH LEFT PARENTHESIS'
  0x816a, // (41) 'RIGHT PARENTHESIS' => 'FULLWIDTH RIGHT PARENTHESIS'
  0x8196, // (42) 'ASTERISK' => 'FULLWIDTH ASTERISK'
  0x817b, // (43) 'PLUS SIGN' => 'FULLWIDTH PLUS SIGN'
  0x8143, // (44) 'COMMA' => 'FULLWIDTH COMMA'
  0x817c, // (45) 'HYPHEN-MINUS' => 'FULLWIDTH HYPHEN-MINUS'
  0x8144, // (46) 'FULL STOP' => 'FULLWIDTH FULL STOP'
  0x815e, // (47) 'SOLIDUS' => 'FULLWIDTH SOLIDUS'
  0x824f, // (48) 'DIGIT ZERO' => 'FULLWIDTH DIGIT ZERO'
  0x8250, // (49) 'DIGIT ONE' => 'FULLWIDTH DIGIT ONE'
  0x8251, // (50) 'DIGIT TWO' => 'FULLWIDTH DIGIT TWO'
  0x8252, // (51) 'DIGIT THREE' => 'FULLWIDTH DIGIT THREE'
  0x8253, // (52) 'DIGIT FOUR' => 'FULLWIDTH DIGIT FOUR'
  0x8254, // (53) 'DIGIT FIVE' => 'FULLWIDTH DIGIT FIVE'
  0x8255, // (54) 'DIGIT SIX' => 'FULLWIDTH DIGIT SIX'
  0x8256, // (55) 'DIGIT SEVEN' => 'FULLWIDTH DIGIT SEVEN'
  0x8257, // (56) 'DIGIT EIGHT' => 'FULLWIDTH DIGIT EIGHT'
  0x8258, // (57) 'DIGIT NINE' => 'FULLWIDTH DIGIT NINE'
  0x8146, // (58) 'COLON' => 'FULLWIDTH COLON'
  0x8147, // (59) 'SEMICOLON' => 'FULLWIDTH SEMICOLON'
  0x8183, // (60) 'LESS-THAN SIGN' => 'FULLWIDTH LESS-THAN SIGN'
  0x8181, // (61) 'EQUALS SIGN' => 'FULLWIDTH EQUALS SIGN'
  0x8184, // (62) 'GREATER-THAN SIGN' => 'FULLWIDTH GREATER-THAN SIGN'
  0x8148, // (63) 'QUESTION MARK' => 'FULLWIDTH QUESTION MARK'
  0x8197, // (64) 'COMMERCIAL AT' => 'FULLWIDTH COMMERCIAL AT'
  0x8260, // (65) 'LATIN CAPITAL LETTER A' => 'FULLWIDTH LATIN CAPITAL LETTER A'
  0x8261, // (66) 'LATIN CAPITAL LETTER B' => 'FULLWIDTH LATIN CAPITAL LETTER B'
  0x8262, // (67) 'LATIN CAPITAL LETTER C' => 'FULLWIDTH LATIN CAPITAL LETTER C'
  0x8263, // (68) 'LATIN CAPITAL LETTER D' => 'FULLWIDTH LATIN CAPITAL LETTER D'
  0x8264, // (69) 'LATIN CAPITAL LETTER E' => 'FULLWIDTH LATIN CAPITAL LETTER E'
  0x8265, // (70) 'LATIN CAPITAL LETTER F' => 'FULLWIDTH LATIN CAPITAL LETTER F'
  0x8266, // (71) 'LATIN CAPITAL LETTER G' => 'FULLWIDTH LATIN CAPITAL LETTER G'
  0x8267, // (72) 'LATIN CAPITAL LETTER H' => 'FULLWIDTH LATIN CAPITAL LETTER H'
  0x8268, // (73) 'LATIN CAPITAL LETTER I' => 'FULLWIDTH LATIN CAPITAL LETTER I'
  0x8269, // (74) 'LATIN CAPITAL LETTER J' => 'FULLWIDTH LATIN CAPITAL LETTER J'
  0x826a, // (75) 'LATIN CAPITAL LETTER K' => 'FULLWIDTH LATIN CAPITAL LETTER K'
  0x826b, // (76) 'LATIN CAPITAL LETTER L' => 'FULLWIDTH LATIN CAPITAL LETTER L'
  0x826c, // (77) 'LATIN CAPITAL LETTER M' => 'FULLWIDTH LATIN CAPITAL LETTER M'
  0x826d, // (78) 'LATIN CAPITAL LETTER N' => 'FULLWIDTH LATIN CAPITAL LETTER N'
  0x826e, // (79) 'LATIN CAPITAL LETTER O' => 'FULLWIDTH LATIN CAPITAL LETTER O'
  0x826f, // (80) 'LATIN CAPITAL LETTER P' => 'FULLWIDTH LATIN CAPITAL LETTER P'
  0x8270, // (81) 'LATIN CAPITAL LETTER Q' => 'FULLWIDTH LATIN CAPITAL LETTER Q'
  0x8271, // (82) 'LATIN CAPITAL LETTER R' => 'FULLWIDTH LATIN CAPITAL LETTER R'
  0x8272, // (83) 'LATIN CAPITAL LETTER S' => 'FULLWIDTH LATIN CAPITAL LETTER S'
  0x8273, // (84) 'LATIN CAPITAL LETTER T' => 'FULLWIDTH LATIN CAPITAL LETTER T'
  0x8274, // (85) 'LATIN CAPITAL LETTER U' => 'FULLWIDTH LATIN CAPITAL LETTER U'
  0x8275, // (86) 'LATIN CAPITAL LETTER V' => 'FULLWIDTH LATIN CAPITAL LETTER V'
  0x8276, // (87) 'LATIN CAPITAL LETTER W' => 'FULLWIDTH LATIN CAPITAL LETTER W'
  0x8277, // (88) 'LATIN CAPITAL LETTER X' => 'FULLWIDTH LATIN CAPITAL LETTER X'
  0x8278, // (89) 'LATIN CAPITAL LETTER Y' => 'FULLWIDTH LATIN CAPITAL LETTER Y'
  0x8279, // (90) 'LATIN CAPITAL LETTER Z' => 'FULLWIDTH LATIN CAPITAL LETTER Z'
  0x816d, // (91) 'LEFT SQUARE BRACKET' => 'FULLWIDTH LEFT SQUARE BRACKET'
  0x815f, // (92) 'REVERSE SOLIDUS' => 'FULLWIDTH REVERSE SOLIDUS'
  0x816e, // (93) 'RIGHT SQUARE BRACKET' => 'FULLWIDTH RIGHT SQUARE BRACKET'
  0x814f, // (94) 'CIRCUMFLEX ACCENT' => 'FULLWIDTH CIRCUMFLEX ACCENT'
  0x8151, // (95) 'LOW LINE' => 'FULLWIDTH LOW LINE'
  0x814d, // (96) 'GRAVE ACCENT' => 'FULLWIDTH GRAVE ACCENT'
  0x8281, // (97) 'LATIN SMALL LETTER A' => 'FULLWIDTH LATIN SMALL LETTER A'
  0x8282, // (98) 'LATIN SMALL LETTER B' => 'FULLWIDTH LATIN SMALL LETTER B'
  0x8283, // (99) 'LATIN SMALL LETTER C' => 'FULLWIDTH LATIN SMALL LETTER C'
  0x8284, // (100) 'LATIN SMALL LETTER D' => 'FULLWIDTH LATIN SMALL LETTER D'
  0x8285, // (101) 'LATIN SMALL LETTER E' => 'FULLWIDTH LATIN SMALL LETTER E'
  0x8286, // (102) 'LATIN SMALL LETTER F' => 'FULLWIDTH LATIN SMALL LETTER F'
  0x8287, // (103) 'LATIN SMALL LETTER G' => 'FULLWIDTH LATIN SMALL LETTER G'
  0x8288, // (104) 'LATIN SMALL LETTER H' => 'FULLWIDTH LATIN SMALL LETTER H'
  0x8289, // (105) 'LATIN SMALL LETTER I' => 'FULLWIDTH LATIN SMALL LETTER I'
  0x828a, // (106) 'LATIN SMALL LETTER J' => 'FULLWIDTH LATIN SMALL LETTER J'
  0x828b, // (107) 'LATIN SMALL LETTER K' => 'FULLWIDTH LATIN SMALL LETTER K'
  0x828c, // (108) 'LATIN SMALL LETTER L' => 'FULLWIDTH LATIN SMALL LETTER L'
  0x828d, // (109) 'LATIN SMALL LETTER M' => 'FULLWIDTH LATIN SMALL LETTER M'
  0x828e, // (110) 'LATIN SMALL LETTER N' => 'FULLWIDTH LATIN SMALL LETTER N'
  0x828f, // (111) 'LATIN SMALL LETTER O' => 'FULLWIDTH LATIN SMALL LETTER O'
  0x8290, // (112) 'LATIN SMALL LETTER P' => 'FULLWIDTH LATIN SMALL LETTER P'
  0x8291, // (113) 'LATIN SMALL LETTER Q' => 'FULLWIDTH LATIN SMALL LETTER Q'
  0x8292, // (114) 'LATIN SMALL LETTER R' => 'FULLWIDTH LATIN SMALL LETTER R'
  0x8293, // (115) 'LATIN SMALL LETTER S' => 'FULLWIDTH LATIN SMALL LETTER S'
  0x8294, // (116) 'LATIN SMALL LETTER T' => 'FULLWIDTH LATIN SMALL LETTER T'
  0x8295, // (117) 'LATIN SMALL LETTER U' => 'FULLWIDTH LATIN SMALL LETTER U'
  0x8296, // (118) 'LATIN SMALL LETTER V' => 'FULLWIDTH LATIN SMALL LETTER V'
  0x8297, // (119) 'LATIN SMALL LETTER W' => 'FULLWIDTH LATIN SMALL LETTER W'
  0x8298, // (120) 'LATIN SMALL LETTER X' => 'FULLWIDTH LATIN SMALL LETTER X'
  0x8299, // (121) 'LATIN SMALL LETTER Y' => 'FULLWIDTH LATIN SMALL LETTER Y'
  0x829a, // (122) 'LATIN SMALL LETTER Z' => 'FULLWIDTH LATIN SMALL LETTER Z'
  0x816f, // (123) 'LEFT CURLY BRACKET' => 'FULLWIDTH LEFT CURLY BRACKET'
  0x8162, // (124) 'VERTICAL LINE' => 'FULLWIDTH VERTICAL LINE'
  0x8170, // (125) 'RIGHT CURLY BRACKET' => 'FULLWIDTH RIGHT CURLY BRACKET'
  0x8160, // (126) 'TILDE' => 'FULLWIDTH TILDE'
};

unsigned short upper_hantozen_table[] = {
  0x8142, // 'HALFWIDTH IDEOGRAPHIC FULL STOP' => 'IDEOGRAPHIC FULL STOP'
  0x8175, // 'HALFWIDTH LEFT CORNER BRACKET' => 'LEFT CORNER BRACKET'
  0x8176, // 'HALFWIDTH RIGHT CORNER BRACKET' => 'RIGHT CORNER BRACKET'
  0x8141, // 'HALFWIDTH IDEOGRAPHIC COMMA' => 'IDEOGRAPHIC COMMA'
  0x8145, // 'HALFWIDTH KATAKANA MIDDLE DOT' => 'KATAKANA MIDDLE DOT'
  0x8392, // 'HALFWIDTH KATAKANA LETTER WO' => 'KATAKANA LETTER WO'
  0x8340, // 'HALFWIDTH KATAKANA LETTER SMALL A' => 'KATAKANA LETTER SMALL A'
  0x8342, // 'HALFWIDTH KATAKANA LETTER SMALL I' => 'KATAKANA LETTER SMALL I'
  0x8344, // 'HALFWIDTH KATAKANA LETTER SMALL U' => 'KATAKANA LETTER SMALL U'
  0x8346, // 'HALFWIDTH KATAKANA LETTER SMALL E' => 'KATAKANA LETTER SMALL E'
  0x8348, // 'HALFWIDTH KATAKANA LETTER SMALL O' => 'KATAKANA LETTER SMALL O'
  0x8383, // 'HALFWIDTH KATAKANA LETTER SMALL YA' => 'KATAKANA LETTER SMALL YA'
  0x8385, // 'HALFWIDTH KATAKANA LETTER SMALL YU' => 'KATAKANA LETTER SMALL YU'
  0x8387, // 'HALFWIDTH KATAKANA LETTER SMALL YO' => 'KATAKANA LETTER SMALL YO'
  0x8362, // 'HALFWIDTH KATAKANA LETTER SMALL TU' => 'KATAKANA LETTER SMALL TU'
  0x815b, // 'HALFWIDTH KATAKANA-HIRAGANA PROLONGED SOUND MARK' => 'KATAKANA-HIRAGANA PROLONGED SOUND MARK'
  0x8341, // 'HALFWIDTH KATAKANA LETTER A' => 'KATAKANA LETTER A'
  0x8343, // 'HALFWIDTH KATAKANA LETTER I' => 'KATAKANA LETTER I'
  0x8345, // 'HALFWIDTH KATAKANA LETTER U' => 'KATAKANA LETTER U'
  0x8347, // 'HALFWIDTH KATAKANA LETTER E' => 'KATAKANA LETTER E'
  0x8349, // 'HALFWIDTH KATAKANA LETTER O' => 'KATAKANA LETTER O'
  0x834a, // 'HALFWIDTH KATAKANA LETTER KA' => 'KATAKANA LETTER KA'
  0x834c, // 'HALFWIDTH KATAKANA LETTER KI' => 'KATAKANA LETTER KI'
  0x834e, // 'HALFWIDTH KATAKANA LETTER KU' => 'KATAKANA LETTER KU'
  0x8350, // 'HALFWIDTH KATAKANA LETTER KE' => 'KATAKANA LETTER KE'
  0x8352, // 'HALFWIDTH KATAKANA LETTER KO' => 'KATAKANA LETTER KO'
  0x8354, // 'HALFWIDTH KATAKANA LETTER SA' => 'KATAKANA LETTER SA'
  0x8356, // 'HALFWIDTH KATAKANA LETTER SI' => 'KATAKANA LETTER SI'
  0x8358, // 'HALFWIDTH KATAKANA LETTER SU' => 'KATAKANA LETTER SU'
  0x835a, // 'HALFWIDTH KATAKANA LETTER SE' => 'KATAKANA LETTER SE'
  0x835c, // 'HALFWIDTH KATAKANA LETTER SO' => 'KATAKANA LETTER SO'
  0x835e, // 'HALFWIDTH KATAKANA LETTER TA' => 'KATAKANA LETTER TA'
  0x8360, // 'HALFWIDTH KATAKANA LETTER TI' => 'KATAKANA LETTER TI'
  0x8363, // 'HALFWIDTH KATAKANA LETTER TU' => 'KATAKANA LETTER TU'
  0x8365, // 'HALFWIDTH KATAKANA LETTER TE' => 'KATAKANA LETTER TE'
  0x8367, // 'HALFWIDTH KATAKANA LETTER TO' => 'KATAKANA LETTER TO'
  0x8369, // 'HALFWIDTH KATAKANA LETTER NA' => 'KATAKANA LETTER NA'
  0x836a, // 'HALFWIDTH KATAKANA LETTER NI' => 'KATAKANA LETTER NI'
  0x836b, // 'HALFWIDTH KATAKANA LETTER NU' => 'KATAKANA LETTER NU'
  0x836c, // 'HALFWIDTH KATAKANA LETTER NE' => 'KATAKANA LETTER NE'
  0x836d, // 'HALFWIDTH KATAKANA LETTER NO' => 'KATAKANA LETTER NO'
  0x836e, // 'HALFWIDTH KATAKANA LETTER HA' => 'KATAKANA LETTER HA'
  0x8371, // 'HALFWIDTH KATAKANA LETTER HI' => 'KATAKANA LETTER HI'
  0x8374, // 'HALFWIDTH KATAKANA LETTER HU' => 'KATAKANA LETTER HU'
  0x8377, // 'HALFWIDTH KATAKANA LETTER HE' => 'KATAKANA LETTER HE'
  0x837a, // 'HALFWIDTH KATAKANA LETTER HO' => 'KATAKANA LETTER HO'
  0x837d, // 'HALFWIDTH KATAKANA LETTER MA' => 'KATAKANA LETTER MA'
  0x837e, // 'HALFWIDTH KATAKANA LETTER MI' => 'KATAKANA LETTER MI'
  0x8380, // 'HALFWIDTH KATAKANA LETTER MU' => 'KATAKANA LETTER MU'
  0x8381, // 'HALFWIDTH KATAKANA LETTER ME' => 'KATAKANA LETTER ME'
  0x8382, // 'HALFWIDTH KATAKANA LETTER MO' => 'KATAKANA LETTER MO'
  0x8384, // 'HALFWIDTH KATAKANA LETTER YA' => 'KATAKANA LETTER YA'
  0x8386, // 'HALFWIDTH KATAKANA LETTER YU' => 'KATAKANA LETTER YU'
  0x8388, // 'HALFWIDTH KATAKANA LETTER YO' => 'KATAKANA LETTER YO'
  0x8389, // 'HALFWIDTH KATAKANA LETTER RA' => 'KATAKANA LETTER RA'
  0x838a, // 'HALFWIDTH KATAKANA LETTER RI' => 'KATAKANA LETTER RI'
  0x838b, // 'HALFWIDTH KATAKANA LETTER RU' => 'KATAKANA LETTER RU'
  0x838c, // 'HALFWIDTH KATAKANA LETTER RE' => 'KATAKANA LETTER RE'
  0x838d, // 'HALFWIDTH KATAKANA LETTER RO' => 'KATAKANA LETTER RO'
  0x838f, // 'HALFWIDTH KATAKANA LETTER WA' => 'KATAKANA LETTER WA'
  0x8393, // 'HALFWIDTH KATAKANA LETTER N' => 'KATAKANA LETTER N'
  0x814A, // 'HALFWIDTH KATAKANA VOICED SOUND MARK' => 'KATAKANA-HIRAGANA VOICED SOUND MARK'
  0x814B  // 'HALFWIDTH KATAKANA SEMI-VOICED SOUND MARK' => 'KATAKANA-HIRAGANA SEMI-VOICED SOUND MARK'
};

/**
 * Converts half-width ASCII and katakana characters to their
 * full-width equivalents in a CP932 string.
 *
 * @param string Input strin
 * @return Output string with full-width characters.
 */
string hantozen_cp932(const std::string& input, int transformation)
{
  // hantozen only makes sense in the context of Cp932
  if (transformation == 0) {
    string output;
    const char* s = input.c_str();
    const char* end = input.c_str() + input.size();

    while (s < end) {
      if (shiftjis_lead_byte(*s)) {
        copyOneShiftJisCharacter(s, output);
      } else {
        char c = *s;
        if (c >= ' ' && c <= '~') {
          // Lower ASCII conversion table
          addShiftJISChar(lower_hantozen_table[c - ' '], output);
        } else if (c >= 0xA1 && c <= 0xDF) {
          addShiftJISChar(upper_hantozen_table[c - 0xA1], output);
        } else {
          output += c;
        }
        s++;
      }
    }

    return output;
  }

  return input;
}

// -----------------------------------------------------------------------

// Helper function used in zentohan_cp932.
static int findCharInTable(const char* str,
                           unsigned short upper_hantozen_table[],
                           char start, char end) {
  unsigned short c = (str[0] << 8) | str[1];

  int max = end - start;
  for (int i = 0; i < max; ++i) {
    if (upper_hantozen_table[i] == c)
      return start + i;
  }

  return 0;
}

// -----------------------------------------------------------------------

string zentohan_cp932(const std::string& input, int transformation)
{
  string output;

  // hantozen only makes sense in the context of Cp932
  if (transformation == 0) {
    // This is a really bad, naive implementation of zentohan. The idea is that
    // we iterate over both tables used in hantozen above, and if we see a
    // target code, we deduce the ASCII.
    //
    // This algorithm is O(n), but the sizeof the tables is a pretty huge
    // coefficient.
    const char* s = input.c_str();
    const char* end = input.c_str() + input.size();

    while (s < end) {
      if (shiftjis_lead_byte(*s)) {
        int han = findCharInTable(s, lower_hantozen_table, ' ', '~');
        if (han == 0)
          han = findCharInTable(s, upper_hantozen_table, 0xA1, 0xDF);

        if (han) {
          output += char(han);
          s += 2;
        } else {
          // Not an anything.
          copyOneShiftJisCharacter(s, output);
        }
      } else {
        copyOneShiftJisCharacter(s, output);
      }
    }

    return output;
  }

  return input;
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
