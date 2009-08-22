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
#include "Systems/Base/GraphicsObject.hpp"
#include "Systems/Base/GraphicsObjectData.hpp"

#include "alphablit.h"

#include <iostream>
#include <sstream>
#include <boost/bind.hpp>

using namespace std;
using boost::bind;
using boost::ptr_vector;

// -----------------------------------------------------------------------

SDL_Surface* buildNewSurface(const Size& size) {
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

  if (tmp == NULL) {
    ostringstream ss;
    ss << "Couldn't allocate surface in build_new_surface"
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
  int x, int y, int w, int h, unsigned int bytes_per_pixel,
  int byte_order, int byte_type)
  : texture(new Texture(surface, x, y, w, h, bytes_per_pixel, byte_order,
                        byte_type)),
    x_(x), y_(y), w_(w), h_(h), bytes_per_pixel_(bytes_per_pixel),
    byte_order_(byte_order), byte_type_(byte_type) {}

// -----------------------------------------------------------------------

void SDLSurface::TextureRecord::reupload(SDL_Surface* surface) {
  if (texture) {
    texture->reupload(surface, x_, y_, w_, h_, bytes_per_pixel_, byte_order_,
                      byte_type_);
  } else {
    texture.reset(new Texture(surface, x_, y_, w_, h_, bytes_per_pixel_,
                              byte_order_, byte_type_));
  }
}

// -----------------------------------------------------------------------

void SDLSurface::TextureRecord::forceUnload() {
  texture.reset();
}

// -----------------------------------------------------------------------
// SDLSurface
// -----------------------------------------------------------------------

SDLSurface::SDLSurface(SDLGraphicsSystem* system)
    : surface_(NULL), texture_is_valid_(false), is_dc0_(false),
      graphics_system_(system), is_mask_(false) {
  registerWithGraphicsSystem();
}

// -----------------------------------------------------------------------

SDLSurface::SDLSurface(SDLGraphicsSystem* system, SDL_Surface* surf)
  : surface_(surf), texture_is_valid_(false), is_dc0_(false),
    graphics_system_(system), is_mask_(false) {
  registerWithGraphicsSystem();
  buildRegionTable(Size(surf->w, surf->h));
}

// -----------------------------------------------------------------------

/// Surface that takes ownership of an externally created surface.
SDLSurface::SDLSurface(SDLGraphicsSystem* system, SDL_Surface* surf,
                       const vector<SDLSurface::GrpRect>& region_table)
  : surface_(surf), region_table_(region_table),
    texture_is_valid_(false), is_dc0_(false), graphics_system_(system),
    is_mask_(false) {
  registerWithGraphicsSystem();
}

// -----------------------------------------------------------------------

SDLSurface::SDLSurface(SDLGraphicsSystem* system, const Size& size)
  : surface_(NULL), texture_is_valid_(false), is_dc0_(false),
    graphics_system_(system), is_mask_(false) {
  registerWithGraphicsSystem();
  allocate(size);
  buildRegionTable(size);
}

// -----------------------------------------------------------------------

/// Constructor helper function
void SDLSurface::buildRegionTable(const Size& size) {
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

void SDLSurface::registerWithGraphicsSystem() {
  if (graphics_system_)
    graphics_system_->registerSurface(this);
}

// -----------------------------------------------------------------------

void SDLSurface::unregisterFromGraphicsSystem() {
  if (graphics_system_) {
    graphics_system_->unregisterSurface(this);
    graphics_system_ = NULL;
  }
}

// -----------------------------------------------------------------------

SDLSurface::~SDLSurface() {
  unregisterFromGraphicsSystem();
  deallocate();
}

void SDLSurface::invalidate() {
  // Force unloading of all OpenGL resources
  for (std::vector<TextureRecord>::iterator it = textures_.begin();
       it != textures_.end(); ++it) {
    it->forceUnload();
  }

  texture_is_valid_ = false;
}

// -----------------------------------------------------------------------

Size SDLSurface::size() const { return Size(surface_->w, surface_->h); }

// -----------------------------------------------------------------------

void SDLSurface::dump() {
  static int count = 0;
  ostringstream ss;
  ss << "dump_" << count << ".bmp";
  count++;
  SDL_SaveBMP(surface_, ss.str().c_str());
}

// -----------------------------------------------------------------------

void SDLSurface::allocate(const Size& size) {
  deallocate();

  surface_ = buildNewSurface(size);

  fill(RGBAColour::Black());
}

// -----------------------------------------------------------------------

void SDLSurface::allocate(const Size& size, bool is_dc0) {
  is_dc0_ = is_dc0;
  allocate(size);
}

// -----------------------------------------------------------------------

void SDLSurface::deallocate() {
  textures_.clear();
  if (surface_) {
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
static void stretch(SDL_Surface *src, SDL_Surface *dst) {
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


	switch (src->format->BytesPerPixel) {
	case 1:
		for (looph = 0; looph < dstheight; ++looph) {
			Uint8 *srcpix = (Uint8*)srcrow, *dstpix = (Uint8*)dstrow;
			w_err = srcwidth2 - dstwidth2;
			for (loopw = 0; loopw < dstwidth; ++ loopw) {
				*dstpix++ = *srcpix;
				while (w_err >= 0) {++srcpix; w_err -= dstwidth2;}
				w_err += srcwidth2;
			}
			while (h_err >= 0) {srcrow += srcpitch; h_err -= dstheight2;}
			dstrow += dstpitch;
			h_err += srcheight2;
		}break;
	case 2:
		for (looph = 0; looph < dstheight; ++looph) {
			Uint16 *srcpix = (Uint16*)srcrow, *dstpix = (Uint16*)dstrow;
			w_err = srcwidth2 - dstwidth2;
			for (loopw = 0; loopw < dstwidth; ++ loopw) {
				*dstpix++ = *srcpix;
				while (w_err >= 0) {++srcpix; w_err -= dstwidth2;}
				w_err += srcwidth2;
			}
			while (h_err >= 0) {srcrow += srcpitch; h_err -= dstheight2;}
			dstrow += dstpitch;
			h_err += srcheight2;
		}break;
	case 3:
		for (looph = 0; looph < dstheight; ++looph) {
			Uint8 *srcpix = (Uint8*)srcrow, *dstpix = (Uint8*)dstrow;
			w_err = srcwidth2 - dstwidth2;
			for (loopw = 0; loopw < dstwidth; ++ loopw) {
				dstpix[0] = srcpix[0]; dstpix[1] = srcpix[1]; dstpix[2] = srcpix[2];
				dstpix += 3;
				while (w_err >= 0) {srcpix+=3; w_err -= dstwidth2;}
				w_err += srcwidth2;
			}
			while (h_err >= 0) {srcrow += srcpitch; h_err -= dstheight2;}
			dstrow += dstpitch;
			h_err += srcheight2;
		}break;
	default: /*case 4:*/
		for (looph = 0; looph < dstheight; ++looph) {
			Uint32 *srcpix = (Uint32*)srcrow, *dstpix = (Uint32*)dstrow;
			w_err = srcwidth2 - dstwidth2;
			for (loopw = 0; loopw < dstwidth; ++ loopw) {
				*dstpix++ = *srcpix;
				while (w_err >= 0) {++srcpix; w_err -= dstwidth2;}
				w_err += srcwidth2;
			}
			while (h_err >= 0) {srcrow += srcpitch; h_err -= dstheight2;}
			dstrow += dstpitch;
			h_err += srcheight2;
		}break;
	}
}

// -----------------------------------------------------------------------


/**
 * @todo This function doesn't ignore alpha blending when use_src_alpha
 *       is false; thus, grp_open and grp_mask_open are really grp_mask_open.
 * @todo Make this only upload the changed parts of the texture!
 */
void SDLSurface::blitToSurface(Surface& dest_surface,
                               const Rect& src, const Rect& dst,
                               int alpha, bool use_src_alpha) {
  SDLSurface& sdl_dest_surface = dynamic_cast<SDLSurface&>(dest_surface);

  SDL_Rect src_rect, dest_rect;
  RectToSDLRect(src, &src_rect);
  RectToSDLRect(dst, &dest_rect);

  if (src.size() != dst.size()) {
    // Blit the source rectangle into its own image.
    SDL_Surface* src_image = buildNewSurface(src.size());
    if (pygame_AlphaBlit(surface_, &src_rect, src_image, NULL))
      reportSDLError("SDL_BlitSurface", "SDLGrpahicsSystem::blitSurfaceToDC()");

    SDL_Surface* tmp = buildNewSurface(dst.size());
    stretch(src_image, tmp);

    if (use_src_alpha) {
      if (SDL_SetAlpha(tmp, SDL_SRCALPHA, alpha))
        reportSDLError("SDL_SetAlpha", "SDLGrpahicsSystem::blitSurfaceToDC()");
    } else {
      if (SDL_SetAlpha(tmp, 0, 0))
        reportSDLError("SDL_SetAlpha", "SDLGrpahicsSystem::blitSurfaceToDC()");
    }

    if (SDL_BlitSurface(tmp, NULL, sdl_dest_surface.surface(), &dest_rect))
      reportSDLError("SDL_BlitSurface", "SDLGrpahicsSystem::blitSurfaceToDC()");

    SDL_FreeSurface(tmp);
    SDL_FreeSurface(src_image);
  } else {
    if (use_src_alpha) {
      if (SDL_SetAlpha(surface_, SDL_SRCALPHA, alpha))
        reportSDLError("SDL_SetAlpha", "SDLGrpahicsSystem::blitSurfaceToDC()");
    } else {
      if (SDL_SetAlpha(surface_, 0, 0))
        reportSDLError("SDL_SetAlpha", "SDLGrpahicsSystem::blitSurfaceToDC()");
    }

    if (SDL_BlitSurface(surface_, &src_rect, sdl_dest_surface.surface(),
                       &dest_rect))
      reportSDLError("SDL_BlitSurface", "SDLGrpahicsSystem::blitSurfaceToDC()");

  }
  sdl_dest_surface.markWrittenTo();
}

// -----------------------------------------------------------------------

/**
 * Allows for tight coupling with SDL_ttf. Rethink the existence of
 * this function later.
 */
void SDLSurface::blitFROMSurface(SDL_Surface* src_surface,
                                 const Rect& src, const Rect& dst,
                                 int alpha, bool use_src_alpha) {
  SDL_Rect src_rect, dest_rect;
  RectToSDLRect(src, &src_rect);
  RectToSDLRect(dst, &dest_rect);

  if (use_src_alpha) {
    if (pygame_AlphaBlit(src_surface, &src_rect, surface_, &dest_rect))
      reportSDLError("pygame_AlphaBlit", "SDLGrpahicsSystem::blitSurfaceToDC()");
  } else {
    if (SDL_BlitSurface(src_surface, &src_rect, surface_, &dest_rect))
      reportSDLError("SDL_BlitSurface", "SDLGrpahicsSystem::blitSurfaceToDC()");
  }

  markWrittenTo();
}

// -----------------------------------------------------------------------

static void determineProperties(
  SDL_Surface* surface, bool is_mask,
  GLenum& bytes_per_pixel, GLint& byte_order, GLint& byte_type) {
  SDL_LockSurface(surface); {
    bytes_per_pixel = surface->format->BytesPerPixel;
    byte_order = GL_RGBA;
    byte_type = GL_UNSIGNED_BYTE;

    // Determine the byte order of the surface
    SDL_PixelFormat* format = surface->format;
    if (bytes_per_pixel == 4) {
      // If the order is RGBA...
      if (format->Rmask == 0xFF000000 && format->Amask == 0xFF)
        byte_order = GL_RGBA;
      // OSX's crazy ARGB pixel format
      else if ((format->Amask == 0x0 || format->Amask == 0xFF000000) &&
              format->Rmask == 0xFF0000 &&
              format->Gmask == 0xFF00 && format->Bmask == 0xFF) {
        // This is an insane hack to get around OSX's crazy byte order
        // for alpha on PowerPC. Since there isn't a GL_ARGB type, we
        // need to specify BGRA and then tell the byte type to be
        // reversed order.
        //
        // 20070303: Whoah! Is this the internal format on all
        // platforms!?
        byte_order = GL_BGRA;
        byte_type = GL_UNSIGNED_INT_8_8_8_8_REV;
      } else {
        ios_base::fmtflags f = cerr.flags(ios::hex | ios::uppercase);
        cerr << "Unknown mask: (" << format->Rmask << ", " << format->Gmask
             << ", " << format->Bmask << ", " << format->Amask << ")" << endl;
        cerr.flags(f);
      }
    } else if (bytes_per_pixel == 3) {
      // For now, just assume RGB.
      byte_order = GL_RGB;
      cerr << "Warning: Am I really an RGB Surface? Check Texture::Texture()!"
           << endl;
    } else {
      ostringstream oss;
      oss << "Error loading texture: bytes_per_pixel == " << int(bytes_per_pixel)
          << " and we only handle 3 or 4.";
      throw SystemError(oss.str());
    }

  }
  SDL_UnlockSurface(surface);

  if (is_mask) {
    // Compile shader for use:
    bytes_per_pixel = GL_ALPHA;
  }
}

// -----------------------------------------------------------------------

void SDLSurface::uploadTextureIfNeeded() {
  if (!texture_is_valid_) {
    if (textures_.size() == 0) {
      GLenum bytes_per_pixel;
      GLint byte_order, byte_type;
      determineProperties(surface_, is_mask_, bytes_per_pixel, byte_order,
                          byte_type);

      // ---------------------------------------------------------------------

      // Figure out the optimal way of splitting up the image.
      vector<int> x_pieces, y_pieces;
      x_pieces = segmentPicture(surface_->w);
      y_pieces = segmentPicture(surface_->h);

      int x_offset = 0;
      for (vector<int>::const_iterator it = x_pieces.begin();
          it != x_pieces.end(); ++it) {
        int y_offset = 0;
        for (vector<int>::const_iterator jt = y_pieces.begin();
            jt != y_pieces.end(); ++jt) {
          TextureRecord record(
            surface_, x_offset, y_offset, *it, *jt,  bytes_per_pixel,
            byte_order, byte_type);
          textures_.push_back(record);

          y_offset += *jt;
        }

        x_offset += *it;
      }
    } else {
      // Reupload the textures without reallocating them.
      for_each(textures_.begin(), textures_.end(),
               bind(&TextureRecord::reupload, _1, surface_));
    }

    texture_is_valid_ = true;
  }
}

// -----------------------------------------------------------------------

void SDLSurface::renderToScreen(const Rect& src, const Rect& dst, int alpha) {
  uploadTextureIfNeeded();

  for (vector<TextureRecord>::iterator it = textures_.begin();
      it != textures_.end(); ++it) {
    it->texture->renderToScreen(src, dst, alpha);
  }
}

// -----------------------------------------------------------------------

void SDLSurface::renderToScreenAsColorMask(
  const Rect& src, const Rect& dst, const RGBAColour& rgba, int filter) {
  uploadTextureIfNeeded();

  for (vector<TextureRecord>::iterator it = textures_.begin();
      it != textures_.end(); ++it) {
    it->texture->renderToScreenAsColorMask(src, dst, rgba, filter);
  }
}

// -----------------------------------------------------------------------

void SDLSurface::renderToScreen(const Rect& src, const Rect& dst,
                                const int opacity[4]) {
  uploadTextureIfNeeded();

  for (vector<TextureRecord>::iterator it = textures_.begin();
      it != textures_.end(); ++it) {
    it->texture->renderToScreen(src, dst, opacity);
  }
}

// -----------------------------------------------------------------------

void SDLSurface::renderToScreenAsObject(
  const GraphicsObject& rp, const Rect& src, const Rect& dst, int alpha) {
  uploadTextureIfNeeded();

  for (vector<TextureRecord>::iterator it = textures_.begin();
      it != textures_.end(); ++it) {
    it->texture->renderToScreenAsObject(rp, *this, src, dst, alpha);
  }
}

// -----------------------------------------------------------------------

void SDLSurface::rawRenderQuad(const int src_coords[8],
                               const int dest_coords[8],
                               const int opacity[4]) {
  uploadTextureIfNeeded();

  for (vector<TextureRecord>::iterator it = textures_.begin();
      it != textures_.end(); ++it) {
    it->texture->rawRenderQuad(src_coords, dest_coords, opacity);
  }
}

// -----------------------------------------------------------------------

void SDLSurface::fill(const RGBAColour& colour) {
  // Fill the entire surface with the incoming colour
  Uint32 sdl_colour = MapRGBA(surface_->format, colour);

  if (SDL_FillRect(surface_, NULL, sdl_colour))
    reportSDLError("SDL_FillRect", "SDLGrpahicsSystem::wipe()");

  // If we are the main screen, then we want to update the screen
  markWrittenTo();
}

// -----------------------------------------------------------------------

void SDLSurface::fill(const RGBAColour& colour, const Rect& area) {
  // Fill the entire surface with the incoming colour
  Uint32 sdl_colour = MapRGBA(surface_->format, colour);

  SDL_Rect rect;
  RectToSDLRect(area, &rect);

  if (SDL_FillRect(surface_, &rect, sdl_colour))
    reportSDLError("SDL_FillRect", "SDLGrpahicsSystem::wipe()");

  // If we are the main screen, then we want to update the screen
  markWrittenTo();
}

// -----------------------------------------------------------------------

void SDLSurface::markWrittenTo() {
  // If we are marked as dc0, alert the SDLGraphicsSystem.
  if (is_dc0_ && graphics_system_) {
    graphics_system_->markScreenAsDirty(GUT_DRAW_DC0);
  }

  // Mark that the texture needs reuploading
  texture_is_valid_ = false;
}

// -----------------------------------------------------------------------

int SDLSurface::numPatterns() const {
  return region_table_.size();
}

// -----------------------------------------------------------------------

const SDLSurface::GrpRect& SDLSurface::getPattern(int patt_no) const {
  return region_table_.at(patt_no);
}

// -----------------------------------------------------------------------

Surface* SDLSurface::clone() const {
  SDL_Surface* tmp_surface =
    SDL_CreateRGBSurface(surface_->flags, surface_->w, surface_->h,
                         surface_->format->BitsPerPixel,
                         surface_->format->Rmask, surface_->format->Gmask,
                         surface_->format->Bmask, surface_->format->Amask);

  // Disable alpha blending because we're copying onto a blank (and
  // blank alpha!) surface
  if (SDL_SetAlpha(surface_, 0, 0))
    reportSDLError("SDL_SetAlpha", "SDLGraphicsSystem::blitSurfaceToDC()");

  if (SDL_BlitSurface(surface_, NULL, tmp_surface, NULL))
    reportSDLError("SDL_BlitSurface", "SDLSurface::clone()");

  return new SDLSurface(graphics_system_, tmp_surface, region_table_);
}

// -----------------------------------------------------------------------

/**
 * @todo This scheme may be entirely suboptimal. Needs field testing.
 */
vector<int> SDLSurface::segmentPicture(int size_remainging) {
  static GLint max_texture_size = 0;
  if (max_texture_size == 0)
    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &max_texture_size);

  vector<int> output;
  while (size_remainging > max_texture_size) {
    output.push_back(max_texture_size);
    size_remainging -= max_texture_size;
  }

  while (size_remainging) {
    int ss = SafeSize(size_remainging);
    if (ss > 512) {
      output.push_back(512);
      size_remainging -= 512;
    } else {
      output.push_back(size_remainging);
      size_remainging = 0;
    }
  }

  return output;
}

// -----------------------------------------------------------------------

void SDLSurface::getDCPixel(const Point& pos, int& r, int& g, int& b) {
  SDL_Color colour;
  Uint32 col = 0 ;

  //determine position
  char* p_position = ( char* ) surface_->pixels ;

  //offset by y
  p_position += ( surface_->pitch * pos.y() ) ;

  //offset by x
  p_position += ( surface_->format->BytesPerPixel * pos.x() ) ;

  //copy pixel data
  memcpy ( &col , p_position , surface_->format->BytesPerPixel ) ;

  // Before someone tries to simplify the following four lines,
  // remember that sizeof(int) != sizeof(Uint8).
  SDL_GetRGB ( col , surface_->format , &colour.r , &colour.g , &colour.b ) ;
  r = colour.r;
  g = colour.g;
  b = colour.b;
}

// -----------------------------------------------------------------------

boost::shared_ptr<Surface> SDLSurface::clipAsColorMask(
  const Rect& clip_rect, int r, int g, int b) {
  const char* function_name = "SDLGraphicsSystem::clipAsColorMask()";

  // TODO: This needs to be made exception safe and so does the rest
  // of this file.
  SDL_Surface* tmp_surface =
      SDL_CreateRGBSurface(NULL, surface_->w, surface_->h, 24,
                           0xFF0000, 0xFF00, 0xFF, 0);

  if (!tmp_surface)
    reportSDLError("SDL_CreateRGBSurface", function_name);

  if (SDL_BlitSurface(surface_, NULL, tmp_surface, NULL))
    reportSDLError("SDL_BlitSurface", function_name);

  Uint32 colour = SDL_MapRGB(tmp_surface->format, r, g, b);
  if (SDL_SetColorKey(tmp_surface, SDL_SRCCOLORKEY, colour))
    reportSDLError("SDL_SetAlpha", function_name);

  // The OpenGL pieces don't know what to do an image formatted to
  // (FF0000, FF00, FF, 0), so convert it to a standard RGBA image
  // (and clip to the desired rectangle)
  SDL_Surface* surface = buildNewSurface(clip_rect.size());
  SDL_Rect srcrect;
  RectToSDLRect(clip_rect, &srcrect);
  if (SDL_BlitSurface(tmp_surface, &srcrect, surface, NULL))
    reportSDLError("SDL_BlitSurface", function_name);

  SDL_FreeSurface(tmp_surface);

  return boost::shared_ptr<Surface>(new SDLSurface(graphics_system_, surface));
}
