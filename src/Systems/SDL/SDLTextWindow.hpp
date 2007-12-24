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

#ifndef __SDLTextWindow_hpp__
#define __SDLTextWindow_hpp__

#include "Systems/Base/TextWindow.hpp"

#include <string>
#include <boost/shared_ptr.hpp>
#include "SDL_ttf.h"

class SDLSurface;
class SelectionElement;

class SDLTextWindow : public TextWindow
{
private:
  /// 
  std::string m_currentValue;

  /// Converted surface for uploading.
  boost::shared_ptr<SDLSurface> m_surface;

  /// Font being used.
  boost::shared_ptr<TTF_Font> m_font;
  boost::shared_ptr<TTF_Font> m_rubyFont;

  /// Insertion point for text.
  int m_insertionPointX, m_insertionPointY;

  /// Current ruby insertion point (or -1 if markRubyBegin() hasn't
  /// been called)
  int m_rubyBeginPoint;

  boost::shared_ptr<SDLSurface> m_wakuMain;
  boost::shared_ptr<SDLSurface> m_wakuBacking;
  boost::shared_ptr<SDLSurface> m_wakuButton;

  typedef boost::ptr_vector<SelectionElement> Selections;
  Selections m_selections;

  void setIndentation();

  void renderButtons(RLMachine& machine);

  void setIndentationIfNextCharIsOpeningQuoteMark(const std::string& nextChar);

public:
  SDLTextWindow(RLMachine& machine, int window);
  ~SDLTextWindow();

  void setCurrentText(RLMachine& machine, const std::string& tex);

  virtual void setMousePosition(RLMachine& machine, int x, int y);
  virtual bool handleMouseClick(RLMachine& machine, int x, int y, bool pressed);

  virtual void render(RLMachine& machine);

  virtual void clearWin();

  virtual bool displayChar(RLMachine& machine, 
                           const std::string& current,
                           const std::string& next);

  virtual bool isFull() const;

  virtual void setWakuMain(RLMachine& machine, const std::string& name);
  virtual void setWakuBacking(RLMachine& machine, const std::string& name);
  virtual void setWakuButton(RLMachine& machine, const std::string& name);

  virtual void setName(RLMachine& machine, const std::string& utf8name,
                       const std::string& nextChar);
  virtual void hardBrake();
  virtual void resetIndentation();

  virtual void markRubyBegin();
  virtual void displayRubyText(RLMachine& machine, const std::string& utf8str);


  virtual void addSelectionItem(RLMachine& machine, const std::string& utf8str);
  virtual void endSelectionMode();
};


#endif
