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

#include "Precompiled.hpp"

// -----------------------------------------------------------------------

/**
 * @file   SDLTextWindow.cpp
 * @author Elliot Glaysher
 * @date   Wed Mar  7 22:11:17 2007
 *
 * @brief
 */

#include "Systems/Base/System.hpp"
#include "Systems/Base/SystemError.hpp"
#include "Systems/Base/GraphicsSystem.hpp"
#include "Systems/SDL/SDLTextSystem.hpp"
#include "Systems/Base/TextWindowButton.hpp"
#include "Systems/Base/SelectionElement.hpp"
#include "Systems/SDL/SDLTextWindow.hpp"
#include "Systems/SDL/SDLSurface.hpp"
#include "Systems/SDL/SDLUtils.hpp"

#include "libReallive/gameexe.h"

#include <SDL/SDL_opengl.h>
#include <SDL/SDL_ttf.h>

#include <boost/function.hpp>
#include <boost/bind.hpp>
#include "Utilities.h"

#include "utf8.h"
#include "Utilities/StringUtilities.hpp"

#include <iostream>
#include <vector>

using namespace std;
using namespace boost;

// -----------------------------------------------------------------------

SDLTextWindow::SDLTextWindow(System& system, int window_num)
  : TextWindow(system, window_num)
{
  SDLTextSystem& text = dynamic_cast<SDLTextSystem&>(system.text());
  font_ = text.getFontOfSize(fontSizeInPixels());
  ruby_font_ = text.getFontOfSize(rubyTextSize());

  clearWin();
}

// -----------------------------------------------------------------------

SDLTextWindow::~SDLTextWindow()
{
}

// -----------------------------------------------------------------------

void SDLTextWindow::clearWin() {
  TextWindow::clearWin();

  // Allocate the text window surface
  if (!surface_)
    surface_.reset(new SDLSurface(textWindowSize()));
  surface_->fill(RGBAColour::Clear());
}

// -----------------------------------------------------------------------

bool SDLTextWindow::displayChar(const std::string& current,
                                const std::string& next)
{
  // If this text page is already full, save some time and reject
  // early.
  if(isFull())
    return false;

  setVisible(true);

  if(current != "")
  {
    SDL_Color color;
    RGBColourToSDLColor(font_colour_, &color);
    int cur_codepoint = codepoint(current);
    int next_codepoint = codepoint(next);

    // U+3010 (LEFT BLACK LENTICULAR BRACKET) and U+3011 (RIGHT BLACK
    // LENTICULAR BRACKET) should be handled before this
    // function. Otherwise, it's an error.
    if(cur_codepoint == 0x3010 || cur_codepoint == 0x3011)
    {
      throw SystemError(
        "Bug in parser; \\{name} construct should be handled before display_char");
    }

    SDL_Surface* tmp =
      TTF_RenderUTF8_Blended(font_.get(), current.c_str(), color);

    if (tmp == NULL) {
      // Bug during Kyou's path. The string is printed "". Regression in parser?
      cerr << "WARNING. TTF_RenderUTF8_Blended didn't render the string \""
           << current << "\". Hopefully continuing..." << endl;

      return true;
    }

    // If the width of this glyph plus the spacing will put us over the
    // edge of the window, then line increment.
    //
    // If the current character will fit on this line, and it is NOT
    // in this set, then we should additionally check the next
    // character.  If that IS in this set and will not fit on the
    // current line, then we break the line before the current
    // character instead, to prevent the next character being stranded
    // at the start of a line.
    //
    bool char_will_fit_on_line = text_insertion_point_x_ + tmp->w + x_spacing_ <=
      textWindowSize().width();
    bool next_char_will_fit_on_line = text_insertion_point_x_ + 2*(tmp->w + x_spacing_) <=
      textWindowSize().width();
    if(!char_will_fit_on_line ||
       (char_will_fit_on_line && !isKinsoku(cur_codepoint) &&
        !next_char_will_fit_on_line && isKinsoku(next_codepoint)))
    {
      hardBrake();

      if(isFull())
        return false;
    }

    // Render glyph to surface
    Size s(tmp->w,tmp->h);
    surface_->blitFROMSurface(
      tmp,
      Rect(Point(0, 0), s),
      Rect(Point(text_insertion_point_x_, text_insertion_point_y_), s),
      255);

    // Move the insertion point forward one character
    text_insertion_point_x_ += font_size_in_pixels_ + x_spacing_;

    SDL_FreeSurface(tmp);
  }

  // When we aren't rendering a piece of text with a ruby gloss, mark
  // the screen as dirty so that this character renders.
  if(ruby_begin_point_ == -1)
  {
    system_.graphics().markScreenAsDirty(GUT_TEXTSYS);
  }

  return true;
}

// -----------------------------------------------------------------------

int SDLTextWindow::charWidth(unsigned short codepoint) const {
  int minx, maxx, miny, maxy, advance;
  TTF_GlyphMetrics(font_.get(), codepoint,
                   &minx, &maxx, &miny, &maxy, &advance);
  return advance;
}

// -----------------------------------------------------------------------

void SDLTextWindow::setIndentation()
{
  current_indentation_in_pixels_ = text_insertion_point_x_;
}

