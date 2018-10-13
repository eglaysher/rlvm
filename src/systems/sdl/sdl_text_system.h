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

#ifndef SRC_SYSTEMS_SDL_SDL_TEXT_SYSTEM_H_
#define SRC_SYSTEMS_SDL_SDL_TEXT_SYSTEM_H_

#include <SDL2/SDL_ttf.h>

#include <map>
#include <string>

#include "systems/base/text_system.h"

class Point;
class RLMachine;
class SDLSystem;
class SDLTextWindow;
class TextWindow;

class SDLTextSystem : public TextSystem {
 public:
  SDLTextSystem(SDLSystem& system, Gameexe& gameexe);
  ~SDLTextSystem();

  virtual std::shared_ptr<TextWindow> GetTextWindow(int text_window_number) override;

  virtual Size RenderGlyphOnto(const std::string& current,
                               int font_size,
                               bool italic,
                               const RGBColour& font_colour,
                               const RGBColour* shadow_colour,
                               int insertion_point_x,
                               int insertion_point_y,
                               const std::shared_ptr<Surface>& destination) override;
  virtual int GetCharWidth(int size, uint16_t codepoint) override;
  bool FontIsMonospaced() override;

  // Returns (and caches) a SDL_ttf font object for a font of |size|.
  std::shared_ptr<TTF_Font> GetFontOfSize(int size);

 private:
  // Font storage.
  typedef std::map<int, std::shared_ptr<TTF_Font>> FontSizeMap;
  FontSizeMap map_;

  SDLSystem& sdl_system_;

  std::unique_ptr<bool> is_monospace_;
};

#endif  // SRC_SYSTEMS_SDL_SDL_TEXT_SYSTEM_H_
