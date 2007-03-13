// This file is part of RLVM, a RealLive virtual machine clone.
//
// -----------------------------------------------------------------------
//
// Copyright (C) 2006 El Riot
//  
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
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

#include "Systems/SDL/SDLRenderToTextureSurface.hpp"
#include "Systems/SDL/SDLUtils.hpp"
#include "Systems/SDL/Texture.hpp"
#include "Systems/SDL/SDLGraphicsSystem.hpp"

#include "Systems/Base/SystemError.hpp"

#include <SDL/SDL.h>

#include <sstream>

using namespace std;

// -----------------------------------------------------------------------
// SDLRenderToTextureSurface
// -----------------------------------------------------------------------

SDLRenderToTextureSurface::SDLRenderToTextureSurface(int w, int h)
  : m_texture(new Texture(render_to_texture(), w, h))
{}

// -----------------------------------------------------------------------

SDLRenderToTextureSurface::~SDLRenderToTextureSurface()
{
}

// -----------------------------------------------------------------------

/**
 * @todo This function doesn't ignore alpha blending when useSrcAlpha
 *       is false; thus, grpOpen and grpMaskOpen are really grpMaskOpen.
 */
void SDLRenderToTextureSurface::blitToSurface(Surface& destSurface,
                               int srcX, int srcY, int srcWidth, int srcHeight,
                               int destX, int destY, int destWidth, int destHeight,
                               int alpha, bool useSrcAlpha)
{
  throw SystemError("Unsupported operation blitToSurface on SDLRenderToTextureSurface!");
}

// -----------------------------------------------------------------------

void SDLRenderToTextureSurface::renderToScreen(
                     int srcX1, int srcY1, int srcX2, int srcY2,
                     int destX1, int destY1, int destX2, int destY2,
                     int opacity)
{
  m_texture->renderToScreen(srcX1, srcY1, srcX2, srcY2,
                            destX1, destY1, destX2, destY2,
                            opacity);
}

// -----------------------------------------------------------------------

void SDLRenderToTextureSurface::renderToScreen(
                     int srcX1, int srcY1, int srcX2, int srcY2,
                     int destX1, int destY1, int destX2, int destY2,
                     const int opacity[4])
{
  m_texture->renderToScreen(srcX1, srcY1, srcX2, srcY2,
                            destX1, destY1, destX2, destY2,
                            opacity);
}

// -----------------------------------------------------------------------

void SDLRenderToTextureSurface::rawRenderQuad(const int srcCoords[8], 
                               const int destCoords[8],
                               const int opacity[4])
{
  m_texture->rawRenderQuad(srcCoords, destCoords, opacity);
}

// -----------------------------------------------------------------------

void SDLRenderToTextureSurface::fill(int r, int g, int b, int alpha)
{
  throw SystemError("Unsupported operation fill on SDLRenderToTextureSurface!");
}

// -----------------------------------------------------------------------

void SDLRenderToTextureSurface::fill(int r, int g, int b, int alpha, int x, int y, 
                      int width, int height)
{
  throw SystemError("Unsupported operation fill on SDLRenderToTextureSurface!");
}

// -----------------------------------------------------------------------

int SDLRenderToTextureSurface::width() const
{
  return m_texture->width();
}

// -----------------------------------------------------------------------

int SDLRenderToTextureSurface::height() const
{
  return m_texture->height();
}

// -----------------------------------------------------------------------

Surface* SDLRenderToTextureSurface::clone() const
{
  throw SystemError("Unsupported operation clone on SDLRenderToTextureSurface!");
}
