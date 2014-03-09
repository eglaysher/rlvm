// -*- Mode: C++; tab-width:2; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi:tw=80:et:ts=2:sts=2
//
// -----------------------------------------------------------------------
//
// This file is part of RLVM, a RealLive virtual machine clone.
//
// -----------------------------------------------------------------------
//
// Copyright (C) 2006, 2007 Elliot Glaysher
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

#include "Modules/Module_Str.hpp"

#include <algorithm>
#include <boost/lexical_cast.hpp>
#include <cmath>
#include <iomanip>
#include <sstream>
#include <string>

#include "encodings/codepage.h"
#include "encodings/han2zen.h"
#include "encodings/western.h"
#include "MachineBase/RLOperation.hpp"
#include "MachineBase/RLOperation/RLOp_Store.hpp"
#include "MachineBase/RLOperation/References.hpp"
#include "Systems/Base/System.hpp"
#include "Systems/Base/TextSystem.hpp"
#include "Systems/Base/TextPage.hpp"
#include "Utilities/Exception.hpp"
#include "Utilities/StringUtilities.hpp"

using namespace std;
using namespace boost;
using namespace libreallive;

namespace {

// Counts the number of Shift_JIS characters in a string.
size_t strcharlen(const char* string) {
  if (!string)
    return 0;
  size_t result = 0;
  while (*string) {
    ++result;
    advanceOneShiftJISChar(string);
  }
  return result;
}

// Changes the case of a character to uppercase. In some (most?)  versions of
// gcc, toupper is a macro, and thus can't be used in a transform.
inline char ToUpper(char x) { return toupper(x); }

// Changes the case of a character to lowercase. In some (most?)  versions of
// gcc, tolower is a macro, and thus can't be used in a transform.
inline char ToLower(char x) { return tolower(x); }

// Impelmentation for the ASCII versions of itoa. Sets |length| |fill|
// characters.
string rl_itoa_implementation(int number, int length, char fill) {
  ostringstream ss;
  if (number < 0)
    ss << "-";
  if (length > 0)
    ss << setw(length);
  ss << right << setfill(fill) << abs(number);
  return ss.str();
}

// Implement op<1:Str:00000, 0>, fun strcpy(str, strC).
//
// Assigns the string value val to the string variable dest.
struct strcpy_0 : public RLOp_Void_2<StrReference_T, StrConstant_T> {
  void operator()(RLMachine& machine,
                  StringReferenceIterator dest,
                  string val) {
    *dest = val;
  }
};

// Implement op<1:Str:00000, 1>, fun strcpy(str, strC, intC).
//
// Assigns the first count characters of val to the string variable dest.
struct strcpy_1
    : public RLOp_Void_3<StrReference_T, StrConstant_T, IntConstant_T> {
  void operator()(RLMachine& machine,
                  StringReferenceIterator dest,
                  string val,
                  int count) {
    *dest = val.substr(0, count);
  }
};

// Implement op<1:Str:00001, 0), fun strclear(str).
//
// Clears the string variable dest.
struct strclear_0 : public RLOp_Void_1<StrReference_T> {
  void operator()(RLMachine& machine, StringReferenceIterator dest) {
    *dest = "";
  }
};

// Implement op<1:Str:00001, 1), fun strclear(str 'first', str 'last').
//
// Clears all string variables in the inclusive range [first, last].
struct strclear_1 : public RLOp_Void_2<StrReference_T, StrReference_T> {
  void operator()(RLMachine& machine,
                  StringReferenceIterator first,
                  StringReferenceIterator last) {
    ++last;  // RL ranges are inclusive
    fill(first, last, "");
  }
};

// Implement op<1:Str:00002, 0>, fun strcat(str, strC). Concatenates
// the string into the memory location of the first.
struct Str_strcat : public RLOp_Void_2<StrReference_T, StrConstant_T> {
  void operator()(RLMachine& machine,
                  StringReferenceIterator it,
                  string append) {
    string s = *it;
    s += append;
    *it = s;
  }
};

// Implement op<1:Str:00003, 0>, fun strlen(strC). Returns the length
// of value; Double-byte characters are counted as two bytes.
struct Str_strlen : public RLOp_Store_1<StrConstant_T> {
  int operator()(RLMachine& machine, string value) { return value.size(); }
};

// Implement op<1:Str:00004, 0>, fun strcmp(strC, strC)
//
// Returns the standard strcmp() output of the two strings, ordering the
// strings in JIS X 0208.
//
// TODO(erg): THIS NEEDS TO HANDLE JSX ORDERING, NOT JUST ASCII!
struct Str_strcmp : public RLOp_Store_2<StrConstant_T, StrConstant_T> {
  int operator()(RLMachine& machine, string lhs, string rhs) {
    return strcmp(lhs.c_str(), rhs.c_str());
  }
};

// Implement op<1:Str:00005, 0>, fun strsub(str, strC, intC).
//
// Returns the substring, starting at offset.
struct strsub_0
    : public RLOp_Void_3<StrReference_T, StrConstant_T, IntConstant_T> {
  void operator()(RLMachine& machine,
                  StringReferenceIterator dest,
                  string source,
                  int offset) {
    const char* str = source.c_str();
    string output;

    // Advance the string to the first
    while (offset > 0) {
      if (str[0] == '\0') {
        throw rlvm::Exception(
            "Error in strsub: offset is greater then string length");
      }

      advanceOneShiftJISChar(str);
      offset--;
    }

    // Copy the rest of the string to the output buffer. We do not need to
    // worry about bytes vs. characters since we aren't worrying about the
    // number of characters.
    while (*str) {
      output += *str;
      str++;
    }

    *dest = output;
  }
};

// Implement op<1:Str:00005, 1>, fun strsub(str, strC, intC).
//
// Returns the substring of length length, starting at offset.
struct strsub_1 : public RLOp_Void_4<StrReference_T,
                                     StrConstant_T,
                                     IntConstant_T,
                                     IntConstant_T> {
  void operator()(RLMachine& machine,
                  StringReferenceIterator dest,
                  string source,
                  int offset,
                  int length) {
    const char* str = source.c_str();
    string output;

    // Advance the string to the first
    while (offset > 0) {
      if (*str == '\0') {
        throw rlvm::Exception(
            "Error in strsub: offset is greater then string length");
      }

      advanceOneShiftJISChar(str);
      offset--;
    }

    while (*str && length > 0) {
      copyOneShiftJisCharacter(str, output);
      length--;
    }

    *dest = output;
  }
};

// Implements op<1:Str:00006, 0>, fun strrsub(str, strC, intC).
struct strrsub_0 : public strsub_0 {
  void operator()(RLMachine& machine,
                  StringReferenceIterator dest,
                  string source,
                  int offsetFromBack) {
    int offset = strcharlen(source.c_str()) - offsetFromBack;
    return strsub_0::operator()(machine, dest, source, offset);
  }
};

// Implements op<1:Str:00006, 1>, fun strrsub(str, strC, intC, intC).
struct strrsub_1 : public strsub_1 {
  void operator()(RLMachine& machine,
                  StringReferenceIterator dest,
                  string source,
                  int offsetFromBack,
                  int length) {
    if (length > offsetFromBack) {
      throw rlvm::Exception(
          "strrsub: length of substring greater then offset in rsub");
    }

    int offset = strcharlen(source.c_str()) - offsetFromBack;
    return strsub_1::operator()(machine, dest, source, offset, length);
  }
};

// Implements op<1:Str:00007, 0>, fun strcharlen(strC). Returns the
// number of characters (as opposed to bytes) in a string. This
// function deals with Shift_JIS characters properly.
struct Str_strcharlen : public RLOp_Store_1<StrConstant_T> {
  int operator()(RLMachine& machine, string val) {
    return strcharlen(val.c_str());
  }
};

// Implements op<1:Str:00008, 0>, fun strtrunc(str, intC).
//
// Truncates dest such that its length does not exceed length characters.
struct Str_strtrunc : public RLOp_Void_2<StrReference_T, IntConstant_T> {
  void operator()(RLMachine& machine,
                  StringReferenceIterator dest,
                  int length) {
    string input = *dest;
    const char* str = input.c_str();
    string output;
    while (*str && length > 0) {
      copyOneShiftJisCharacter(str, output);
      --length;
    }
    *dest = output;
  }
};

// Implements op<1:Str:00010, 0>, fun hantozen(>str).
//
// Changes half width characters to their full width equivalents.
struct hantozen_0 : public RLOp_Void_1<StrReference_T> {
  void operator()(RLMachine& machine, StringReferenceIterator dest) {
    *dest = hantozen_cp932(*dest, machine.getTextEncoding());
  }
};

// Implements op<1:Str:00010, 1>, fun hantozen(strC, >str).
//
// Changes half width characters to their full width equivalents.
struct hantozen_1 : public RLOp_Void_2<StrConstant_T, StrReference_T> {
  void operator()(RLMachine& machine,
                  string input,
                  StringReferenceIterator dest) {
    *dest = hantozen_cp932(input, machine.getTextEncoding());
  }
};

// Implements op<1:Str:00011, 0>, fun zentohan(>str).
//
// Changes full width characters to their half width equivalents.
struct zentohan_0 : public RLOp_Void_1<StrReference_T> {
  void operator()(RLMachine& machine, StringReferenceIterator dest) {
    *dest = zentohan_cp932(*dest, machine.getTextEncoding());
  }
};

// Implements op<1:Str:00011, 1>, fun zentohan(strC, >str).
//
// Changes full width characters to their half width equivalents.
struct zentohan_1 : public RLOp_Void_2<StrConstant_T, StrReference_T> {
  void operator()(RLMachine& machine,
                  string input,
                  StringReferenceIterator dest) {
    *dest = zentohan_cp932(input, machine.getTextEncoding());
  }
};

// Implements op<1:Str:00012, 0>, fun Uppercase(str).
//
// Changes the case of all ASCII characters to UPPERCASE. This function does
// not affect full-width Shift_JIS characters.
struct Uppercase_0 : public RLOp_Void_1<StrReference_T> {
  void operator()(RLMachine& machine, StringReferenceIterator dest) {
    string input = *dest;
    transform(input.begin(), input.end(), input.begin(), ToUpper);
    *dest = input;
  }
};

// Implements op<1:Str:00012, 1>, fun Uppercase(strC, str).
//
// Changes the case of all ASCII characters to UPPERCASE. This function does
// not affect full-width Shift_JIS characters.
struct Uppercase_1 : public RLOp_Void_2<StrConstant_T, StrReference_T> {
  void operator()(RLMachine& machine,
                  string input,
                  StringReferenceIterator dest) {
    transform(input.begin(), input.end(), input.begin(), ToUpper);
    *dest = input;
  }
};

// Implements op<1:Str:00012, 0>, fun Lowercase(str).
//
// Changes the case of all ASCII characters to LOWERCASE. This function does
// not affect full-width Shift_JIS characters.
struct Lowercase_0 : public RLOp_Void_1<StrReference_T> {
  void operator()(RLMachine& machine, StringReferenceIterator dest) {
    string input = *dest;
    transform(input.begin(), input.end(), input.begin(), ToLower);
    *dest = input;
  }
};

// Implements op<1:Str:00012, 1>, fun Lowercase(strC, str).
//
// Changes the case of all ASCII characters to LOWERCASE. This function does
// not affect full-width Shift_JIS characters.
struct Lowercase_1 : public RLOp_Void_2<StrConstant_T, StrReference_T> {
  void operator()(RLMachine& machine,
                  string input,
                  StringReferenceIterator dest) {
    transform(input.begin(), input.end(), input.begin(), ToLower);
    *dest = input;
  }
};

// Implements op<1:Str:00014, 0>, fun itoa_ws(intC, str).
//
// Converts the integer value into a decimal representation. I don't understand
// how this function is any different from itoa_0, since we don't have a length
// parameter. See RLdev documentation on itoa_s.
struct itoa_ws_0 : public RLOp_Void_2<IntConstant_T, StrReference_T> {
  void operator()(RLMachine& machine, int input, StringReferenceIterator dest) {
    *dest = hantozen_cp932(rl_itoa_implementation(input, -1, ' '),
                           machine.getTextEncoding());
  }
};

// Implements op<1:Str:00014, 1>, fun itoa_ws(intC, str, intC).
//
// Converts the integer value into a decimal representation, right aligned with
// spaces to length characters.
struct itoa_ws_1
    : public RLOp_Void_3<IntConstant_T, StrReference_T, IntConstant_T> {
  void operator()(RLMachine& machine,
                  int input,
                  StringReferenceIterator dest,
                  int length) {
    *dest = hantozen_cp932(rl_itoa_implementation(input, length, ' '),
                           machine.getTextEncoding());
  }
};

// Implements op<1:Str:00015, 0>, fun itoa_s(intC, str).
//
// Converts the integer value into a decimal representation. I don't understand
// how this function is any different from itoa_0, since we don't have a length
// parameter. See RLdev documentation on itoa_s.
struct itoa_s_0 : public RLOp_Void_2<IntConstant_T, StrReference_T> {
  void operator()(RLMachine& machine, int input, StringReferenceIterator dest) {
    *dest = rl_itoa_implementation(input, -1, ' ');
  }
};

// Implements op<1:Str:00015, 1>, fun itoa_s(intC, str, intC).
//
// Converts the integer value into a decimal representation, right aligned with
// spaces to length characters.
struct itoa_s_1
    : public RLOp_Void_3<IntConstant_T, StrReference_T, IntConstant_T> {
  void operator()(RLMachine& machine,
                  int input,
                  StringReferenceIterator dest,
                  int length) {
    *dest = rl_itoa_implementation(input, length, ' ');
  }
};

// Implements op<1:Str:00016, 0>, fun itoa_w(intC, str).
//
// Converts the integer value into a decimal representation. I don't understand
// how this function is any different from itoa_0, since we don't have a length
// parameter. See RLdev documentation on itoa_s.
struct itoa_w_0 : public RLOp_Void_2<IntConstant_T, StrReference_T> {
  void operator()(RLMachine& machine, int input, StringReferenceIterator dest) {
    *dest = hantozen_cp932(rl_itoa_implementation(input, -1, '0'),
                           machine.getTextEncoding());
  }
};

// Implements op<1:Str:00016, 1>, fun itoa_w(intC, str, intC).
//
// Converts the integer value into a decimal representation, right aligned with
// zeroes to length characters.
struct itoa_w_1
    : public RLOp_Void_3<IntConstant_T, StrReference_T, IntConstant_T> {
  void operator()(RLMachine& machine,
                  int input,
                  StringReferenceIterator dest,
                  int length) {
    *dest = hantozen_cp932(rl_itoa_implementation(input, length, '0'),
                           machine.getTextEncoding());
  }
};

// Implements op<1:Str:00017, 0>, fun itoa(intC, str).
//
// Converts the integer value into a decimal representation. I don't understand
// how this function is any different from itoa_0, since we don't have a length
// parameter. See RLdev documentation on itoa_s.
struct itoa_0 : public RLOp_Void_2<IntConstant_T, StrReference_T> {
  void operator()(RLMachine& machine, int input, StringReferenceIterator dest) {
    *dest = rl_itoa_implementation(input, -1, '0');
  }
};

// Implements op<1:Str:00017, 1>, fun itoa_s(intC, str, intC).
//
// Converts the integer value into a decimal representation, right aligned with
// zeroes to length characters.
struct itoa_1
    : public RLOp_Void_3<IntConstant_T, StrReference_T, IntConstant_T> {
  void operator()(RLMachine& machine,
                  int input,
                  StringReferenceIterator dest,
                  int length) {
    *dest = rl_itoa_implementation(input, length, '0');
  }
};

// Implements op<1:Str:00018, 0>, fun atoi(strC).
//
// Returns the value of the integer represented by string, or 0 if string does
// not represent an integer. Leading whitespace is ignored, as is anything
// following the last decimal digit.
//
// I used to implement this as a call to boost::lexical_cast, until I started
// testing and I failed most of them because lexical_cast has different
// semantics about consuming *all* of the input string.
struct Str_atoi : public RLOp_Store_1<StrConstant_T> {
  int operator()(RLMachine& machine, string word) {
    stringstream ss(word);
    int out;
    ss >> out;
    if (ss)
      return out;
    else
      return 0;
  }
};

// Implements op<1:Str:00019, 0>, fun digits(intC).
//
// Returns the number of digits in the decimal representation of value. This is
// the length of the string that would be generated by an itoa() call with
// length set to 1, excluding any minus sign.
//
// ERG: What the heck is this used for!? This is a standard library function!?
// Haeleth: I've never seen it used...
struct Str_digits : public RLOp_Store_1<IntConstant_T> {
  int operator()(RLMachine& machine, int word) {
    string number = rl_itoa_implementation(abs(word), 1, '0');
    return number.size();
  }
};

// Implements op<1:Str:00020, 0>, fun digit(intC, int, intC).
//
// Sets the variable dest to equal the nth digit from the right in the decimal
// representation of value, and returns the total number of digits in the
// number.
//
// ERG: Who the hell thought this function was a good idea?
// Haeleth: It's used in `Princess Brave' to simplify displaying a number from
// a bitmap of digits.
struct Str_digit
    : public RLOp_Store_3<IntConstant_T, IntReference_T, IntConstant_T> {
  int operator()(RLMachine& machine,
                 int value,
                 IntReferenceIterator dest,
                 int index) {
    string number = rl_itoa_implementation(abs(value), 1, '0');
    *dest = number[number.size() - index] - '0';
    return number.size();
  }
};

// Implements op<1:Str:00030, 0>, fun strpos(strC, strC).
//
// Returns the offset of the first instance of substring in str, or -1 if
// substring is not found.
struct Str_strpos : public RLOp_Store_2<StrConstant_T, StrConstant_T> {
  int operator()(RLMachine& machine, string str, string substring) {
    size_t pos = str.find(substring);
    if (pos == string::npos)
      return -1;
    else
      return pos;
  }
};

// Implements op<1:Str:00031, 0>, fun strpos(strC, strC).
//
// As strpos, but returns the offset of the last instance of substring. If
// substring appears only once, or not at all, in string, the behaviour is
// identical with that of strpos.
struct Str_strlpos : public RLOp_Store_2<StrConstant_T, StrConstant_T> {
  int operator()(RLMachine& machine, string str, string substring) {
    size_t pos = str.rfind(substring);
    if (pos == string::npos)
      return -1;
    else
      return pos;
  }
};

// Implement op<1:Str:00100, 0>, fun strout(strV 'val').
//
// Prints a string.
struct Str_strout : public RLOp_Void_1<StrConstant_T> {
  void operator()(RLMachine& machine, string value) {
    // We collaborate with rlBabel here.
    //
    // This is the point right before we are about to switch from cp932 to
    // unicode. If the character is supposed to be italic, the incoming values
    // may have been munged to be valid cp932 character.
    int encoding = machine.getTextEncoding();
    size_t size = value.size();
    if (encoding != 0 && (size == 1 || size == 2)) {
      // Look at the first character in the
      uint16_t cp932_char = value[0];
      if (size == 2 && shiftjis_lead_byte(cp932_char))
        cp932_char = (cp932_char << 8) | value[1];

      if (Cp::instance(encoding).IsItalic(cp932_char)) {
        // We must make take this character and turn it into its unitalicized
        // form.
        uint16_t decoded = GetItalic(cp932_char);
        value.clear();
        addShiftJISChar(decoded, value);

        // Notify the TextSystem that the next character that will be printed
        // should be printed in italics.
        TextPage& page = machine.system().text().currentPage();
        page.NextCharIsItalic();
      }
    }

    machine.performTextout(value);
  }
};

// Implement op<1:Str:00100, 1>, fun intout(intV 'val').
//
// Prints an integer.
struct Str_intout : public RLOp_Void_1<IntConstant_T> {
  void operator()(RLMachine& machine, int value) {
    // Assumption: Text is in whatever native encoding for getTextEncoding().
    string converted = lexical_cast<string>(value);
    machine.performTextout(converted);
  }
};

// Implement op<1:Str:00200, 1>, fun strused(str).
//
// Returns 0 if the string variable var is empty, otherwise 1.
struct Str_strused : public RLOp_Store_1<StrReference_T> {
  int operator()(RLMachine& machine, StringReferenceIterator it) {
    return string(*it) != "";
  }
};

}  // namespace

