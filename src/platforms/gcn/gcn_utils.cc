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

#include "platforms/gcn/gcn_utils.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#include <guichan/opengl/openglimage.hpp>
#include <guichan/exception.hpp>
#include <guichan/image.hpp>

#include <string>

#include "platforms/gcn/button_disabled.xpm"
#include "platforms/gcn/buttonhi.xpm"
#include "platforms/gcn/buttonpress.xpm"
#include "platforms/gcn/button.xpm"
#include "platforms/gcn/deepbox.xpm"
#include "platforms/gcn/hscroll_left_default.xpm"
#include "platforms/gcn/hscroll_left_pressed.xpm"
#include "platforms/gcn/hscroll_right_default.xpm"
#include "platforms/gcn/hscroll_right_pressed.xpm"
#include "platforms/gcn/vscroll_down_default.xpm"
#include "platforms/gcn/vscroll_down_pressed.xpm"
#include "platforms/gcn/vscroll_grey.xpm"
#include "platforms/gcn/vscroll_up_default.xpm"
#include "platforms/gcn/vscroll_up_pressed.xpm"

// -----------------------------------------------------------------------

char** IMAGE_TABLE[] =  {
  button_disabled,
  buttonhi,
  button,
  buttonpress,
  deepbox,
  hscroll_left_default,
  hscroll_left_pressed,
  hscroll_right_default,
  hscroll_right_pressed,
  vscroll_down_default,
  vscroll_down_pressed,
  vscroll_grey,
  vscroll_up_default,
  vscroll_up_pressed
};

// -----------------------------------------------------------------------

// Copy from SDLImageLoader
static SDL_Surface* convertToStandardFormat(SDL_Surface* surface) {
  Uint32 rmask, gmask, bmask, amask;
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
  rmask = 0xff000000;
  gmask = 0x00ff0000;
  bmask = 0x0000ff00;
  amask = 0x000000ff;
#else
  rmask = 0x000000ff;
  gmask = 0x0000ff00;
  bmask = 0x00ff0000;
  amask = 0xff000000;
#endif

  SDL_Surface* colourSurface =
      SDL_CreateRGBSurface(SDL_SWSURFACE, 0, 0, 32, rmask, gmask, bmask, amask);

  SDL_Surface* tmp = NULL;

  if (colourSurface != NULL) {
    tmp = SDL_ConvertSurface(surface, colourSurface->format, SDL_SWSURFACE);
    SDL_FreeSurface(colourSurface);
  }

  return tmp;
}

// -----------------------------------------------------------------------

static gcn::Image* buildThemeGCNImageFrom(SDL_Surface* loadedSurface) {
  if (loadedSurface == NULL) {
    throw GCN_EXCEPTION(std::string("Unable to theme image"));
  }

  SDL_Surface* surface = convertToStandardFormat(loadedSurface);

  if (surface == NULL) {
    throw GCN_EXCEPTION(std::string("Not enough memory to load theme image"));
  }

  gcn::OpenGLImage* image = new gcn::OpenGLImage(
      (unsigned int*)surface->pixels, surface->w, surface->h, true);
  SDL_FreeSurface(surface);

  return image;
}

// -----------------------------------------------------------------------

gcn::Image* getThemeImage(enum ThemeImage img) {
  SDL_Surface* loadedSurface = IMG_ReadXPMFromArray(IMAGE_TABLE[img]);
  gcn::Image* image = buildThemeGCNImageFrom(loadedSurface);
  SDL_FreeSurface(loadedSurface);

  return image;
}
