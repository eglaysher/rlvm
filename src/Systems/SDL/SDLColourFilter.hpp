// -*- Mode: C++; tab-width:2; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi:tw=80:et:ts=2:sts=2
//
// -----------------------------------------------------------------------
//
// This file is part of RLVM, a RealLive virtual machine clone.
//
// -----------------------------------------------------------------------
//
// Copyright (C) 2013 Elliot Glaysher
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

#ifndef SRC_SYSTEMS_SDL_SDLCOLOURFILTER_HPP_
#define SRC_SYSTEMS_SDL_SDLCOLOURFILTER_HPP_

#include <SDL/SDL_opengl.h>

#include "Systems/Base/ColourFilter.hpp"
#include "Systems/Base/Rect.hpp"

class RGBAColour;

// OpenGL specific implementation of a ColourFilter.
class SDLColourFilter : public ColourFilter {
 public:
  SDLColourFilter(const Rect& screen_rect);
  virtual ~SDLColourFilter();

  // Overriden from ColourFilter:
  virtual void Fill(const GraphicsObject& go,
                    const RGBAColour& colour);

 private:
  Rect screen_rect_;

  unsigned int texture_width_;
  unsigned int texture_height_;

  GLuint back_texture_id_;
};

#endif  // SRC_SYSTEMS_SDL_SDLCOLOURFILTER_HPP_
