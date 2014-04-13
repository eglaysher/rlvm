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

#include "systems/sdl/sdl_text_window.h"

#include <SDL/SDL_opengl.h>
#include <SDL/SDL_ttf.h>

#include <string>
#include <vector>

#include "libreallive/gameexe.h"
#include "systems/base/colour.h"
#include "systems/base/graphics_system.h"
#include "systems/base/selection_element.h"
#include "systems/base/system_error.h"
#include "systems/base/text_window_button.h"
#include "systems/sdl/sdl_surface.h"
#include "systems/sdl/sdl_system.h"
#include "systems/sdl/sdl_text_system.h"
#include "systems/sdl/sdl_utils.h"
#include "utf8cpp/utf8.h"
#include "utilities/exception.h"
#include "utilities/string_utilities.h"

SDLTextWindow::SDLTextWindow(SDLSystem& system, int window_num)
    : TextWindow(system, window_num), sdl_system_(system) {
  clearWin();
}

SDLTextWindow::~SDLTextWindow() {}

boost::shared_ptr<Surface> SDLTextWindow::textSurface() { return surface_; }

boost::shared_ptr<Surface> SDLTextWindow::nameSurface() {
  return name_surface_;
}

void SDLTextWindow::clearWin() {
  TextWindow::clearWin();

  // Allocate the text window surface
  if (!surface_)
    surface_.reset(new SDLSurface(getSDLGraphics(system()), textSurfaceSize()));
  surface_->Fill(RGBAColour::Clear());

  name_surface_.reset();
}

void SDLTextWindow::renderNameInBox(const std::string& utf8str) {
  RGBColour shadow = RGBAColour::Black().rgb();
  name_surface_ = system_.text().RenderText(
      utf8str, fontSizeInPixels(), 0, 0, font_colour_, &shadow, 0);
}

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

  SelectionElement* element =
      new SelectionElement(system(),
                           boost::shared_ptr<Surface>(new SDLSurface(
                               getSDLGraphics(system()), normal)),
                           boost::shared_ptr<Surface>(new SDLSurface(
                               getSDLGraphics(system()), inverted)),
                           selectionCallback(),
                           selection_id,
                           position);

  text_insertion_point_y_ += (font_size_in_pixels_ + y_spacing_ + ruby_size_);
  selections_.push_back(element);
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

    system_.graphics().MarkScreenAsDirty(GUT_TEXTSYS);

    ruby_begin_point_ = -1;
  }

  last_token_was_name_ = false;
}
