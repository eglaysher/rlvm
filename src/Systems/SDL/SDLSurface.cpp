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

#include <SDL/SDL.h>

#include "Systems/SDL/SDLSurface.hpp"
#include "Systems/SDL/SDLUtils.hpp"
#include "Systems/SDL/Texture.hpp"
#include "Systems/SDL/SDLGraphicsSystem.hpp"

#include "Systems/Base/Colour.hpp"
#include "Systems/Base/SystemError.hpp"
#include "Systems/Base/GraphicsObjectData.hpp"

#include "alphablit.h"

#include <iostream>
#include <sstream>
#include <boost/bind.hpp>

using namespace std;
using boost::bind;
using boost::ptr_vector;

// -----------------------------------------------------------------------

SDL_Surface* buildNewSurface(const Size& size)
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
    SDL_CreateRGBSurface(
      SDL_HWSURFACE | SDL_SRCALPHA, size.width(), size.height(),
      32, rmask, gmask, bmask, amask);

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
// SDLSurface::TextureRecord
// -----------------------------------------------------------------------
SDLSurface::TextureRecord::TextureRecord(
  SDL_Surface* surface,
  int _x, int _y, int _w, int _h, unsigned int _bytesPerPixel,
  int _byteOrder, int _byteType)
  : texture(new Texture(surface, _x, _y, _w, _h, _bytesPerPixel, _byteOrder,
                        _byteType)),
    x(_x), y(_y), w(_w), h(_h), bytesPerPixel(_bytesPerPixel),
    byteOrder(_byteOrder), byteType(_byteType)
{}

// -----------------------------------------------------------------------

void SDLSurface::TextureRecord::reupload(SDL_Surface* surface)
{
  texture->reupload(surface, x, y, w, h, bytesPerPixel, byteOrder, byteType);
}

// -----------------------------------------------------------------------
// SDLSurface
// -----------------------------------------------------------------------

SDLSurface::SDLSurface()
  : surface_(NULL), texture_is_valid_(false), graphics_system_(NULL),
    is_mask_(false)
{}

// -----------------------------------------------------------------------

SDLSurface::SDLSurface(SDL_Surface* surf)
  : surface_(surf), texture_is_valid_(false), graphics_system_(NULL),
    is_mask_(false)
{
  buildRegionTable(Size(surf->w, surf->h));
}

// -----------------------------------------------------------------------

/// Surface that takes ownership of an externally created surface.
SDLSurface::SDLSurface(SDL_Surface* surf,
                       const vector<SDLSurface::GrpRect>& region_table)
  : surface_(surf), region_table_(region_table),
    texture_is_valid_(false), graphics_system_(NULL),
    is_mask_(false)
{}

// -----------------------------------------------------------------------

SDLSurface::SDLSurface(const Size& size)
  : surface_(NULL), texture_is_valid_(false), graphics_system_(NULL),
    is_mask_(false)
{
  allocate(size);
  buildRegionTable(size);
}

// -----------------------------------------------------------------------

/// Constructor helper function
void SDLSurface::buildRegionTable(const Size& size)
{
  // Build a region table with one entry the size of the surface (This
  // should never need to be used with objects created with this
  // constructor, but let's make sure everything is initialized since
  // it'll happen somehow.)
  SDLSurface::GrpRect rect;
  rect.rect = Rect(Point(0, 0), size);
  rect.originX = 0;
  rect.originY = 0;
  region_table_.push_back(rect);
}

// -----------------------------------------------------------------------

SDLSurface::~SDLSurface()
{
  deallocate();
}

// -----------------------------------------------------------------------

Size SDLSurface::size() const { return Size(surface_->w, surface_->h); }

// -----------------------------------------------------------------------

void SDLSurface::dump()
{
  static int count = 0;
  ostringstream ss;
  ss << "dump_" << count << ".bmp";
  count++;
  SDL_SaveBMP(surface_, ss.str().c_str());
}

// -----------------------------------------------------------------------

void SDLSurface::allocate(const Size& size)
{
  deallocate();

  surface_ = buildNewSurface(size);

  fill(RGBAColour::Black());
}

// -----------------------------------------------------------------------

void SDLSurface::allocate(const Size& size, SDLGraphicsSystem* sys)
{
  graphics_system_ = sys;
  allocate(size);
}

