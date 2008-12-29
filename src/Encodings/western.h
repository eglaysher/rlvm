/*
  rlBabel: Western language settings

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

  As a special exception to the GNU Lesser General Public License, you may
  include a publicly distributed version of the library in a "work that uses the
  Library" to produce a scenario file containing portions of the library, and
  distribute that scenario file under terms of your choice, without any of the
  additional requirements listed in clause 6 of the GNU Lesser General Public
  License.  A "publicly distributed version of the library" means either an
  unmodified library as distributed by Haeleth, or a modified version of the
  library that is distributed under the conditions defined in clause 2 of the
  GNU Lesser General Public License.  Note that this exception does not
  invalidate any other reasons why the scenario file might be covered by the
  GNU Lesser General Public License.
*/
  
#ifndef WESTERN_H

#include "codepage.h"

struct Cp1252 : public Codepage {
	USHORT JisDecode(USHORT ch) const;
	void JisEncodeString(LPCTSTR s, LPTSTR buf, size_t buflen) const;
	USHORT Convert(USHORT ch) const;
	wchar_t* ConvertString(const UCHAR* s) const;
	bool DbcsDelim(UCHAR* str) const;
	bool IsItalic(USHORT ch) const;
	Cp1252();
};

USHORT __stdcall GetItalic(USHORT ch);
USHORT __stdcall Italicise(USHORT ch);

#define WESTERN_H
#endif
