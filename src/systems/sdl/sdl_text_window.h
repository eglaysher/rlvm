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
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
//
// -----------------------------------------------------------------------

#ifndef SRC_SYSTEMS_SDL_SDL_TEXT_WINDOW_H_
#define SRC_SYSTEMS_SDL_SDL_TEXT_WINDOW_H_

#include <SDL2/SDL_ttf.h>

#include <memory>
#include <string>

#include "systems/base/text_window.h"

class SDLSurface;
class SDLSystem;
class SelectionElement;

class SDLTextWindow : public TextWindow {
 public:
  SDLTextWindow(SDLSystem& system, int window);
  virtual ~SDLTextWindow();

  // Overridden from TextWindow:
  virtual std::shared_ptr<Surface> GetTextSurface() override;
  virtual std::shared_ptr<Surface> GetNameSurface() override;
  virtual void ClearWin() override;
  virtual void RenderNameInBox(const std::string& utf8str) override;
  virtual void DisplayRubyText(const std::string& utf8str) override;
  virtual void AddSelectionItem(const std::string& utf8str,
                                int selection_id) override;

 private:
  SDLSystem& sdl_system_;

  std::shared_ptr<SDLSurface> surface_;
  std::shared_ptr<Surface> name_surface_;
};

#endif  // SRC_SYSTEMS_SDL_SDL_TEXT_WINDOW_H_
