// -*- Mode: C++; tab-width:2; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi:tw=80:et:ts=2:sts=2
//
// -----------------------------------------------------------------------
//
// This file is part of RLVM, a RealLive virtual machine clone.
//
// -----------------------------------------------------------------------
//
// Copyright (C) 2006 Elliot Glaysher
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

#ifndef SRC_SYSTEMS_SDL_SDL_UTILS_H_
#define SRC_SYSTEMS_SDL_SDL_UTILS_H_

#include <SDL2/SDL.h>

#include <string>

class Rect;
class RGBColour;
class RGBAColour;

void ShowGLErrors(void);

inline void DebugShowGLErrors() {
#ifndef SRC_SYSTEMS_SDL_SDL_UTILS_H_
  // Don't do roundtrips to the graphics card for most debugging.
  ShowGLErrors();
#endif  // SRC_SYSTEMS_SDL_SDL_UTILS_H_
}

void reportSDLError(const std::string& sdl_name,
                    const std::string& function_name);

// Whether we can safely use non-power of two textures.
bool IsNPOTSafe();

// Returns the max texture size.
int GetMaxTextureSize();

// Returns a safe size that can contain |i|.
//
// (This function is misused in some places; it can create a texture smaller
// than |i| if GL_MAX_TEXTURE_SIZE is small.)
int SafeSize(int i);

struct SDL_Surface;
SDL_Surface* AlphaInvert(SDL_Surface* in_surface);

void RectToSDLRect(const Rect& rect, SDL_Rect* out);

void RGBColourToSDLColor(const RGBColour& in, SDL_Color* out);
Uint32 MapRGBA(SDL_PixelFormat* fmt, const RGBAColour& in);

void glColorRGBA(const RGBAColour& rgba);

#endif  // SRC_SYSTEMS_SDL_SDL_UTILS_H_
