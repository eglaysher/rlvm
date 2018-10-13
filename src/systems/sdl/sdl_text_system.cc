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

#include "systems/sdl/sdl_text_system.h"

#include <SDL2/SDL_ttf.h>

#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include "systems/base/graphics_system.h"
#include "systems/base/rect.h"
#include "systems/base/system_error.h"
#include "systems/base/text_key_cursor.h"
#include "systems/sdl/sdl_surface.h"
#include "systems/sdl/sdl_system.h"
#include "systems/sdl/sdl_text_window.h"
#include "systems/sdl/sdl_utils.h"
#include "utilities/exception.h"
#include "utilities/find_font_file.h"
#include "libreallive/gameexe.h"

SDLTextSystem::SDLTextSystem(SDLSystem& system, Gameexe& gameexe)
    : TextSystem(system, gameexe), sdl_system_(system) {
  if (TTF_Init() == -1) {
    std::ostringstream oss;
    oss << "Error initializing SDL_ttf: " << TTF_GetError();
    throw SystemError(oss.str());
  }
}

SDLTextSystem::~SDLTextSystem() {
  // We should be calling TTF_Quit() here, but somebody is holding on to a font
  // reference so we'll just leak the FreeType structures.
}

std::shared_ptr<TextWindow> SDLTextSystem::GetTextWindow(int text_window) {
  WindowMap::iterator it = text_window_.find(text_window);
  if (it == text_window_.end()) {
    it = text_window_.emplace(text_window,
                              std::shared_ptr<TextWindow>(new SDLTextWindow(
                                  sdl_system_, text_window))).first;
  }

  return it->second;
}

Size SDLTextSystem::RenderGlyphOnto(
    const std::string& current,
    int font_size,
    bool italic,
    const RGBColour& font_colour,
    const RGBColour* shadow_colour,
    int insertion_point_x,
    int insertion_point_y,
    const std::shared_ptr<Surface>& destination) {
  SDLSurface* sdl_surface = static_cast<SDLSurface*>(destination.get());

  std::shared_ptr<TTF_Font> font = GetFontOfSize(font_size);

  if (italic) {
    TTF_SetFontStyle(font.get(), TTF_STYLE_ITALIC);
  }

  SDL_Color sdl_colour;
  RGBColourToSDLColor(font_colour, &sdl_colour);
  std::shared_ptr<SDL_Surface> character(
      TTF_RenderUTF8_Blended(font.get(), current.c_str(), sdl_colour),
      SDL_FreeSurface);

  if (character == NULL) {
    // Bug during Kyou's path. The string is printed "". Regression in parser?
    std::cerr << "WARNING. TTF_RenderUTF8_Blended didn't render the "
              << "character \"" << current << "\". Hopefully continuing..."
              << std::endl;
    return Size(0, 0);
  }

  std::shared_ptr<SDL_Surface> shadow;
  if (shadow_colour && sdl_system_.text().font_shadow()) {
    SDL_Color sdl_shadow_colour;
    RGBColourToSDLColor(*shadow_colour, &sdl_shadow_colour);

    shadow.reset(
        TTF_RenderUTF8_Blended(font.get(), current.c_str(), sdl_shadow_colour),
        SDL_FreeSurface);
  }

  if (italic) {
    TTF_SetFontStyle(font.get(), TTF_STYLE_NORMAL);
  }

  Point insertion(insertion_point_x, insertion_point_y);

  if (shadow) {
    Size offset(shadow->w, shadow->h);
    sdl_surface->blitFROMSurface(shadow.get(),
                                 Rect(Point(0, 0), offset),
                                 Rect(insertion + Point(2, 2), offset),
                                 255);
  }

  Size size(character->w, character->h);
  sdl_surface->blitFROMSurface(
      character.get(), Rect(Point(0, 0), size), Rect(insertion, size), 255);
  return size;
}

int SDLTextSystem::GetCharWidth(int size, uint16_t codepoint) {
  std::shared_ptr<TTF_Font> font = GetFontOfSize(size);
  int minx, maxx, miny, maxy, advance;
  TTF_GlyphMetrics(font.get(), codepoint, &minx, &maxx, &miny, &maxy, &advance);
  return advance;
}

std::shared_ptr<TTF_Font> SDLTextSystem::GetFontOfSize(int size) {
  FontSizeMap::iterator it = map_.find(size);
  if (it == map_.end()) {
    std::string filename = FindFontFile(system()).native();
    TTF_Font* f = TTF_OpenFont(filename.c_str(), size);
    if (f == NULL) {
      std::ostringstream oss;
      oss << "Error loading font: " << TTF_GetError();
      throw SystemError(oss.str());
    }

    TTF_SetFontStyle(f, TTF_STYLE_NORMAL);

    // Build a smart_ptr to own this font, and set a deleter function.
    std::shared_ptr<TTF_Font> font(f, TTF_CloseFont);

    map_[size] = font;

    // Now that we've put this font into the cache, we can safely use
    // GetCharWidth to ensure whether we're monospaced.
    if (!is_monospace_.get()) {
      is_monospace_.reset(new bool);
      // Why not use TTF_FontFaceIsFixedWidth()? Because that checks if the
      // font is fixed width, which msgothic.ttc among others is not. However,
      // it does use monospaced characters.
      *is_monospace_ = GetCharWidth(size, 'i') == GetCharWidth(size, 'm');
    }
    return font;
  } else {
    return it->second;
  }
}

bool SDLTextSystem::FontIsMonospaced() {
  return is_monospace_ ? *is_monospace_ : false;
}