// -----------------------------------------------------------------------

void SDLSurface::deallocate()
{
  textures_.clear();
  if(surface_)
  {
    SDL_FreeSurface(surface_);
    surface_ = NULL;
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
                               const Rect& src, const Rect& dst,
                               int alpha, bool useSrcAlpha)
{
  SDLSurface& dest_surface = dynamic_cast<SDLSurface&>(destSurface);

  SDL_Rect srcRect, destRect;
  RectToSDLRect(src, &srcRect);
  RectToSDLRect(dst, &destRect);

  if(src.size() != dst.size())
  {
    // Blit the source rectangle into its own image.
    SDL_Surface* srcImage = buildNewSurface(src.size());
    if(pygame_AlphaBlit(surface_, &srcRect, srcImage, NULL))
      reportSDLError("SDL_BlitSurface", "SDLGrpahicsSystem::blitSurfaceToDC()");

    SDL_Surface* tmp = buildNewSurface(dst.size());
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

    if(SDL_BlitSurface(tmp, NULL, dest_surface.surface(), &destRect))
      reportSDLError("SDL_BlitSurface", "SDLGrpahicsSystem::blitSurfaceToDC()");

    SDL_FreeSurface(tmp);
    SDL_FreeSurface(srcImage);
  }
  else
  {
    if(useSrcAlpha)
    {
      if(SDL_SetAlpha(surface_, SDL_SRCALPHA, alpha))
        reportSDLError("SDL_SetAlpha", "SDLGrpahicsSystem::blitSurfaceToDC()");
    }
    else
    {
      if(SDL_SetAlpha(surface_, 0, 0))
        reportSDLError("SDL_SetAlpha", "SDLGrpahicsSystem::blitSurfaceToDC()");
    }

    if(SDL_BlitSurface(surface_, &srcRect, dest_surface.surface(), &destRect))
      reportSDLError("SDL_BlitSurface", "SDLGrpahicsSystem::blitSurfaceToDC()");

  }
  dest_surface.markWrittenTo();
}

// -----------------------------------------------------------------------

/**
 * Allows for tight coupling with SDL_ttf. Rethink the existence of
 * this function later.
 */
void SDLSurface::blitFROMSurface(SDL_Surface* srcSurface,
                                 const Rect& src, const Rect& dst,
                                 int alpha, bool useSrcAlpha)
{
  SDL_Rect srcRect, destRect;
  RectToSDLRect(src, &srcRect);
  RectToSDLRect(dst, &destRect);

  if(useSrcAlpha)
  {
    if(pygame_AlphaBlit(srcSurface, &srcRect, surface_, &destRect))
      reportSDLError("pygame_AlphaBlit", "SDLGrpahicsSystem::blitSurfaceToDC()");
  }
  else
  {
    if(SDL_BlitSurface(srcSurface, &srcRect, surface_, &destRect))
      reportSDLError("SDL_BlitSurface", "SDLGrpahicsSystem::blitSurfaceToDC()");
  }

  markWrittenTo();
}

// -----------------------------------------------------------------------

static void determineProperties(
  SDL_Surface* surface, bool isMask,
  GLenum& bytesPerPixel, GLint& byteOrder, GLint& byteType)
{
  SDL_LockSurface(surface);
  {
    bytesPerPixel = surface->format->BytesPerPixel;
    byteOrder = GL_RGBA;
    byteType = GL_UNSIGNED_BYTE;

    // Determine the byte order of the surface
    SDL_PixelFormat* format = surface->format;
    if(bytesPerPixel == 4)
    {
      // If the order is RGBA...
      if(format->Rmask == 0xFF000000 && format->Amask == 0xFF)
        byteOrder = GL_RGBA;
      // OSX's crazy ARGB pixel format
      else if((format->Amask == 0x0 || format->Amask == 0xFF000000) &&
              format->Rmask == 0xFF0000 &&
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

  }
  SDL_UnlockSurface(surface);

  if(isMask)
  {
    // Compile shader for use:
    bytesPerPixel = GL_ALPHA;
  }
}

// -----------------------------------------------------------------------

void SDLSurface::uploadTextureIfNeeded()
{
  if(!texture_is_valid_)
  {
    if(textures_.size() == 0)
    {
      GLenum bytesPerPixel;
      GLint byteOrder, byteType;
      determineProperties(surface_, is_mask_, bytesPerPixel, byteOrder,
                          byteType);

      // ---------------------------------------------------------------------

      // Figure out the optimal way of splitting up the image.
      vector<int> xPieces, yPieces;
      xPieces = segmentPicture(surface_->w);
      yPieces = segmentPicture(surface_->h);

      int xOffset = 0;
      for(vector<int>::const_iterator it = xPieces.begin();
          it != xPieces.end(); ++it)
      {
        int yOffset = 0;
        for(vector<int>::const_iterator jt = yPieces.begin();
            jt != yPieces.end(); ++jt)
        {
          TextureRecord record(
            surface_, xOffset, yOffset, *it, *jt,  bytesPerPixel,
            byteOrder, byteType);
          textures_.push_back(record);

          yOffset += *jt;
        }

        xOffset += *it;
      }
    }
    else
    {
      // Reupload the textures without reallocating them.
      for_each(textures_.begin(), textures_.end(),
               bind(&TextureRecord::reupload, _1, surface_));
    }

    texture_is_valid_ = true;
  }
}

// -----------------------------------------------------------------------

void SDLSurface::renderToScreen(const Rect& src, const Rect& dst, int alpha)
{
  uploadTextureIfNeeded();

  for(vector<TextureRecord>::iterator it = textures_.begin();
      it != textures_.end(); ++it)
  {
    it->texture->renderToScreen(src, dst, alpha);
  }
}

// -----------------------------------------------------------------------

void SDLSurface::renderToScreenAsColorMask(
  const Rect& src, const Rect& dst, const RGBAColour& rgba, int filter)
{
  uploadTextureIfNeeded();

  for(vector<TextureRecord>::iterator it = textures_.begin();
      it != textures_.end(); ++it)
  {
    it->texture->renderToScreenAsColorMask(src, dst, rgba, filter);
  }
}

// -----------------------------------------------------------------------

void SDLSurface::renderToScreen(const Rect& src, const Rect& dst,
                                const int opacity[4])
{
  uploadTextureIfNeeded();

  for(vector<TextureRecord>::iterator it = textures_.begin();
      it != textures_.end(); ++it)
  {
    it->texture->renderToScreen(src, dst, opacity);
  }
}

// -----------------------------------------------------------------------

void SDLSurface::renderToScreenAsObject(const GraphicsObject& rp)
{
  static const GraphicsObjectOverride overrideData;
  uploadTextureIfNeeded();

  for(vector<TextureRecord>::iterator it = textures_.begin();
      it != textures_.end(); ++it)
  {
    it->texture->renderToScreenAsObject(rp, *this, overrideData);
  }
}

// -----------------------------------------------------------------------

void SDLSurface::renderToScreenAsObject(const GraphicsObject& rp,
                                        const GraphicsObjectOverride& override)
{
  uploadTextureIfNeeded();

  for(vector<TextureRecord>::iterator it = textures_.begin();
      it != textures_.end(); ++it)
  {
    it->texture->renderToScreenAsObject(rp, *this, override);
  }
}

// -----------------------------------------------------------------------

void SDLSurface::rawRenderQuad(const int srcCoords[8],
                               const int destCoords[8],
                               const int opacity[4])
{
  uploadTextureIfNeeded();

  for(vector<TextureRecord>::iterator it = textures_.begin();
      it != textures_.end(); ++it)
  {
    it->texture->rawRenderQuad(srcCoords, destCoords, opacity);
  }
}

// -----------------------------------------------------------------------

void SDLSurface::fill(const RGBAColour& colour)
{
  // Fill the entire surface with the incoming color
  Uint32 color = MapRGBA(surface_->format, colour);

  if(SDL_FillRect(surface_, NULL, color))
    reportSDLError("SDL_FillRect", "SDLGrpahicsSystem::wipe()");

  // If we are the main screen, then we want to update the screen
  markWrittenTo();
}

// -----------------------------------------------------------------------

void SDLSurface::fill(const RGBAColour& colour, const Rect& area)
{
  // Fill the entire surface with the incoming color
  Uint32 color = MapRGBA(surface_->format, colour);

  SDL_Rect rect;
  RectToSDLRect(area, &rect);

  if(SDL_FillRect(surface_, &rect, color))
    reportSDLError("SDL_FillRect", "SDLGrpahicsSystem::wipe()");

  // If we are the main screen, then we want to update the screen
  markWrittenTo();
}

// -----------------------------------------------------------------------

void SDLSurface::markWrittenTo()
{
  // If we are marked as dc0, alert the SDLGraphicsSystem.
  if(graphics_system_) {
    graphics_system_->markScreenAsDirty(GUT_DRAW_DC0);
  }

  // Mark that the texture needs reuploading
  texture_is_valid_ = false;
}

// -----------------------------------------------------------------------

int SDLSurface::numPatterns() const
{
  return region_table_.size();
}

// -----------------------------------------------------------------------

const SDLSurface::GrpRect& SDLSurface::getPattern(int pattNo) const
{
  return region_table_.at(pattNo);
}

// -----------------------------------------------------------------------

Surface* SDLSurface::clone() const
{
  SDL_Surface* tmpSurface =
    SDL_CreateRGBSurface(surface_->flags, surface_->w, surface_->h,
                         surface_->format->BitsPerPixel,
                         surface_->format->Rmask, surface_->format->Gmask,
                         surface_->format->Bmask, surface_->format->Amask);

  // Disable alpha blending because we're copying onto a blank (and
  // blank alpha!) surface
  if(SDL_SetAlpha(surface_, 0, 0))
    reportSDLError("SDL_SetAlpha", "SDLGraphicsSystem::blitSurfaceToDC()");

  if(SDL_BlitSurface(surface_, NULL, tmpSurface, NULL))
    reportSDLError("SDL_BlitSurface", "SDLSurface::clone()");

  return new SDLSurface(tmpSurface, region_table_);
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

void SDLSurface::getDCPixel(const Point& pos, int& r, int& g, int& b)
{
  SDL_Color color;
  Uint32 col = 0 ;

  //determine position
  char* pPosition = ( char* ) surface_->pixels ;

  //offset by y
  pPosition += ( surface_->pitch * pos.y() ) ;

  //offset by x
  pPosition += ( surface_->format->BytesPerPixel * pos.x() ) ;

  //copy pixel data
  memcpy ( &col , pPosition , surface_->format->BytesPerPixel ) ;

  // Before someone tries to simplify the following four lines,
  // remember that sizeof(int) != sizeof(Uint8).
  SDL_GetRGB ( col , surface_->format , &color.r , &color.g , &color.b ) ;
  r = color.r;
  g = color.g;
  b = color.b;
}

// -----------------------------------------------------------------------

boost::shared_ptr<Surface> SDLSurface::clipAsColorMask(
  const Rect& clipRect, int r, int g, int b)
{
  const char* functionName = "SDLGraphicsSystem::clipAsColorMask()";

  // TODO: This needs to be made exception safe and so does the rest
  // of this file.
  SDL_Surface* tmpSurface =
    SDL_CreateRGBSurface(surface_->flags & ~SDL_SRCALPHA,
                         surface_->w, surface_->h,
                         surface_->format->BitsPerPixel,
                         surface_->format->Rmask, surface_->format->Gmask,
                         surface_->format->Bmask, 0);
  if(!tmpSurface)
    reportSDLError("SDL_CreateRGBSurface", functionName);

  if(SDL_BlitSurface(surface_, NULL, tmpSurface, NULL))
    reportSDLError("SDL_BlitSurface", functionName);

  Uint32 color = SDL_MapRGB(tmpSurface->format, r, g, b);
  if(SDL_SetColorKey(tmpSurface, SDL_SRCCOLORKEY, color))
    reportSDLError("SDL_SetAlpha", functionName);

  // The OpenGL pieces don't know what to do an image formatted to
  // (FF0000, FF00, FF, 0), so convert it to a standard RGBA image
  // (and clip to the desired rectangle)
  SDL_Surface* surface = buildNewSurface(clipRect.size());
  SDL_Rect srcrect;
  RectToSDLRect(clipRect, &srcrect);
  if(SDL_BlitSurface(tmpSurface, &srcrect, surface, NULL))
    reportSDLError("SDL_BlitSurface", functionName);

  SDL_FreeSurface(tmpSurface);

  return boost::shared_ptr<Surface>(new SDLSurface(surface));
}
