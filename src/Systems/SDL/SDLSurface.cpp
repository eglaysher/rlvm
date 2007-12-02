// This file is part of RLVM, a RealLive virtual machine clone.
//
// -----------------------------------------------------------------------
//
// Copyright (C) 2006, 2007 Elliot Glaysher
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

#include "Precompiled.hpp"

// -----------------------------------------------------------------------

#include "Systems/SDL/SDLSurface.hpp"
#include "Systems/SDL/SDLUtils.hpp"
#include "Systems/SDL/Texture.hpp"
#include "Systems/SDL/SDLGraphicsSystem.hpp"

#include "Systems/Base/SystemError.hpp"
#include "Systems/Base/GraphicsObjectData.hpp"

#include <SDL/SDL.h>
#include "alphablit.h"

#include <iostream>
#include <sstream>

using namespace std;

using boost::ptr_vector;

// -----------------------------------------------------------------------

SDL_Surface* buildNewSurface(int width, int height)
{
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
    ss << "Couldn't allocate surface in buildNewSurface"
       << ": " << SDL_GetError();
    throw SystemError(ss.str());
  }

  SDL_Surface* out = SDL_DisplayFormatAlpha(tmp);
  SDL_FreeSurface(tmp);

  return out;
}


// -----------------------------------------------------------------------
// SDLSurface
// -----------------------------------------------------------------------

SDLSurface::SDLSurface()
  : m_surface(NULL), m_textureIsValid(false), m_graphicsSystem(NULL),
    m_isMask(false)
{}

// -----------------------------------------------------------------------

SDLSurface::SDLSurface(SDL_Surface* surf)
  : m_surface(surf), m_textureIsValid(false), m_graphicsSystem(NULL),
    m_isMask(false)
{
  buildRegionTable(surf->w, surf->h);
}

// -----------------------------------------------------------------------

/// Surface that takes ownership of an externally created surface.
SDLSurface::SDLSurface(SDL_Surface* surf, 
                       const vector<SDLSurface::GrpRect>& region_table)
  : m_surface(surf), m_regionTable(region_table),
    m_textureIsValid(false), m_graphicsSystem(NULL),
    m_isMask(false)
{}

// -----------------------------------------------------------------------

SDLSurface::SDLSurface(int width, int height)
  : m_surface(NULL), m_textureIsValid(false), m_graphicsSystem(NULL),
    m_isMask(false)
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
  rect.originX = 0;
  rect.originY = 0;
  m_regionTable.push_back(rect);
}

// -----------------------------------------------------------------------

SDLSurface::~SDLSurface()
{
  deallocate();
}

// -----------------------------------------------------------------------

int SDLSurface::width() const { return m_surface->w; }
int SDLSurface::height() const { return m_surface->h; }

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

  m_surface = buildNewSurface(width, height);

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
  m_textures.clear();
  if(m_surface)
  {
    SDL_FreeSurface(m_surface);
    m_surface = NULL;
  }
}

// -----------------------------------------------------------------------

/** 
 * Stretches out an image.
 *
 * @author Pete Shinners, Taken from pygame
 * @license LGPL
 */
