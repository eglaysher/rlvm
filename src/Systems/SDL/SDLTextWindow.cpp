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

/**
 * @file   SDLTextWindow.cpp
 * @author Elliot Glaysher
 * @date   Wed Mar  7 22:11:17 2007
 *
 * @brief
 */

#include "Systems/Base/Colour.hpp"
#include "Systems/Base/SystemError.hpp"
#include "Systems/Base/GraphicsSystem.hpp"
#include "Systems/SDL/SDLTextSystem.hpp"
#include "Systems/Base/TextWindowButton.hpp"
#include "Systems/Base/SelectionElement.hpp"
#include "Systems/SDL/SDLTextWindow.hpp"
#include "Systems/SDL/SDLSurface.hpp"
#include "Systems/SDL/SDLSystem.hpp"
#include "Systems/SDL/SDLUtils.hpp"

#include "libReallive/gameexe.h"

#include <SDL/SDL_opengl.h>
#include <SDL/SDL_ttf.h>

#include <boost/function.hpp>
#include <boost/bind.hpp>
#include "Utilities/Exception.hpp"

#include "utf8cpp/utf8.h"
#include "Utilities/StringUtilities.hpp"

#include <iostream>
#include <string>
#include <vector>

using namespace std;
using namespace boost;

// -----------------------------------------------------------------------

SDLTextWindow::SDLTextWindow(SDLSystem& system, int window_num)
    : TextWindow(system, window_num), sdl_system_(system) {
  clearWin();
}

// -----------------------------------------------------------------------

SDLTextWindow::~SDLTextWindow() {
}

// -----------------------------------------------------------------------

boost::shared_ptr<Surface> SDLTextWindow::textSurface() {
  return surface_;
}

// -----------------------------------------------------------------------

boost::shared_ptr<Surface> SDLTextWindow::nameSurface() {
  return name_surface_;
}

// -----------------------------------------------------------------------

void SDLTextWindow::clearWin() {
  TextWindow::clearWin();

  // Allocate the text window surface
  if (!surface_)
    surface_.reset(new SDLSurface(getSDLGraphics(system()), textSurfaceSize()));
  surface_->fill(RGBAColour::Clear());

  name_surface_.reset();
}

// -----------------------------------------------------------------------

void SDLTextWindow::renderNameInBox(const std::string& utf8str) {
  RGBColour shadow = RGBAColour::Black().rgb();
  name_surface_ = system_.text().renderText(
      utf8str, fontSizeInPixels(), 0, 0, font_colour_, &shadow);
}

// -----------------------------------------------------------------------

void SDLTextWindow::addSelectionItem(const std::string& utf8str,
                                     int selection_id) {
  boost::shared_ptr<TTF_Font> font =
      sdl_system_.text().getFontOfSize(fontSizeInPixels());

  // Render the incoming string for both selected and not-selected.
  SDL_Color colour;
  RGBColourToSDLColor(font_colour_, &colour);

  SDL_Surface* normal =
    TTF_RenderUTF8_Blended(font.get(), utf8str.c_str(), colour);

  // Copy and invert the surface for whatever.
  SDL_Surface* inverted = AlphaInvert(normal);

  // Figure out xpos and ypos
  Point position = textSurfaceRect().origin() +
                   Size(text_insertion_point_x_, text_insertion_point_y_);

  SelectionElement* element = new SelectionElement(
      system().graphics(),
      shared_ptr<Surface>(new SDLSurface(getSDLGraphics(system()), normal)),
      shared_ptr<Surface>(new SDLSurface(getSDLGraphics(system()), inverted)),
      selectionCallback(), selection_id, position);

  text_insertion_point_y_ += (font_size_in_pixels_ + y_spacing_ + ruby_size_);
  selections_.push_back(element);
}

// -----------------------------------------------------------------------

void SDLTextWindow::renderGlyphAt(const std::string& current, int font_size,
                                  const RGBColour& font_colour,
                                  const RGBColour* shadow_colour,
                                  int insertion_point_x,
                                  int insertion_point_y) {
  boost::shared_ptr<TTF_Font> font =
      sdl_system_.text().getFontOfSize(font_size);

  SDL_Color sdl_colour;
  RGBColourToSDLColor(font_colour, &sdl_colour);
  boost::shared_ptr<SDL_Surface> character(
      TTF_RenderUTF8_Blended(font.get(), current.c_str(), sdl_colour),
      SDL_FreeSurface);

  if (character == NULL) {
    // Bug during Kyou's path. The string is printed "". Regression in parser?
    cerr << "WARNING. TTF_RenderUTF8_Blended didn't render the string \""
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
    surface_->blitFROMSurface(
        shadow.get(), Rect(Point(0, 0), offset),
        Rect(insertion + Point(2, 2), offset),
        255);
  }

  Size size(character->w, character->h);
  surface_->blitFROMSurface(
      character.get(), Rect(Point(0, 0), size), Rect(insertion, size), 255);
}

void SDLTextWindow::displayRubyText(const std::string& utf8str) {
  if (ruby_begin_point_ != -1) {
    boost::shared_ptr<TTF_Font> font =
        sdl_system_.text().getFontOfSize(rubyTextSize());
    int end_point = text_insertion_point_x_ - x_spacing_;

    if (ruby_begin_point_ > end_point) {
      ruby_begin_point_ = -1;
      throw rlvm::Exception("We don't handle ruby across line breaks yet!");
    }

    SDL_Color colour;
    RGBColourToSDLColor(font_colour_, &colour);
    SDL_Surface* tmp =
      TTF_RenderUTF8_Blended(font.get(), utf8str.c_str(), colour);

    // Render glyph to surface
    int w = tmp->w;
    int h = tmp->h;
    int height_location = text_insertion_point_y_ - rubyTextSize();
    int width_start =
      int(ruby_begin_point_ + ((end_point - ruby_begin_point_) * 0.5f) -
          (w * 0.5f));
    surface_->blitFROMSurface(
      tmp,
      Rect(Point(0, 0), Size(w, h)),
      Rect(Point(width_start, height_location), Size(w, h)),
      255);
    SDL_FreeSurface(tmp);

    system_.graphics().markScreenAsDirty(GUT_TEXTSYS);

    ruby_begin_point_ = -1;
  }

  last_token_was_name_ = false;
}
