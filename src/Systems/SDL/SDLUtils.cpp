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

#include "Systems/SDL/SDLUtils.hpp"
#include "Systems/Base/SystemError.hpp"
#include "Systems/Base/Rect.hpp"

#include <SDL/SDL.h>
#include <SDL/SDL_opengl.h>

#include <sstream>

using namespace std;

// -----------------------------------------------------------------------

void ShowGLErrors(void)
{
  GLenum error;
  const GLubyte* errStr;
  if ((error = glGetError()) != GL_NO_ERROR)
  {
    errStr = gluErrorString(error);
    abort();
    ostringstream oss;
    oss << "OpenGL Error: " << (char*)errStr;
    throw SystemError(oss.str());
  }
}

// -----------------------------------------------------------------------

int SafeSize(int i)
{
  static GLint maxTextureSize = 0;
  if(maxTextureSize == 0)
    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxTextureSize);
  int p;

  if (i > maxTextureSize) return maxTextureSize;

  for (p = 0; p < 24; p++)
    if (i <= (1<<p))
      return 1<<p;

  return maxTextureSize;
}

// -----------------------------------------------------------------------

void reportSDLError(const std::string& sdlName,
                    const std::string& functionName)
{
  ostringstream ss;
  ss << "Error while calling SDL function '" << sdlName << "' in "
     << functionName << ": " << SDL_GetError();
  throw SystemError(ss.str());
}

// -----------------------------------------------------------------------

/// @todo This is not endian safe in any way.
SDL_Surface* AlphaInvert(SDL_Surface* inSurface)
{
  SDL_PixelFormat* format = inSurface->format;

  if(format->BitsPerPixel != 32)
    throw SystemError("AlphaInvert requires an alpha channel!");

  // Build a copy of the surface
  SDL_Surface* dst = SDL_AllocSurface(
    inSurface->flags, inSurface->w, inSurface->h,
    format->BitsPerPixel, format->Rmask,
    format->Gmask, format->Bmask, 0);

  SDL_BlitSurface(inSurface, NULL, dst, NULL);

  // iterate over the copy and make the alpha value = 255 - alpha value.
  if(SDL_MUSTLOCK(dst)) SDL_LockSurface(dst);
  {
    int numPixels = dst->h * dst->pitch;
    char* pData = (char*)dst->pixels;

    for(int i = 0; i < numPixels; i += 4)
    {
      // Invert the pixel here.
      pData[i] = 255;
      pData[i + 1] = 255;
      pData[i + 2] = 255;
      pData[i + 3] = 255 - pData[i +3];
    }
  }
  if(SDL_MUSTLOCK(dst)) SDL_UnlockSurface(dst);

  return dst;
}

// -----------------------------------------------------------------------

void RectToSDLRect(const Rect& rect, SDL_Rect* out)
{
  out->x = rect.x();
  out->y = rect.y();
  out->w = rect.width();
  out->h = rect.height();
}
