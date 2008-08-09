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
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
//
// -----------------------------------------------------------------------
/**
 * @file   cp932toUnicode.hpp
 * @author Elliot Glaysher
 * @date   Sun Sep 17 14:24:05 2006
 *
 * @brief  Header that declares the two string conversion functions...
 */

#ifndef __cp932toUnicode_hpp__
#define __cp932toUnicode_hpp__

#include <string>

std::string unicodetocp932(const std::wstring& line);
std::wstring cp932toUnicode(const std::string& line, int transformation);

std::string hantozen_cp932(const std::string& string);
std::string zentohan_cp932(const std::string& string);

std::string unicodeToUTF8(const std::wstring& widestring);

// For convenience...
std::string cp932toUTF8(const std::string& line, int transformation);

bool isKinsoku(int codepoint);
int codepoint(const std::string& c);

#endif
