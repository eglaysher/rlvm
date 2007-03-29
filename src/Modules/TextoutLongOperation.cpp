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

#include "Precompiled.hpp"

// -----------------------------------------------------------------------

#include "Modules/TextoutLongOperation.hpp"
#include "Modules/Module_Msg.hpp"

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

TextoutLongOperation::TextoutLongOperation(RLMachine& machine,
                                           const std::string& utf8string)
  : NiceLongOperation(machine), m_utf8string(utf8string), 
    m_currentPosition(m_utf8string.begin()), m_noWait(false)
{
  // Retrieve the first character (prime the loop in operator())
  string::iterator tmp = m_currentPosition;
  utf8::next(tmp, m_utf8string.end());
  m_currentChar = string(m_currentPosition, tmp);
  m_currentPosition = tmp;

  cerr << "UTF: " << m_utf8string << endl;
}

// -----------------------------------------------------------------------

TextoutLongOperation::~TextoutLongOperation()
{
}

// -----------------------------------------------------------------------

bool TextoutLongOperation::displayAsMuchAsWeCanThenPause(RLMachine& machine)
{
  // Continue to print characters until 

  
}

// -----------------------------------------------------------------------

bool TextoutLongOperation::displayOneMoreCharacter(RLMachine& machine)
{
  // Isolate the next character
  string::iterator it = m_currentPosition;

  if(it != m_utf8string.end())
  {
    int codepoint = utf8::next(it, m_utf8string.end());
    TextPage& page = machine.system().text().currentPage(machine);
    if(codepoint)
    {
      string nextChar(m_currentPosition, it);
      bool rendered = page.character(m_currentChar, nextChar);

      // Check to see if this character was rendered to the screen. If
      // this is false, then the page is probably full and the check
      // later on will do something about that.
      if(rendered)
      {
        m_currentChar = nextChar;
        m_currentPosition = it;
      }
    }
    else
    {
      // advance to the next character if we've somehow hit an
      // embedded NULL that isn't the end of the string
      m_currentPosition = it;
    }

    // Call the pause operation if we've filled up the current page.
    if(page.isFull())
    {
      cerr << "Pause " << endl;
      machine.pushLongOperation(new Longop_pause(machine));
    }

    return false;
  }
  else
  {
    machine.system().text().currentPage(machine).
      character(m_currentChar, "");

    return true;
  }
}

// -----------------------------------------------------------------------

bool TextoutLongOperation::operator()(RLMachine& machine)
{
  if(m_noWait)
    return displayAsMuchAsWeCanThenPause(machine);
  else
    return displayOneMoreCharacter(machine);
}
