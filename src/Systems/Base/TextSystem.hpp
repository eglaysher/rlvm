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

#ifndef __TextSystem_hpp__
#define __TextSystem_hpp__

#include <string>

class RLMachine;

class TextSystem
{
private:
  /// Fast text mode
  bool m_fastTextMode;

  /// Internal 'no wait' flag
  bool m_messageNoWait;

  /// Message speed; range from 0 to 255
  char m_messageSpeed;

public:
  virtual void setActiveTextWindow(RLMachine& machine, int window) = 0;
  virtual void render(RLMachine& machine) = 0;

  virtual void setCurrentText(RLMachine& machine, 
                              const std::string& cp932encodedText) = 0;

  void setFastTextMode(int i) { m_fastTextMode = i; }
  int fastTextMode() const { return m_fastTextMode; }

  void setMessageNoWait(int i) { m_messageNoWait = i; }
  int messageNoWait() const { return m_messageNoWait; }

  void setMessageSpeed(int i) { m_messageSpeed = i; }
  int messageSpeed() const { return m_messageSpeed; }
};

#endif