static void stretch(SDL_Surface *src, SDL_Surface *dst)
{
	int looph, loopw;
	
	Uint8* srcrow = (Uint8*)src->pixels;
	Uint8* dstrow = (Uint8*)dst->pixels;

	int srcpitch = src->pitch;
	int dstpitch = dst->pitch;

	int dstwidth = dst->w;
	int dstheight = dst->h;
	int dstwidth2 = dst->w << 1;
	int dstheight2 = dst->h << 1;

	int srcwidth2 = src->w << 1;
	int srcheight2 = src->h << 1;

	int w_err, h_err = srcheight2 - dstheight2;


	switch(src->format->BytesPerPixel)
	{
	case 1:
		for(looph = 0; looph < dstheight; ++looph)
		{
			Uint8 *srcpix = (Uint8*)srcrow, *dstpix = (Uint8*)dstrow;
			w_err = srcwidth2 - dstwidth2;
			for(loopw = 0; loopw < dstwidth; ++ loopw)
			{
				*dstpix++ = *srcpix;
				while(w_err >= 0) {++srcpix; w_err -= dstwidth2;}
				w_err += srcwidth2;
			}
			while(h_err >= 0) {srcrow += srcpitch; h_err -= dstheight2;}
			dstrow += dstpitch;
			h_err += srcheight2;
		}break;
	case 2:
		for(looph = 0; looph < dstheight; ++looph)
		{
			Uint16 *srcpix = (Uint16*)srcrow, *dstpix = (Uint16*)dstrow;
			w_err = srcwidth2 - dstwidth2;
			for(loopw = 0; loopw < dstwidth; ++ loopw)
			{
				*dstpix++ = *srcpix;
				while(w_err >= 0) {++srcpix; w_err -= dstwidth2;}
				w_err += srcwidth2;
			}
			while(h_err >= 0) {srcrow += srcpitch; h_err -= dstheight2;}
			dstrow += dstpitch;
			h_err += srcheight2;
		}break;
	case 3:
		for(looph = 0; looph < dstheight; ++looph)
		{
			Uint8 *srcpix = (Uint8*)srcrow, *dstpix = (Uint8*)dstrow;
			w_err = srcwidth2 - dstwidth2;
			for(loopw = 0; loopw < dstwidth; ++ loopw)
			{
				dstpix[0] = srcpix[0]; dstpix[1] = srcpix[1]; dstpix[2] = srcpix[2];
				dstpix += 3;
				while(w_err >= 0) {srcpix+=3; w_err -= dstwidth2;}
				w_err += srcwidth2;
			}
			while(h_err >= 0) {srcrow += srcpitch; h_err -= dstheight2;}
			dstrow += dstpitch;
			h_err += srcheight2;
		}break;
	default: /*case 4:*/
		for(looph = 0; looph < dstheight; ++looph)
		{
			Uint32 *srcpix = (Uint32*)srcrow, *dstpix = (Uint32*)dstrow;
			w_err = srcwidth2 - dstwidth2;
			for(loopw = 0; loopw < dstwidth; ++ loopw)
			{
				*dstpix++ = *srcpix;
				while(w_err >= 0) {++srcpix; w_err -= dstwidth2;}
				w_err += srcwidth2;
			}
			while(h_err >= 0) {srcrow += srcpitch; h_err -= dstheight2;}
			dstrow += dstpitch;
			h_err += srcheight2;
		}break;
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

  if(srcWidth != destWidth || srcHeight != destHeight)
  {
    // Blit the source rectangle into its own image.
    SDL_Surface* srcImage = buildNewSurface(srcWidth, srcHeight);
    if(pygame_AlphaBlit(m_surface, &srcRect, srcImage, NULL))
      reportSDLError("SDL_BlitSurface", "SDLGrpahicsSystem::blitSurfaceToDC()");

    SDL_Surface* tmp = buildNewSurface(destWidth, destHeight);
    stretch(srcImage, tmp);

    if(useSrcAlpha) 
    {
      if(SDL_SetAlpha(tmp, SDL_SRCALPHA, alpha))
        reportSDLError("SDL_SetAlpha", "SDLGrpahicsSystem::blitSurfaceToDC()");
    }
    else
    {
      if(SDL_SetAlpha(tmp, 0, 0))
        reportSDLError("SDL_SetAlpha", "SDLGrpahicsSystem::blitSurfaceToDC()");
    }

    if(SDL_BlitSurface(tmp, NULL, dest.surface(), &destRect))
      reportSDLError("SDL_BlitSurface", "SDLGrpahicsSystem::blitSurfaceToDC()");

    SDL_FreeSurface(tmp);
    SDL_FreeSurface(srcImage);
  }
  else
  {
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

  }
  dest.markWrittenTo();
}

// -----------------------------------------------------------------------

/**
 * Allows for tight coupling with SDL_ttf. Rethink the existence of
 * this function later.
 */
void SDLSurface::blitFROMSurface(SDL_Surface* srcSurface,
                                 int srcX, int srcY, int srcWidth, int srcHeight,
                                 int destX, int destY, int destWidth, int destHeight,
                                 int alpha, bool useSrcAlpha)
{
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
//     if(SDL_SetAlpha(srcSurface, SDL_SRCALPHA, alpha))
//       reportSDLError("SDL_SetAlpha", "SDLGrpahicsSystem::blitSurfaceToDC()");
//     if(SDL_SetAlpha(m_surface, SDL_SRCALPHA, alpha))
//       reportSDLError("SDL_SetAlpha", "SDLGrpahicsSystem::blitSurfaceToDC()");

    if(pygame_AlphaBlit(srcSurface, &srcRect, m_surface, &destRect))
      reportSDLError("pygame_AlphaBlit", "SDLGrpahicsSystem::blitSurfaceToDC()");
  }
  else
  {
    if(SDL_BlitSurface(srcSurface, &srcRect, m_surface, &destRect))
      reportSDLError("SDL_BlitSurface", "SDLGrpahicsSystem::blitSurfaceToDC()");

//     if(SDL_SetAlpha(srcSurface, 0, 0))
//       reportSDLError("SDL_SetAlpha", "SDLGrpahicsSystem::blitSurfaceToDC()");
  }

  markWrittenTo();
}                                 

