// -*- Mode: C++; tab-width:2; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi:tw=80:et:ts=2:sts=2
//
// -----------------------------------------------------------------------
//
// This file is part of RLVM, a RealLive virtual machine clone.
//
// -----------------------------------------------------------------------
//
// Copyright (C) 2006, 2007 Elliot Glaysher
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 3 of the License, or
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

#include "SDL_ttf.h"

class RLMachine;
class TextWindow;
class SDLTextWindow;
class Point;

class SDLTextSystem : public TextSystem
{
private:
  /**
   * @name Font storage
   *
   * @{
   */
  typedef std::map< int , boost::shared_ptr<TTF_Font> > FontSizeMap;
  FontSizeMap m_map;
  /// @}

  void updateWindowsForChangeToWindowAttr();

public:
  SDLTextSystem(Gameexe& gameexe);
  ~SDLTextSystem();

  virtual void render(RLMachine& machine);
  virtual TextWindow& textWindow(RLMachine&, int textWindowNumber);

  virtual void setDefaultWindowAttr(const std::vector<int>& attr);

  virtual void setWindowAttrR(int i);
  virtual void setWindowAttrG(int i);
  virtual void setWindowAttrB(int i);
  virtual void setWindowAttrA(int i);
  virtual void setWindowAttrF(int i);

  virtual void setMousePosition(RLMachine& machine, const Point& pos);
  virtual bool handleMouseClick(RLMachine& machine, const Point& pos,
                                bool pressed);

  virtual boost::shared_ptr<Surface> renderText(
    RLMachine& machine, const std::string& utf8str, int size, int xspace,
    int yspace, int colour);

  boost::shared_ptr<TTF_Font> getFontOfSize(RLMachine& machine, int size);
};

#endif

