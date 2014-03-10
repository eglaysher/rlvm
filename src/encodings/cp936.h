/*
  rlBabel: Simplified Chinese (CP936 / GBK) settings

  Copyright (c) 2006 Peter Jolly.

  This library is free software; you can redistribute it and/or modify it under
  the terms of the GNU Lesser General Public License as published by the Free
  Software Foundation; either version 2.1 of the License, or (at your option) any
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

#ifndef SRC_ENCODINGS_CP936_H_
#define SRC_ENCODINGS_CP936_H_

#include <string>

#include "encodings/codepage.h"

struct Cp936 : public Codepage {
  unsigned short JisDecode(unsigned short ch) const;
  void JisEncodeString(const char* s, char* buf, size_t buflen) const;
  unsigned short Convert(unsigned short ch) const;
  std::wstring ConvertString(const std::string& s) const;
  Cp936();
};

#endif  // SRC_ENCODINGS_CP936_H_
