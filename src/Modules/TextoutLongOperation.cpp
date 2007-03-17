// This file is part of RLVM, a RealLive virtual machine clone.
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

#include "Modules/TextoutLongOperation.hpp"

#include "MachineBase/RLMachine.hpp"
#include "Systems/Base/System.hpp"
#include "Systems/Base/TextSystem.hpp"
#include "Systems/Base/TextPage.hpp"
#include "Systems/Base/EventSystem.hpp"

#include <boost/utility.hpp>

#include <string>
#include <iostream>
#include <algorithm>

#include "utf8.h"

using namespace std;

// -----------------------------------------------------------------------
// TextoutLongOperation
// -----------------------------------------------------------------------

TextoutLongOperation::TextoutLongOperation(const std::string& utf8string)
  : m_utf8string(utf8string), m_currentPosition(m_utf8string.begin())
{
}

// -----------------------------------------------------------------------

TextoutLongOperation::~TextoutLongOperation()
{
}

// -----------------------------------------------------------------------

bool TextoutLongOperation::operator()(RLMachine& machine)
{
//  machine.system().event().wait(10);

  // Isolate the next character
  string::iterator it = m_currentPosition;

  int codepoint = utf8::next(it, m_utf8string.end());
  if(codepoint)
  {
    string c(m_currentPosition, it);

    machine.system().text().currentPage(machine).text(c);

    // advance to the next character
    m_currentPosition = it;

    return it == m_utf8string.end();
  }
  else
    return true;
}
