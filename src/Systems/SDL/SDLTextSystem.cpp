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

#include "Systems/SDL/SDLTextSystem.hpp"

#include "Systems/Base/GraphicsSystem.hpp"
#include "Systems/Base/Rect.hpp"
#include "Systems/Base/SystemError.hpp"
#include "Systems/Base/TextKeyCursor.hpp"
#include "Systems/SDL/SDLSurface.hpp"
#include "Systems/SDL/SDLSystem.hpp"
#include "Systems/SDL/SDLTextWindow.hpp"
#include "Systems/SDL/SDLUtils.hpp"
#include "Utilities/Exception.hpp"
#include "Utilities/algoplus.hpp"
#include "Utilities/findFontFile.h"
#include "libReallive/gameexe.h"

#include <boost/bind.hpp>
#include <SDL/SDL_ttf.h>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

using namespace std;
using namespace boost;

SDLTextSystem::SDLTextSystem(SDLSystem& system, Gameexe& gameexe)
    : TextSystem(system, gameexe), sdl_system_(system) {
  if (TTF_Init() == -1) {
    ostringstream oss;
    oss << "Error initializing SDL_ttf: " << TTF_GetError();
    throw SystemError(oss.str());
  }
}

SDLTextSystem::~SDLTextSystem() {
  TTF_Quit();
}

boost::shared_ptr<TextWindow> SDLTextSystem::textWindow(int text_window) {
  WindowMap::iterator it = text_window_.find(text_window);
  if (it == text_window_.end()) {
    it = text_window_.insert(std::make_pair(
      text_window, shared_ptr<TextWindow>(
          new SDLTextWindow(sdl_system_, text_window)))).first;
  }

  return it->second;
}

boost::shared_ptr<Surface> SDLTextSystem::renderText(
  const std::string& utf8str, int size, int xspace, int yspace,
  const RGBColour& colour, RGBColour* shadow_colour) {
  // TODO(erg): Entirely rewrite this method in terms of renderUTF8Glyph so we
  // can handle the x/yspacing correctly.

  // Pick the correct font
  shared_ptr<TTF_Font> font = getFontOfSize(size);

  // Naively render. Ignore most of the arguments for now
  SDL_Color sdl_colour;
  RGBColourToSDLColor(colour, &sdl_colour);
  boost::shared_ptr<SDL_Surface> text(
      TTF_RenderUTF8_Blended(font.get(), utf8str.c_str(), sdl_colour),
      SDL_FreeSurface);
  if (text == NULL) {
    // TODO(erg): Make our callers check for NULL.
    return shared_ptr<Surface>(new SDLSurface(getSDLGraphics(system()),
                                              buildNewSurface(Size(1, 1))));
  }

  boost::shared_ptr<SDL_Surface> shadow;
  if (shadow_colour && fontShadow()) {
    SDL_Color sdl_shadow_colour;
    RGBColourToSDLColor(*shadow_colour, &sdl_shadow_colour);
    shadow.reset(
        TTF_RenderUTF8_Blended(font.get(), utf8str.c_str(), sdl_shadow_colour),
        SDL_FreeSurface);
  }

  // TODO(erg): cast is least evil for now because I would otherwise have to do
  // some major redesign to *System.
  Size out_size(text->w, text->h);
  boost::shared_ptr<SDLSurface> surface(
      boost::static_pointer_cast<SDLSurface>(
          system().graphics().buildSurface(out_size)));

  // TODO(erg): Surely there's a way to allocate with something other than
  // black, right?
  surface->fill(RGBAColour::Clear());

  if (shadow) {
    Size offset(text->w - 2, text->h - 2);
    surface->blitFROMSurface(
        shadow.get(), Rect(Point(0, 0), offset), Rect(Point(2, 2), offset),
        255);
  }
  surface->blitFROMSurface(
      text.get(), Rect(Point(0, 0), out_size), Rect(Point(0, 0), out_size),
      255);

  return surface;
}

void SDLTextSystem::renderGlyphOnto(
    const std::string& current,
    int font_size,
    const RGBColour& font_colour,
    const RGBColour* shadow_colour,
    int insertion_point_x,
    int insertion_point_y,
    const boost::shared_ptr<Surface>& destination) {
  SDLSurface* sdl_surface = static_cast<SDLSurface*>(destination.get());

  boost::shared_ptr<TTF_Font> font = getFontOfSize(font_size);

  // TODO: Glyphs go in a cache?
  SDL_Color sdl_colour;
  RGBColourToSDLColor(font_colour, &sdl_colour);
  boost::shared_ptr<SDL_Surface> character(
      TTF_RenderUTF8_Blended(font.get(), current.c_str(), sdl_colour),
      SDL_FreeSurface);

  if (character == NULL) {
    // Bug during Kyou's path. The string is printed "". Regression in parser?
    cerr << "WARNING. TTF_RenderUTF8_Blended didn't render the character \""
         << current << "\". Hopefully continuing..." << endl;
    return;
  }

  boost::shared_ptr<SDL_Surface> shadow;
  if (shadow_colour && sdl_system_.text().fontShadow()) {
    SDL_Color sdl_shadow_colour;
    RGBColourToSDLColor(*shadow_colour, &sdl_shadow_colour);

    shadow.reset(
        TTF_RenderUTF8_Blended(font.get(), current.c_str(), sdl_shadow_colour),
        SDL_FreeSurface);
  }

  Point insertion(insertion_point_x, insertion_point_y);

  if (shadow) {
    Size offset(shadow->w, shadow->h);
    sdl_surface->blitFROMSurface(
        shadow.get(), Rect(Point(0, 0), offset),
        Rect(insertion + Point(2, 2), offset),
        255);
  }

  Size size(character->w, character->h);
  sdl_surface->blitFROMSurface(
      character.get(), Rect(Point(0, 0), size), Rect(insertion, size), 255);
}

int SDLTextSystem::charWidth(int size, uint16_t codepoint) {
  boost::shared_ptr<TTF_Font> font = getFontOfSize(size);
  int minx, maxx, miny, maxy, advance;
  TTF_GlyphMetrics(font.get(), codepoint,
                   &minx, &maxx, &miny, &maxy, &advance);
  return advance;
}

boost::shared_ptr<TTF_Font> SDLTextSystem::getFontOfSize(int size) {
  FontSizeMap::iterator it = map_.find(size);
  if (it == map_.end()) {
    string filename = findFontFile(system()).external_file_string();
    TTF_Font* f = TTF_OpenFont(filename.c_str(), size);
    if (f == NULL) {
      ostringstream oss;
      oss << "Error loading font: " << TTF_GetError();
      throw SystemError(oss.str());
    }

    TTF_SetFontStyle(f, TTF_STYLE_NORMAL);

    // Build a smart_ptr to own this font, and set a deleter function.
    shared_ptr<TTF_Font> font(f, TTF_CloseFont);

    map_[size] = font;
    return font;
  } else {
    return it->second;
  }
}