StrModule::StrModule() : RLModule("Str", 1, 10) {
  addOpcode(0, 0, "strcpy", new strcpy_0);
  addOpcode(0, 1, "strcpy", new strcpy_1);
  addOpcode(1, 0, "strclear", new strclear_0);
  addOpcode(1, 1, "strclear", new strclear_1);
  addOpcode(2, 0, "strcat", new Str_strcat);
  addOpcode(3, 0, "strlen", new Str_strlen);
  addOpcode(4, 0, "strcmp", new Str_strcmp);
  addOpcode(5, 0, "strsub", new strsub_0);
  addOpcode(5, 1, "strsub", new strsub_1);
  addOpcode(6, 0, "strrsub", new strrsub_0);
  addOpcode(6, 1, "strrsub", new strrsub_1);
  addOpcode(7, 0, "strcharlen", new Str_strcharlen);
  addOpcode(8, 0, "strtrunc", new Str_strtrunc);
  addOpcode(10, 0, "hantozen", new hantozen_0);
  addOpcode(10, 1, "hantozen", new hantozen_1);
  addOpcode(11, 0, "zentohan", new zentohan_0);
  addOpcode(11, 1, "zentohan", new zentohan_1);
  addOpcode(12, 0, "Uppercase", new Uppercase_0);
  addOpcode(12, 1, "Uppercase", new Uppercase_1);
  addOpcode(13, 0, "Lowercase", new Lowercase_0);
  addOpcode(13, 1, "Lowercase", new Lowercase_1);
  addOpcode(14, 0, "itoa_ws", new itoa_ws_0);
  addOpcode(14, 1, "itoa_ws", new itoa_ws_1);
  addOpcode(15, 0, "itoa_s", new itoa_s_0);
  addOpcode(15, 1, "itoa_s", new itoa_s_1);
  addOpcode(16, 0, "itoa_w", new itoa_w_0);
  addOpcode(16, 1, "itoa_w", new itoa_w_1);
  addOpcode(17, 0, "itoa", new itoa_0);
  addOpcode(17, 1, "itoa", new itoa_1);
  addOpcode(18, 0, "atoi", new Str_atoi);
  addOpcode(19, 0, "digits", new Str_digits);
  addOpcode(20, 0, "digit", new Str_digit);
  addOpcode(30, 0, "strpos", new Str_strpos);
  addOpcode(31, 0, "strlpos", new Str_strlpos);

  addOpcode(100, 0, "strout", new Str_strout);
  addOpcode(100, 1, "intout", new Str_intout);
  addOpcode(200, 0, "strused", new Str_strused);
}
