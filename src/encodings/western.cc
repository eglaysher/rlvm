/*
  rlBabel: Western language settings

  Copyright (c) 2006 Peter Jolly.

  This library is free software; you can redistribute it and/or modify it under
  the terms of the GNU Lesser General Public License as published by the Free
  Software Foundation; either version 2.1 of the License, or (at your option)
  any
  later version.

  This library is distributed in the hope that it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
  FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
  details.

  You should have received a copy of the GNU Lesser General Public License
  along with this library; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

  As a special exception to the GNU Lesser General Public License (LGPL), you
  may include a publicly distributed version of the library alongside a "work
  that uses the Library" to produce a composite work that includes the library,
  and distribute that work under terms of your choice, without any of the
  additional requirements listed in clause 6 of the LGPL.

  A "publicly distributed version of the library" means either an unmodified
  binary as distributed by Haeleth, or a modified version of the library that is
  distributed under the conditions defined in clause 2 of the LGPL, and a
  "composite work that includes the library" means a RealLive program which
  links to the library, either through the LoadDLL() interface or a #DLL
  directive, and/or includes code from the library's Kepago header.

  Note that this exception does not invalidate any other reasons why any part of
  the work might be covered by the LGPL.
*/

#include "encodings/western.h"

#include <cstdint>
#include <string>

bool Cp1252::IsItalic(uint16_t ch) const {
  return (ch > 0x8300 && ch < 0x8900);
}

uint16_t GetItalic(uint16_t ch) {
  if (ch > 0x8700)
    return ch + 0x0200;
  return ch - (ch >= 0x8380 ? 0x8320 : 0x831f);
}

uint16_t Italicise(uint16_t ch) {
  if (ch == 0x20 || ch > 0xff)
    return ch;
  if (ch > 0x8900)
    return ch - 0x0200;
  return ch + (ch >= 0x60 ? 0x8320 : 0x831f);
}

Cp1252::Cp1252() {
  //  DesirableCharset = ANSI_CHARSET;
  NoTransforms = false;
  //  LANGID SysLang = GetSystemDefaultLangID();
  // For now, just bless some common European languages. (Wait, shouldn't I be
  // checking system charset instead?)
  // UseUnicode = !(SysLang & 0x1ff == 0x07 ||
  //   (SysLang & 0x1ff >= 0x09 && SysLang & 0x1ff <= 0x0c));
}

bool Cp1252::DbcsDelim(char* str) const {
  return str[0] == 0x89 &&
         (str[1] == 0x82 || str[1] == 0x84 || str[1] == 0x91 || str[1] == 0x93);
}

uint16_t Cp1252::JisDecode(uint16_t ch) const {
  if (ch <= 0x7f)
    return ch;
  if (ch >= 0xa1 && ch <= 0xdf)
    return ch + 0x1f;
  if (ch >= 0x8980 && ch <= 0x89bf)
    return ch & 0xff;
  if (ch == 0x89c0)
    return 0xff;
  if (IsItalic(ch))
    return JisDecode(GetItalic(ch));
  return 0;
}

uint16_t JisEncode(uint16_t ch) {
  if (ch <= 0x7f)
    return ch;
  if (ch >= 0x80 && ch <= 0xbf)
    return ch | 0x8900;
  if (ch >= 0xc0 && ch <= 0xfe)
    return ch - 0x1f;
  if (ch == 0xff)
    return 0x89c0;
  return 0;
}

void Cp1252::JisEncodeString(const char* src, char* buf, size_t buflen) const {
  while (*src && buflen--) {
    unsigned int ch = JisEncode(*src);
    if (ch <= 0xff) {
      *buf++ = ch;
    } else {
      *buf++ = (ch >> 8) & 0xff;
      *buf++ = ch & 0xff;
    }
    ++src;
  }
}

const uint16_t cp1252_to_uni[0x9f - 0x80 + 1] = {
    0x20ac, 0xffff, 0x201a, 0x0192, 0x201e, 0x2026, 0x2020, 0x2021,
    0x02c6, 0x2030, 0x0160, 0x2039, 0x0152, 0xffff, 0x017d, 0xffff,
    0xffff, 0x2018, 0x2019, 0x201c, 0x201d, 0x2022, 0x2013, 0x2014,
    0x02dc, 0x2122, 0x0161, 0x203a, 0x0153, 0xffff, 0x017e, 0x0178};

uint16_t Cp1252::Convert(uint16_t ch) const {
  return ch >= 0x80 && ch <= 0x9f ? cp1252_to_uni[ch - 0x80] : ch;
}

std::wstring Cp1252::ConvertString(const std::string& in_string) const {
  std::wstring rv;
  rv.reserve(in_string.size());

  const char* s = in_string.c_str();
  while (*s)
    rv += Convert(*s++);

  return rv;
}
