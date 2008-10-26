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


#ifndef __SDLTrueTypeFont_hpp__
#define __SDLTrueTypeFont_hpp__

#include <map>
#include <string>

#include <SDL/SDL_ttf.h>

#include "guichan/color.hpp"
#include "guichan/font.hpp"
#include "guichan/platform.hpp"

#include "lru_cache.hpp"

namespace gcn {
class Graphics;
class OpenGLImage;
};

// -----------------------------------------------------------------------

/**
 * A hacked up version of the SDLTrueTypeFont file in the guichan/contrib/
 * directory. rlvm has an OpenGL out display and already has infrastructure for
 * uploading the output of SDLTrueTypeFonts as textures...
 *
 * SDL True Type Font implementation of Font. It uses the SDL_ttf library
 * to display True Type Fonts with SDL.
 *
 * NOTE: You must initialize the SDL_ttf library before using this
 *       class. Also, remember to call the SDL_ttf libraries quit
 *       function.
 *
 * @author Elliot Glaysher
 * @author Walluce Pinkham
 * @author Olof Naessén
 */
class SDLTrueTypeFont : public gcn::Font
{
public:

  /**
   * Constructor.
   *
   * @param filename the filename of the True Type Font.
   * @param size the size the font should be in.
   */
  SDLTrueTypeFont (const std::string& filename, int size);

  /**
   * Destructor.
   */
  virtual ~SDLTrueTypeFont();

  /**
   * Sets the spacing between rows in pixels. Default is 0 pixels.
   * The spacing can be negative.
   *
   * @param spacing the spacing in pixels.
   */
  virtual void setRowSpacing (int spacing);

  /**
   * Gets the spacing between rows in pixels.
   *
   * @return the spacing.
   */
  virtual int getRowSpacing();

  /**
   * Sets the spacing between letters in pixels. Default is 0 pixels.
   * The spacing can be negative.
   *
   * @param spacing the spacing in pixels.
   */
  virtual void setGlyphSpacing(int spacing);

  /**
   * Gets the spacing between letters in pixels.
   *
   * @return the spacing.
   */
  virtual int getGlyphSpacing();

  /**
   * Sets the use of anti aliasing..
   *
   * @param antaAlias true for use of antia aliasing.
   */
  virtual void setAntiAlias(bool antiAlias);

  /**
   * Checks if anti aliasing is used.
   *
   * @return true if anti aliasing is used.
   */
  virtual bool isAntiAlias();

  // Inherited from Font
  virtual int getWidth(const std::string& text) const;
  virtual int getHeight() const;
  virtual void drawString(gcn::Graphics* graphics, const std::string& text,
                          int x, int y);

private:
  TTF_Font *mFont;

  int mHeight;
  int mGlyphSpacing;
  int mRowSpacing;

  std::string mFilename;
  bool mAntiAlias;

  LRUCache<std::pair<std::string, std::string>,
           boost::shared_ptr<gcn::OpenGLImage> > image_cache_;
};

#endif
