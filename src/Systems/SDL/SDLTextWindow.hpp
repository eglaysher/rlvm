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

#ifndef __SDLTextWindow_hpp__
#define __SDLTextWindow_hpp__

#include "Systems/Base/TextWindow.hpp"

#include <string>
#include <boost/shared_ptr.hpp>
#include <SDL/SDL_ttf.h>

class SDLSurface;
class SelectionElement;

// -----------------------------------------------------------------------

class SDLTextWindow : public TextWindow
{
public:
  SDLTextWindow(RLMachine& machine, int window);
  ~SDLTextWindow();
  virtual void render(RLMachine& machine,
                      std::ostream* tree);

  virtual void clearWin();

  virtual bool displayChar(RLMachine& machine,
                           const std::string& current,
                           const std::string& next);
  virtual int charWidth(unsigned short codepoint) const;

  virtual void setName(RLMachine& machine, const std::string& utf8name,
                       const std::string& next_char);
  virtual void setNameWithoutDisplay(const std::string& utf8name);
  virtual void displayRubyText(RLMachine& machine, const std::string& utf8str);


  virtual void addSelectionItem(const std::string& utf8str);

private:

  void setIndentation();

  void setIndentationIfNextCharIsOpeningQuoteMark(const std::string& next_char);

  /// Converted surface for uploading.
  boost::shared_ptr<SDLSurface> surface_;

  /// Font being used.
  boost::shared_ptr<TTF_Font> font_;
  boost::shared_ptr<TTF_Font> ruby_font_;
};


#endif
