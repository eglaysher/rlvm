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

#ifndef __SDLTextSystem_hpp__
#define __SDLTextSystem_hpp__

#include "Systems/Base/TextSystem.hpp"

#include <boost/ptr_container/ptr_map.hpp>

class TextWindow;
class SDLTextWindow;

class SDLTextSystem : public TextSystem
{
private:
  /// Window dispatch
  typedef boost::ptr_map<int, SDLTextWindow> WindowMap;
  WindowMap m_textWindow;

public:
  SDLTextSystem();
  ~SDLTextSystem();

  virtual void render(RLMachine& machine);
  virtual void hideAllTextWindows();
  virtual TextWindow& textWindow(RLMachine&, int textWindowNumber);
};

#endif
