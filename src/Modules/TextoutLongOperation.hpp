// This file is part of RLVM, a RealLive virtual machine clone.
//
// -----------------------------------------------------------------------
//
// Copyright (C) 2006 Elliot Glaysher
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

#ifndef __TextoutLongOperation_hpp__
#define __TextoutLongOperation_hpp__

#include "MachineBase/LongOperation.hpp"
#include "Systems/Base/EventHandler.hpp"

#include <boost/function.hpp>
#include <string>

class RLMachine;

class TextoutLongOperation : public NiceLongOperation, public EventHandler
{
private:
  std::string m_utf8string;

  int m_currentCodepoint;
  std::string m_currentChar;
  std::string::iterator m_currentPosition;

  /// Sets whether we should display as much text as we can immediately.
  bool m_noWait;

  bool displayAsMuchAsWeCanThenPause(RLMachine& machine);

  bool displayName(RLMachine& machine);
  bool displayOneMoreCharacter(RLMachine& machine, bool& paused);
  
public:
  TextoutLongOperation(RLMachine& machine, const std::string& utf8string);
  ~TextoutLongOperation();

  void setNoWait(bool in = true) { m_noWait = in; }

  virtual void mouseButtonStateChanged(MouseButton mouseButton, bool pressed);
  virtual void keyStateChanged(KeyCode keyCode, bool pressed);

  virtual bool operator()(RLMachine& machine);
};

// -----------------------------------------------------------------------

void printTextToFunction(
  boost::function<void(const std::string& c, const std::string& nextChar)> fun,
  const std::string& charsToPrint, const std::string& nextCharForFinal);

#endif
