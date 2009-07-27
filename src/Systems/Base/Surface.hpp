// -*- Mode: C++; tab-width:2; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi:tw=80:et:ts=2:sts=2
//
// -----------------------------------------------------------------------
//
// This file is part of RLVM, a RealLive virtual machine clone.
//
// -----------------------------------------------------------------------
//
// Copyright (C) 2007 Elliot Glaysher
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

#ifndef __Surface_hpp__
#define __Surface_hpp__

#include "Systems/Base/Rect.hpp"

#include <boost/enable_shared_from_this.hpp>
#include <boost/shared_ptr.hpp>

class RGBColour;
class RGBAColour;
class GraphicsObject;
struct GraphicsObjectOverride;

/**
 * Abstract concept of a surface.
 */
class Surface : public boost::enable_shared_from_this<Surface>
{
public:
  struct GrpRect {
    Rect rect;

    /// Describes an offset to rect. Why VisualArts threw this in is
    /// unknown.
    int originX, originY;
  };

public:
  Surface();
  virtual ~Surface();

  virtual void setIsMask(const bool is) { }

  virtual Size size() const = 0;
  Rect rect() const;

  virtual void dump();

  /// Blits to another surface
  virtual void blitToSurface(Surface& surface,
                             const Rect& src, const Rect& dst,
                             int alpha = 255, bool use_src_alpha = true) = 0;

  virtual void renderToScreen(
    const Rect& src, const Rect& dst,
    int alpha = 255) = 0;

  virtual void renderToScreenAsColorMask(
    const Rect& src, const Rect& dst, const RGBAColour& colour, int filter) = 0;

  virtual void renderToScreen(
    const Rect& src, const Rect& dst,
    const int opacity[4]) = 0;

  virtual void renderToScreenAsObject(const GraphicsObject& rp,
                                      const Rect& src,
                                      const Rect& dst,
                                      int alpha) = 0;

  virtual int numPatterns() const;
  virtual const GrpRect& getPattern(int patt_no) const;

  virtual void rawRenderQuad(const int src_coords[8],
                             const int dest_coords[8],
                             const int opacity[4]) = 0;

  virtual void fill(const RGBAColour& colour) = 0;
  virtual void fill(const RGBAColour& colour, const Rect& area) = 0;

  virtual void getDCPixel(const Point& pos, int& r, int& g, int& b) = 0;

  virtual boost::shared_ptr<Surface> clipAsColorMask(
    const Rect& clip_rect, int r, int g, int b);

  virtual Surface* clone() const = 0;
};

#endif
