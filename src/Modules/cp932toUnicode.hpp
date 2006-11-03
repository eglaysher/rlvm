// This file is part of RLVM, a RealLive virutal machine clone.
//
// -----------------------------------------------------------------------
//
// Copyright (C) 2006 El Riot
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
std::wstring cp932toUnicode(const std::string& line);

std::string hantozen_cp932(const std::string& string);
std::string zentohan_cp932(const std::string& string);

#endif
