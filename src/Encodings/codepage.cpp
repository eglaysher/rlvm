/*
  rlBabel: abstract base class for codepage definitions

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
  
#include "codepage.h"

Codepage::~Codepage()
{
	// empty virtual destructor
}

USHORT Codepage::JisDecode(USHORT ch) const
{
	return ch;
}

void Codepage::JisEncodeString(LPCTSTR src, LPTSTR buf, size_t buflen) const
{
	strncpy((char*) buf, (const char*) src, buflen);
}

void Codepage::JisDecodeString(LPCTSTR src, LPTSTR buf, size_t buflen) const
{
	int srclen = strlen(src), i = 0, j = 0;
	while (i < srclen && j < buflen) {
		UINT c1 = (unsigned char) src[i++];
		if ((c1 >= 0x81 && c1 < 0xa0) || (c1 >= 0xe0 && c1 < 0xf0))
			c1 = (c1 << 8) | (unsigned char) src[i++];
		UINT c2 = JisDecode(c1);
		if (c2 <= 0xff)
			buf[j++] = c2;
		else {
			buf[j++] = (c2 >> 8) & 0xff;
			buf[j++] = c2 & 0xff;
		}
	}
	buf[j] = 0;
}

USHORT Codepage::Convert(USHORT ch) const
{
	return ch;
}

bool Codepage::DbcsDelim(UCHAR* str) const
{
	return false;
}

bool Codepage::IsItalic(USHORT ch) const
{
	return false;
}


// And the codepage interface, Cp.

#include "interpreter.h"

// Supported codepages
#include "cp932.h"
#include "cp936.h"
#include "cp949.h"
#include "western.h"

Codepage* Cp::instance_ = NULL;
int Cp::codepage = -1;
int Cp::scenario = -1;

Codepage& Cp::instance(int desired)
{
	if (desired == -1 && interpreter->Known) {
		// Interpreter known. Base our codepage selection on RLdev metadata.
		int new_scenario = interpreter->getCurrentScenario();
		if (new_scenario == -1) goto use_default;
		if (new_scenario != scenario) {
			UCHAR* ptr = interpreter->getScenarioPtr();
			// If no scenario is loaded, break out for the default.
			if (!ptr) goto use_default;
			// Otherwise acknowledge the scenario and read its header.
			scenario = new_scenario;
			// Assume this is a type 2 header; I've never seen anything else on a DLL-supporting system.
			DWORD meta_offset = *(DWORD*) (ptr + 0x14) + *(DWORD*) (ptr + 0x1c);
			DWORD data_offset = *(DWORD*) (ptr + 0x20);
			if (meta_offset != data_offset) {
				// Metadata present.
				desired = *(ptr + data_offset - 1);
			}
			else {
				// No metadata. Switch to default codepage.
				desired = DefaultCpID;
			}
		}
	}
	if (desired != -1) {
		// Desired codepage known.
		if (desired != codepage) {
			codepage = desired;
			if (instance_ != NULL) {
				delete instance_;
			}
			switch (desired) {
			case 1:
				instance_ = new Cp936();
				break;
			case 2:
				instance_ = new Cp1252();
				break;
			case 3:
				instance_ = new Cp949();
				break;
			default:
				instance_ = new Cp932();
			}
		}
		return *instance_;
	}
use_default:
	if (instance_ == NULL) {
		instance_ = new DefaultCodepage();
		codepage = DefaultCpID;
	}
	return *instance_;
}