// -----------------------------------------------------------------------

void SDLTextWindow::setName(const std::string& utf8name,
                            const std::string& next_char)
{
  if (name_mod_ == 0) {
    // Display the name in one pass
    printTextToFunction(bind(&SDLTextWindow::displayChar, ref(*this),_1, _2),
                        utf8name, next_char);
    setIndentation();

    setIndentationIfNextCharIsOpeningQuoteMark(next_char);
  } else {
    setNameWithoutDisplay(utf8name);
  }
}

// -----------------------------------------------------------------------

void SDLTextWindow::setNameWithoutDisplay(const std::string& utf8name) {
  if(name_mod_ == 0) {
    // TODO: Save the name for some reason?
  } else if(name_mod_ == 1) {
    throw SystemError("NAME_MOD=1 is unsupported.");
  } else if(name_mod_ == 2) {
    // This doesn't actually fix the problem in Planetarian because
    // the call to set the name and the actual quotetext are in two
    // different strings. This logic will need to be moved.
//    setIndentationIfNextCharIsOpeningQuoteMark(next_char);
  } else {
    throw SystemError("Invalid");
  }
}

// -----------------------------------------------------------------------

void SDLTextWindow::setIndentationIfNextCharIsOpeningQuoteMark(
  const std::string& next_char)
{
  // Check to see if we set the indentation after the
  string::const_iterator it = next_char.begin();
  int next_codepoint = utf8::next(it, next_char.end());
  if(next_codepoint == 0x300C || next_codepoint == 0x300E ||
     next_codepoint == 0xFF08)
  {
    current_indentation_in_pixels_ = text_insertion_point_x_ + font_size_in_pixels_ +
      x_spacing_;
  }
}

// -----------------------------------------------------------------------

/**
 * @todo Make this pass the \#WINDOW_ATTR color off wile rendering the
 *       waku_backing.
 */
void SDLTextWindow::render(std::ostream* tree)
{
  if(surface_ && isVisible())
  {
    Size surface_size = surface_->size();

    // POINT
    int boxX = boxX1();
    int boxY = boxY1();

    if (tree) {
      *tree << "  Text Window #" << window_num_ << endl;
    }

    if(waku_backing_)
    {
      Size backing_size = waku_backing_->size();
      // COLOUR
      waku_backing_->renderToScreenAsColorMask(
        Rect(Point(0, 0), backing_size),
        Rect(Point(boxX, boxY), backing_size),
        colour_, filter_);

      if (tree) {
        *tree << "    Backing Area: " << Rect(Point(boxX, boxY), backing_size)
              << endl;
      }
    }

    if(waku_main_)
    {
      Size main_size = waku_main_->size();
      waku_main_->renderToScreen(
        Rect(Point(0, 0), main_size), Rect(Point(boxX, boxY), main_size), 255);

      if (tree) {
        *tree << "    Main Area: " << Rect(Point(boxX, boxY), main_size)
              << endl;
      }
    }

    if (waku_button_)
      renderButtons();

    int x = textX1();
    int y = textY1();

    if(inSelectionMode())
    {
      for_each(selections_.begin(), selections_.end(),
               bind(&SelectionElement::render, _1));
    }
    else
    {
      surface_->renderToScreen(
        Rect(Point(0, 0), surface_size),
        Rect(Point(x, y), surface_size),
        255);

      if (tree) {
        *tree << "    Text Area: " << Rect(Point(x, y), surface_size) << endl;
      }
    }
  }
}

// -----------------------------------------------------------------------

void SDLTextWindow::displayRubyText(const std::string& utf8str)
{
  if(ruby_begin_point_ != -1)
  {
    int end_point = text_insertion_point_x_ - x_spacing_;

    if(ruby_begin_point_ > end_point)
    {
      ruby_begin_point_ = -1;
      throw rlvm::Exception("We don't handle ruby across line breaks yet!");
    }

    SDL_Color color;
    RGBColourToSDLColor(font_colour_, &color);
    SDL_Surface* tmp =
      TTF_RenderUTF8_Blended(ruby_font_.get(), utf8str.c_str(), color);

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
}

// -----------------------------------------------------------------------

void SDLTextWindow::addSelectionItem(const std::string& utf8str)
{
  // Render the incoming string for both selected and not-selected.
  SDL_Color color;
  RGBColourToSDLColor(font_colour_, &color);
  SDL_Surface* normal =
    TTF_RenderUTF8_Blended(font_.get(), utf8str.c_str(), color);

  // Copy and invert the surface for whatever.
  SDL_Surface* inverted = AlphaInvert(normal);

  // Figure out xpos and ypos
  // POINT
  SelectionElement* element = new SelectionElement(
      system().graphics(),
      shared_ptr<Surface>(new SDLSurface(normal)),
      shared_ptr<Surface>(new SDLSurface(inverted)),
      selectionCallback(), getNextSelectionID(),
      Point(textX1() + text_insertion_point_x_,
            textY1() + text_insertion_point_y_));

  text_insertion_point_y_ += (font_size_in_pixels_ + y_spacing_ + ruby_size_);
  selections_.push_back(element);
}