// -----------------------------------------------------------------------

void SDLSurface::uploadTextureIfNeeded()
{
  if(!m_textureIsValid)
  {
    m_textures.clear();

    GLenum bytesPerPixel;
    GLint byteOrder, byteType;
    SDL_LockSurface(m_surface);
    {
      bytesPerPixel = m_surface->format->BytesPerPixel;
      byteOrder = GL_RGBA;
      byteType = GL_UNSIGNED_BYTE;

      // Determine the byte order of the surface
      SDL_PixelFormat* format = m_surface->format;
      if(bytesPerPixel == 4)
      {
        // If the order is RGBA...
        if(format->Rmask == 0xFF000000 && format->Amask == 0xFF)
          byteOrder = GL_RGBA;
        // OSX's crazy ARGB pixel format
        else if(format->Amask == 0xFF000000 && format->Rmask == 0xFF0000 &&
                format->Gmask == 0xFF00 && format->Bmask == 0xFF)
        {
          // This is an insane hack to get around OSX's crazy byte order
          // for alpha on PowerPC. Since there isn't a GL_ARGB type, we
          // need to specify BGRA and then tell the byte type to be
          // reversed order.
          //
          // 20070303: Whoah! Is this the internal format on all
          // platforms!?
          byteOrder = GL_BGRA;
          byteType = GL_UNSIGNED_INT_8_8_8_8_REV;
        }
        else 
        {
          ios_base::fmtflags f = cerr.flags(ios::hex | ios::uppercase);
          cerr << "Unknown mask: (" << format->Rmask << ", " << format->Gmask
               << ", " << format->Bmask << ", " << format->Amask << ")" << endl;
          cerr.flags(f);
        }
      }
      else if(bytesPerPixel == 3)
      {
        // For now, just assume RGB.
        byteOrder = GL_RGB;
        cerr << "Warning: Am I really an RGB Surface? Check Texture::Texture()!"
             << endl;
      }
      else
      {
        ostringstream oss;
        oss << "Error loading texture: bytesPerPixel == " << int(bytesPerPixel)
            << " and we only handle 3 or 4.";
        throw SystemError(oss.str());
      }

      if(m_isMask)
      {
        // Compile shader for use:
        bytesPerPixel = GL_ALPHA;
      }
    }
    SDL_UnlockSurface(m_surface);

    // ---------------------------------------------------------------------

    // Figure out the optimal way of splitting up the image.
    vector<int> xPieces, yPieces;
    xPieces = segmentPicture(m_surface->w);
    yPieces = segmentPicture(m_surface->h);

    int xOffset = 0;
    for(vector<int>::const_iterator it = xPieces.begin(); 
        it != xPieces.end(); ++it)
    {
      int yOffset = 0;
      for(vector<int>::const_iterator jt = yPieces.begin();
          jt != yPieces.end(); ++jt)
      {
        m_textures.push_back(
          new Texture(m_surface,
            xOffset, yOffset, *it, *jt,  bytesPerPixel, byteOrder, byteType));

        yOffset += *jt;
      }

      xOffset += *it;
    }

    m_textureIsValid = true;
  }
}

// -----------------------------------------------------------------------

void SDLSurface::renderToScreen(
                     int srcX1, int srcY1, int srcX2, int srcY2,
                     int destX1, int destY1, int destX2, int destY2,
                     int alpha)
{
  uploadTextureIfNeeded();

  for(ptr_vector<Texture>::iterator it = m_textures.begin();
      it != m_textures.end(); ++it)
  {
    it->renderToScreen(srcX1, srcY1, srcX2, srcY2,
                       destX1, destY1, destX2, destY2,
                       alpha);
  }
}

// -----------------------------------------------------------------------

