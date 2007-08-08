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

#ifndef __NullTextSystem_hpp__
#define __NullTextSystem_hpp__

#include "Systems/Base/TextSystem.hpp"
#include <boost/ptr_container/ptr_map.hpp>

class RLMachine;
class TextWindow;
class NullTextWindow;

class NullTextSystem : public TextSystem
{
private:
  /// Window dispatch
  typedef boost::ptr_map<int, NullTextWindow> WindowMap;
  WindowMap m_textWindow;

public:
  NullTextSystem(Gameexe& gexe);
  ~NullTextSystem();

  void executeTextSystem(RLMachine& machine) { }

  void render(RLMachine& machine) { }
  void hideTextWindow(int winNumber) { }
  void hideAllTextWindows() { }
  void clearAllTextWindows() { }

  TextWindow& textWindow(RLMachine& machine, int textWindowNumber);

  // We can safely ignore mouse clicks...for now.
  void setMousePosition(RLMachine& machine, int x, int y) { }
  bool handleMouseClick(RLMachine& machine, int x, int y,
                        bool pressed) { }

  boost::shared_ptr<Surface> renderText(
	RLMachine& machine, const std::string& utf8str, int size, int xspace,
	int yspace, int colour) { return boost::shared_ptr<Surface>(); }
};

#endif
