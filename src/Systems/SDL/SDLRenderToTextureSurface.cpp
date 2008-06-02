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
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
//  
// -----------------------------------------------------------------------

#include "Precompiled.hpp"

// -----------------------------------------------------------------------

#include "Systems/SDL/SDLRenderToTextureSurface.hpp"
#include "Systems/SDL/SDLUtils.hpp"
#include "Systems/SDL/Texture.hpp"
#include "Systems/SDL/SDLGraphicsSystem.hpp"

#include "Systems/Base/SystemError.hpp"

#include <SDL/SDL.h>

#include <sstream>

#include "Utilities.h"

using namespace std;

// -----------------------------------------------------------------------
// SDLRenderToTextureSurface
// -----------------------------------------------------------------------

SDLRenderToTextureSurface::SDLRenderToTextureSurface(const Size& size)
  : m_texture(new Texture(render_to_texture(), size.width(), size.height()))
{}

// -----------------------------------------------------------------------

SDLRenderToTextureSurface::~SDLRenderToTextureSurface()
{
}

// -----------------------------------------------------------------------

void SDLRenderToTextureSurface::dump()
{
  cerr << "Attempting to dump a remembered texture." << endl;
}

// -----------------------------------------------------------------------

/**
 * @todo This function doesn't ignore alpha blending when useSrcAlpha
 *       is false; thus, grpOpen and grpMaskOpen are really grpMaskOpen.
 */
void SDLRenderToTextureSurface::blitToSurface(Surface& destSurface,
                                              const Rect& src, const Rect& dst,
                                              int alpha, bool useSrcAlpha)
{
  throw SystemError("Unsupported operation blitToSurface on SDLRenderToTextureSurface!");
}

// -----------------------------------------------------------------------

void SDLRenderToTextureSurface::renderToScreen(
  const Rect& src, const Rect& dst, int opacity)
{
  m_texture->renderToScreen(src, dst, opacity);
}

// -----------------------------------------------------------------------

void SDLRenderToTextureSurface::renderToScreen(
  const Rect& src, const Rect& dst, const int opacity[4])
{
  m_texture->renderToScreen(src, dst, opacity);
}

// -----------------------------------------------------------------------

void SDLRenderToTextureSurface::renderToScreenAsColorMask(
  const Rect& src, const Rect& dst,
  int r, int g, int b, int alpha, int filter)
{
  throw rlvm::Exception(
    "SDLRenderToTextureSurface::renderToScreenAsColorMask unimplemented");
}

// -----------------------------------------------------------------------

void SDLRenderToTextureSurface::renderToScreenAsObject(const GraphicsObject& rp)
{
  throw rlvm::Exception(
    "SDLRenderToTextureSurface::renderToScreenAsObject unimplemented");
}

// -----------------------------------------------------------------------

void SDLRenderToTextureSurface::renderToScreenAsObject(
  const GraphicsObject& rp, 
  const GraphicsObjectOverride& override)
{
  throw rlvm::Exception(
    "SDLRenderToTextureSurface::renderToScreenAsObject unimplemented");
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

void SDLRenderToTextureSurface::fill(int r, int g, int b, int alpha, 
                                     const Rect& rect)
{
  throw SystemError("Unsupported operation fill on SDLRenderToTextureSurface!");
}

// -----------------------------------------------------------------------

void SDLRenderToTextureSurface::getDCPixel(const Point& pos, int& r, int& g, int& b)
{
  throw SystemError("Unsupported operation fill on SDLRenderToTextureSurface!");
}

// -----------------------------------------------------------------------

Size SDLRenderToTextureSurface::size() const
{
  return Size(m_texture->width(), m_texture->height());
}

// -----------------------------------------------------------------------

Surface* SDLRenderToTextureSurface::clone() const
{
  throw SystemError("Unsupported operation clone on SDLRenderToTextureSurface!");
}
