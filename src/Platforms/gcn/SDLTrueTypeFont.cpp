// -*- Mode: C++; tab-width:2; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi:tw=80:et:ts=2:sts=2
//
// -----------------------------------------------------------------------
//
// This file is part of RLVM, a RealLive virtual machine clone.
//
// -----------------------------------------------------------------------
//
// Copyright (C) 2008 Elliot Glaysher
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
// -----------------------------------------------------------------------

#include "Precompiled.hpp"

// -----------------------------------------------------------------------

#include "Platforms/gcn/SDLTrueTypeFont.hpp"

#include "Systems/Base/Rect.hpp"
#include "Systems/SDL/SDLSurface.hpp"

#include <guichan/exception.hpp>
#include <guichan/graphics.hpp>
#include <guichan/color.hpp>
#include <guichan/opengl/openglimage.hpp>

#include <boost/shared_ptr.hpp>

#include <iostream>
using namespace std;

// -----------------------------------------------------------------------
// SDLTrueTypeFont
// -----------------------------------------------------------------------
SDLTrueTypeFont::SDLTrueTypeFont (const std::string& filename, int size)
  : image_cache_(125)
{
  mRowSpacing = 0;
  mGlyphSpacing = 0;
  mAntiAlias = true;
  mFilename = filename;
  mFont = NULL;

  mFont = TTF_OpenFont(filename.c_str(), size);

  if (mFont == NULL) {
    throw GCN_EXCEPTION("SDLTrueTypeFont::SDLTrueTypeFont. "+std::string(TTF_GetError()));
  }
}

// -----------------------------------------------------------------------

SDLTrueTypeFont::~SDLTrueTypeFont()
{
  TTF_CloseFont(mFont);
}

// -----------------------------------------------------------------------

int SDLTrueTypeFont::getWidth(const std::string& text) const
{
  int w, h;
  TTF_SizeUTF8(mFont, text.c_str(), &w, &h);

  return w;
}

// -----------------------------------------------------------------------

int SDLTrueTypeFont::getHeight() const
{
  return TTF_FontHeight(mFont) + mRowSpacing;
}

// -----------------------------------------------------------------------

void SDLTrueTypeFont::drawString(gcn::Graphics* graphics,
                                 const std::string& text,
                                 const int x, const int y)
{
  if (text == "")
    return;

  // This is needed for drawing the Glyph in the middle if we have spacing
  int yoffset = getRowSpacing() / 2;

  gcn::Color col = graphics->getColor();
  std::string colstr;
  colstr += char(col.r);
  colstr += char(col.g);
  colstr += char(col.b);

  boost::shared_ptr<gcn::OpenGLImage> image =
    image_cache_.fetch(make_pair(text, colstr));
  if (!image) {
    SDL_Color sdlCol;
    sdlCol.b = col.b;
    sdlCol.r = col.r;
    sdlCol.g = col.g;

    SDL_Surface *textSurface;
    if (mAntiAlias) {
      textSurface = TTF_RenderUTF8_Blended(mFont, text.c_str(), sdlCol);
    } else {
      textSurface = TTF_RenderUTF8_Solid(mFont, text.c_str(), sdlCol);
    }

    SDL_LockSurface(textSurface);
    {
      image.reset(new gcn::OpenGLImage((const unsigned int*)textSurface->pixels,
                                       textSurface->w, textSurface->h));
    }
    SDL_UnlockSurface(textSurface);
    SDL_FreeSurface(textSurface);

    image_cache_.insert(make_pair(text, colstr), image);
  }

  graphics->drawImage(image.get(),
                      0, 0,
                      x, y + yoffset,
                      image->getWidth(),
                      image->getHeight());
}

// -----------------------------------------------------------------------

void SDLTrueTypeFont::setRowSpacing(int spacing)
{
  mRowSpacing = spacing;
}

// -----------------------------------------------------------------------

int SDLTrueTypeFont::getRowSpacing()
{
  return mRowSpacing;
}

// -----------------------------------------------------------------------

void SDLTrueTypeFont::setGlyphSpacing(int spacing)
{
  mGlyphSpacing = spacing;
}

// -----------------------------------------------------------------------

int SDLTrueTypeFont::getGlyphSpacing()
{
  return mGlyphSpacing;
}

// -----------------------------------------------------------------------

void SDLTrueTypeFont::setAntiAlias(bool antiAlias)
{
  mAntiAlias = antiAlias;
}

// -----------------------------------------------------------------------

bool SDLTrueTypeFont::isAntiAlias()
{
  return mAntiAlias;
}
