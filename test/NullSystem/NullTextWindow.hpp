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

#ifndef __NullTextWindow_hpp__
#define __NullTextWindow_hpp__

#include <string>
#include "Systems/Base/TextWindow.hpp"

class NullTextWindow : public TextWindow
{
private:
  std::string m_currentContents;

public:
  NullTextWindow(RLMachine& machine, int x) : TextWindow(machine, x) {}
  ~NullTextWindow() {}
  virtual void execute(RLMachine& machine) {}

  // We don't test graphics in the null system, so don't really
  // implement the waku parts.
  virtual void setWakuMain(RLMachine& machine, const std::string& name) {}
  virtual void setWakuBacking(RLMachine& machine, const std::string& name) {}
  virtual void setWakuButton(RLMachine& machine, const std::string& name) {}
  virtual void render(RLMachine& machine) {}

  // To implement
  virtual void clearWin();
  virtual bool displayChar(RLMachine& machine, const std::string& current,
                           const std::string& next);
  virtual void setName(RLMachine& machine, const std::string& utf8name, 
                       const std::string& nextChar);

  virtual void hardBrake();
  virtual void resetIndentation() {}

  virtual void markRubyBegin() {}
  virtual void displayRubyText(RLMachine& machine, const std::string& utf8str) {}


  virtual bool isFull() const { return false; }


  std::string currentContents() const { return m_currentContents; }

  virtual void addSelectionItem(RLMachine& machine, const std::string& utf8str) {}
};

#endif
