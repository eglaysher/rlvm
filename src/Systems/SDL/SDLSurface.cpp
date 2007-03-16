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

#include "Systems/SDL/SDLSurface.hpp"
#include "Systems/SDL/SDLUtils.hpp"
#include "Systems/SDL/Texture.hpp"
#include "Systems/SDL/SDLGraphicsSystem.hpp"

#include <SDL/SDL.h>

#include <sstream>
#include "libReallive/defs.h"

using namespace std;
using namespace libReallive;

// -----------------------------------------------------------------------
// SDLSurface
// -----------------------------------------------------------------------

SDLSurface::SDLSurface()
  : m_surface(NULL), m_textureIsValid(false), m_graphicsSystem(NULL)
{}

// -----------------------------------------------------------------------

SDLSurface::SDLSurface(SDL_Surface* surf)
  : m_surface(surf), m_textureIsValid(false), m_graphicsSystem(NULL)
{
  buildRegionTable(surf->w, surf->h);
}

// -----------------------------------------------------------------------

/// Surface that takes ownership of an externally created surface.
SDLSurface::SDLSurface(SDL_Surface* surf, 
                       const vector<SDLSurface::GrpRect>& region_table)
  : m_surface(surf), m_regionTable(region_table),
    m_textureIsValid(false), m_graphicsSystem(NULL)
{}

SDLSurface::SDLSurface(int width, int height)
  : m_textureIsValid(false), m_graphicsSystem(NULL)
{
  allocate(width, height);
  buildRegionTable(width, height);
}

// -----------------------------------------------------------------------

/// Constructor helper function
void SDLSurface::buildRegionTable(int width, int height)
{
  // Build a region table with one entry the size of the surface (This
  // should never need to be used with objects created with this
  // constructor, but let's make sure everything is initialized since
  // it'll happen somehow.)
  SDLSurface::GrpRect rect;
  rect.x1 = 0;
  rect.y1 = 0;
  rect.x2 = width;
  rect.y2 = height;
  m_regionTable.push_back(rect);
}

// -----------------------------------------------------------------------

SDLSurface::~SDLSurface()
{
  deallocate();
}

// -----------------------------------------------------------------------

void SDLSurface::dump()
{
  static int count = 0;
  ostringstream ss;
  ss << "dump_" << count << ".bmp";
  count++;
  SDL_SaveBMP(m_surface, ss.str().c_str());
}

// -----------------------------------------------------------------------

void SDLSurface::allocate(int width, int height)
{
  deallocate();

  // Create an empty surface
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

  SDL_Surface* tmp = 
    SDL_CreateRGBSurface(SDL_HWSURFACE | SDL_SRCALPHA, width, height, 
                         32, 
                         rmask, gmask, bmask, amask);

  if(tmp == NULL)
  {
    ostringstream ss;
    ss << "Couldn't allocate surface in SDLSurface::SDLSurface"
       << ": " << SDL_GetError();
    throw Error(ss.str());
  }

  SDL_Surface* out = SDL_DisplayFormatAlpha(tmp);
  SDL_FreeSurface(tmp);
  
  m_surface = out;

  fill(0, 0, 0, 255);
}

// -----------------------------------------------------------------------

void SDLSurface::allocate(int width, int height, SDLGraphicsSystem* sys)
{
  m_graphicsSystem = sys;
  allocate(width, height);
}

// -----------------------------------------------------------------------

void SDLSurface::deallocate()
{
  m_texture.reset();
  if(m_surface)
  {
    SDL_FreeSurface(m_surface);
    m_surface = NULL;
  }
}

// -----------------------------------------------------------------------

/**
 * @todo This function doesn't ignore alpha blending when useSrcAlpha
 *       is false; thus, grpOpen and grpMaskOpen are really grpMaskOpen.
 * @todo Make this only upload the changed parts of the texture!
 */
void SDLSurface::blitToSurface(Surface& destSurface,
                               int srcX, int srcY, int srcWidth, int srcHeight,
                               int destX, int destY, int destWidth, int destHeight,
                               int alpha, bool useSrcAlpha)
{
  SDLSurface& dest = dynamic_cast<SDLSurface&>(destSurface);

  SDL_Rect srcRect, destRect;
  srcRect.x = srcX;
  srcRect.y = srcY;
  srcRect.w = srcWidth;
  srcRect.h = srcHeight;

  destRect.x = destX;
  destRect.y = destY;
  destRect.w = destWidth;
  destRect.h = destHeight;

  if(useSrcAlpha) 
  {
    if(SDL_SetAlpha(m_surface, SDL_SRCALPHA, alpha))
      reportSDLError("SDL_SetAlpha", "SDLGrpahicsSystem::blitSurfaceToDC()");
  }
  else
  {
    if(SDL_SetAlpha(m_surface, 0, 0))
      reportSDLError("SDL_SetAlpha", "SDLGrpahicsSystem::blitSurfaceToDC()");
  }

  if(SDL_BlitSurface(m_surface, &srcRect, dest.surface(), &destRect))
    reportSDLError("SDL_BlitSurface", "SDLGrpahicsSystem::blitSurfaceToDC()");


  dest.markWrittenTo();
}

