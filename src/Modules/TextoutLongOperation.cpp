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
    m_currentCodepoint(0), m_currentPosition(m_utf8string.begin()), 
    m_noWait(false)
{
  // Retrieve the first character (prime the loop in operator())
  string::iterator tmp = m_currentPosition;
  m_currentCodepoint = utf8::next(tmp, m_utf8string.end());
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
  throw "Unimplemented";
}

// -----------------------------------------------------------------------

/** 
 * Extract a name and send it to the text system as an automic
 * operation.
 *
 * @todo Right now, this doesn't deal with ###PRINT() syntax in the
 *       name, even though character names are one of the places where
 *       that's evaluated.
 */
bool TextoutLongOperation::displayName(RLMachine& machine)
{
  // Ignore the starting bracket
  string::iterator it = m_currentPosition;
  string::iterator curend = it;
  string::iterator strend = m_utf8string.end();
  int codepoint = utf8::next(it, strend);

  // Eat all characters between the name brackets
  while(codepoint != 0x3011 && it != strend)
  {
    curend = it;
    codepoint = utf8::next(it, strend);
  }

  if(it == strend)
    throw "Malformed string code. Opening bracket in \{name} construct, but"
      " missing closing bracket.";

  // Grab the name
  string name(m_currentPosition, curend);

  // Consume the next character
  m_currentPosition = it;
  m_currentCodepoint = utf8::next(it, strend);
  m_currentChar = string(m_currentPosition, it);
  m_currentPosition = it;

  TextPage& page = machine.system().text().currentPage(machine);
//  cerr << "Displaying name: " << name << endl;
  page.name(name, m_currentChar);

  return false;
}

// -----------------------------------------------------------------------

bool TextoutLongOperation::displayOneMoreCharacter(RLMachine& machine)
{
  if(m_currentCodepoint == 0x3010)
  {
    // The current character is the opening character for a name. We
    // treat names as a single display operation
    return displayName(machine);
  }
  else
  {
    // Isolate the next character
    string::iterator it = m_currentPosition;
    string::iterator strend = m_utf8string.end();

    if(it != strend)
    {
      int codepoint = utf8::next(it, strend);
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
//        cerr << "Pause " << endl;
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
}

// -----------------------------------------------------------------------

bool TextoutLongOperation::operator()(RLMachine& machine)
{
  if(m_noWait)
    return displayAsMuchAsWeCanThenPause(machine);
  else
    return displayOneMoreCharacter(machine);
}

// -----------------------------------------------------------------------

void printTextToFunction(
  boost::function<void(const std::string& c, const std::string& nextChar)> fun,
  const std::string& charsToPrint, const std::string& nextCharForFinal)
{
  // Iterate over each incoming character to display (we do this
  // instead of rendering the entire string so that we can perform
  // indentation, et cetera.)
  string::const_iterator cur = charsToPrint.begin();
  string::const_iterator tmp = cur;
  string::const_iterator end = charsToPrint.end();
  utf8::next(tmp, end);
  string curChar(cur, tmp);
  for(cur = tmp; tmp != end; cur = tmp)
  {
    utf8::next(tmp, end);
    string next(cur, tmp);
    fun(curChar, next);
    curChar = next;
  }

  fun(curChar, nextCharForFinal);
}
