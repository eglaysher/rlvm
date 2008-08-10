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

#ifndef __SDLRenderToTextureSurface_hpp__
#define __SDLRenderToTextureSurface_hpp__

#include <boost/scoped_ptr.hpp>

#include "Systems/Base/Surface.hpp"

class Texture;

/**
 * Fake SDLSurface used to
 */
class SDLRenderToTextureSurface : public Surface
{
private:
  /// The SDLTexture which wraps one or more OpenGL textures
  boost::scoped_ptr<Texture> texture_;

public:
  SDLRenderToTextureSurface(const Size& size);
  ~SDLRenderToTextureSurface();

  virtual void dump();

  /// Blits to another surface
  virtual void blitToSurface(Surface& surface,
                             const Rect& src, const Rect& dst,
                             int alpha = 255, bool use_src_alpha = true);

  virtual void renderToScreen(const Rect& src, const Rect& dst, int alpha = 255);

  virtual void renderToScreen(const Rect& src, const Rect& dst,
                              const int opacity[4]);

  virtual void renderToScreenAsColorMask(
    const Rect& src, const Rect& dst, const RGBAColour& rgba, int filter);

  virtual void renderToScreenAsObject(const GraphicsObject& rp);
  virtual void renderToScreenAsObject(const GraphicsObject& rp,
                                      const GraphicsObjectOverride& override);

  virtual void rawRenderQuad(const int src_coords[8],
                             const int dest_coords[8],
                             const int opacity[4]);

  virtual void fill(const RGBAColour& colour);
  virtual void fill(const RGBAColour& colour, const Rect& rect);

  virtual void getDCPixel(const Point& pos, int& r, int& g, int& b);

  virtual Size size() const;

  virtual Surface* clone() const;
};


#endif