void SDLSurface::renderToScreenAsColorMask(
                     int srcX1, int srcY1, int srcX2, int srcY2,
                     int destX1, int destY1, int destX2, int destY2,
                     int r, int g, int b, int alpha, int filter)
{
  uploadTextureIfNeeded();

  for(ptr_vector<Texture>::iterator it = m_textures.begin();
      it != m_textures.end(); ++it)
  {
    it->renderToScreenAsColorMask(srcX1, srcY1, srcX2, srcY2,
                                  destX1, destY1, destX2, destY2, 
                                  r, g, b, alpha, filter);
  }
}

// -----------------------------------------------------------------------

void SDLSurface::renderToScreen(
                     int srcX1, int srcY1, int srcX2, int srcY2,
                     int destX1, int destY1, int destX2, int destY2,
                     const int opacity[4])
{
  uploadTextureIfNeeded();

  for(ptr_vector<Texture>::iterator it = m_textures.begin();
      it != m_textures.end(); ++it)
  {
    it->renderToScreen(srcX1, srcY1, srcX2, srcY2,
                       destX1, destY1, destX2, destY2,
                       opacity);
  }
}

// -----------------------------------------------------------------------

void SDLSurface::renderToScreenAsObject(const GraphicsObject& rp)
{
  static const GraphicsObjectOverride overrideData;
  uploadTextureIfNeeded();

  for(ptr_vector<Texture>::iterator it = m_textures.begin();
      it != m_textures.end(); ++it)
  {
    it->renderToScreenAsObject(rp, *this, overrideData);
  }
}

// -----------------------------------------------------------------------

void SDLSurface::renderToScreenAsObject(const GraphicsObject& rp, 
                                        const GraphicsObjectOverride& override)
{
  uploadTextureIfNeeded();

  for(ptr_vector<Texture>::iterator it = m_textures.begin();
      it != m_textures.end(); ++it)
  {
    it->renderToScreenAsObject(rp, *this, override);
  }  
}

// -----------------------------------------------------------------------

void SDLSurface::rawRenderQuad(const int srcCoords[8], 
                               const int destCoords[8],
                               const int opacity[4])
{
  uploadTextureIfNeeded();

  for(ptr_vector<Texture>::iterator it = m_textures.begin();
      it != m_textures.end(); ++it)
  {
    it->rawRenderQuad(srcCoords, destCoords, opacity);
  }
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

int SDLSurface::numPatterns() const
{
  return m_regionTable.size();
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
      reportSDLError("SDL_SetAlpha", "SDLGraphicsSystem::blitSurfaceToDC()");

  if(SDL_BlitSurface(m_surface, NULL, tmpSurface, NULL))
    reportSDLError("SDL_BlitSurface", "SDLSurface::clone()");

  return new SDLSurface(tmpSurface, m_regionTable);
}

// -----------------------------------------------------------------------

/** 
 * @todo This scheme may be entirely suboptimal. Needs field testing.
 */
vector<int> SDLSurface::segmentPicture(int sizeRemainging)
{
  static GLint maxTextureSize = 0;
  if(maxTextureSize == 0)
    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxTextureSize);

  vector<int> output;
  while(sizeRemainging > maxTextureSize) 
  {
    output.push_back(maxTextureSize);
    sizeRemainging -= maxTextureSize;
  }

  while(sizeRemainging)
  {
    int ss = SafeSize(sizeRemainging);
    if(ss > 512)
    {
      output.push_back(512);
      sizeRemainging -= 512;
    }
    else
    {
      output.push_back(sizeRemainging);
      sizeRemainging = 0;
    }
  }

  return output;
}

// -----------------------------------------------------------------------

void SDLSurface::getDCPixel(int x, int y, int& r, int& g, int& b)
{
  SDL_Color color;
  Uint32 col = 0 ;

  //determine position
  char* pPosition = ( char* ) m_surface->pixels ;

  //offset by y
  pPosition += ( m_surface->pitch * y ) ;

  //offset by x
  pPosition += ( m_surface->format->BytesPerPixel * x ) ;

  //copy pixel data
  memcpy ( &col , pPosition , m_surface->format->BytesPerPixel ) ;

  // Before someone tries to simplify the following four lines,
  // remember that sizeof(int) != sizeof(Uint8).
  SDL_GetRGB ( col , m_surface->format , &color.r , &color.g , &color.b ) ;
  r = color.r;
  g = color.g;
  b = color.b;
}
