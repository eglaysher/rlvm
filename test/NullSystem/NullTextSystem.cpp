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

#include "NullTextSystem.hpp"
#include "NullTextWindow.hpp"

// -----------------------------------------------------------------------

NullTextSystem::NullTextSystem(Gameexe& gexe) 
  : TextSystem(gexe) {}

// -----------------------------------------------------------------------

NullTextSystem::~NullTextSystem() { }

// -----------------------------------------------------------------------

TextWindow& NullTextSystem::textWindow(RLMachine& machine, int textWindowNum)
{
  WindowMap::iterator it = m_textWindow.find(textWindowNum);
  if(it == m_textWindow.end())
  {
    it = m_textWindow.insert(
      textWindowNum, new NullTextWindow(machine, textWindowNum)).first;
  }

  return *it;
}