// -----------------------------------------------------------------------

void SDLSurface::uploadTextureIfNeeded()
{
  if(!m_textureIsValid)
  {
//    cout << "Uploading texture!" << endl;
    m_texture.reset(new Texture(m_surface));
    m_textureIsValid = true;
  }
}

// -----------------------------------------------------------------------

Texture& SDLSurface::texture()
{
  uploadTextureIfNeeded();
  return *m_texture;
}

// -----------------------------------------------------------------------

void SDLSurface::renderToScreen(
                     int srcX1, int srcY1, int srcX2, int srcY2,
                     int destX1, int destY1, int destX2, int destY2,
                     int alpha)
{
  uploadTextureIfNeeded();

  m_texture->renderToScreen(srcX1, srcY1, srcX2, srcY2,
                            destX1, destY1, destX2, destY2,
                            alpha);
}

// -----------------------------------------------------------------------

void SDLSurface::renderToScreen(
                     int srcX1, int srcY1, int srcX2, int srcY2,
                     int destX1, int destY1, int destX2, int destY2,
                     const int opacity[4])
{
  uploadTextureIfNeeded();

  m_texture->renderToScreen(srcX1, srcY1, srcX2, srcY2,
                            destX1, destY1, destX2, destY2,
                            opacity);
}

// -----------------------------------------------------------------------

void SDLSurface::renderToScreenAsObject(const GraphicsObject& rp)
{
  uploadTextureIfNeeded();
  m_texture->renderToScreenAsObject(rp, *this);
}

// -----------------------------------------------------------------------

void SDLSurface::rawRenderQuad(const int srcCoords[8], 
                               const int destCoords[8],
                               const int opacity[4])
{
  uploadTextureIfNeeded();

  m_texture->rawRenderQuad(srcCoords, destCoords, opacity);
}

// -----------------------------------------------------------------------

void SDLSurface::fill(int r, int g, int b, int alpha)
{
  // Fill the entire surface with the incoming color
  Uint32 color = SDL_MapRGBA(m_surface->format, r, g, b, alpha);

  if(SDL_FillRect(m_surface, NULL, color))
    reportSDLError("SDL_FillRect", "SDLGrpahicsSystem::wipe()");

  // If we are the main screen, then we want to update the screen
  markWrittenTo();
}

// -----------------------------------------------------------------------

void SDLSurface::fill(int r, int g, int b, int alpha, int x, int y, 
                      int width, int height)
{
  // Fill the entire surface with the incoming color
  Uint32 color = SDL_MapRGBA(m_surface->format, r, g, b, alpha);

  SDL_Rect rect;
  rect.x = x;
  rect.y = y;
  rect.w = width;
  rect.h = height;

  if(SDL_FillRect(m_surface, &rect, color))
    reportSDLError("SDL_FillRect", "SDLGrpahicsSystem::wipe()");

  // If we are the main screen, then we want to update the screen
  markWrittenTo();
}

// -----------------------------------------------------------------------

void SDLSurface::markWrittenTo()
{
  // If we are marked as dc0, alert the SDLGraphicsSystem.
  if(m_graphicsSystem) {
    m_graphicsSystem->markScreenAsDirty();
  }

  // Mark that the texture needs reuploading
  m_textureIsValid = false;
}

// -----------------------------------------------------------------------

const SDLSurface::GrpRect& SDLSurface::getPattern(int pattNo) const
{
  return m_regionTable.at(pattNo);
}

// -----------------------------------------------------------------------

Surface* SDLSurface::clone() const
{
  SDL_Surface* tmpSurface = 
    SDL_CreateRGBSurface(m_surface->flags, m_surface->w, m_surface->h, 
                         m_surface->format->BitsPerPixel,
                         m_surface->format->Rmask, m_surface->format->Gmask,
                         m_surface->format->Bmask, m_surface->format->Amask);

  // Disable alpha blending because we're copying onto a blank (and
  // blank alpha!) surface
  if(SDL_SetAlpha(m_surface, 0, 0))
      reportSDLError("SDL_SetAlpha", "SDLGrpahicsSystem::blitSurfaceToDC()");

  if(SDL_BlitSurface(m_surface, NULL, tmpSurface, NULL))
    reportSDLError("SDL_BlitSurface", "SDLSurface::clone()");

  return new SDLSurface(tmpSurface, m_regionTable);
}
