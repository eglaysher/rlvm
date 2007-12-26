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

#ifndef __Surface_hpp__
#define __Surface_hpp__

class GraphicsObject;
struct GraphicsObjectOverride;

/** 
 * Abstract concept of a surface.
 */
class Surface
{
public:
  struct GrpRect {
    int x1, y1, x2, y2;

    int originX, originY;
  };

public:
  Surface();
  virtual ~Surface();

  virtual int width() const = 0;
  virtual int height() const = 0;

  virtual void dump() {}

  /// Blits to another surface
  virtual void blitToSurface(Surface& surface, 
                             int srcX, int srcY, int srcWidth, int srcHeight,
                             int destX, int destY, int destWidth, int destHeight,
                             int alpha = 255, bool useSrcAlpha = true) { }

  virtual void renderToScreen(
                     int srcX, int srcY, int srcWidth, int srcHeight,
                     int destX, int destY, int destWidth, int destHeight,
                     int alpha = 255) { }

  virtual void renderToScreenAsColorMask(
                     int srcX1, int srcY1, int srcX2, int srcY2,
                     int destX1, int destY1, int destX2, int destY2,
                     int r, int g, int b, int alpha, int filter) { }

  virtual void renderToScreen(
    int srcX1, int srcY1, int srcX2, int srcY2,
    int destX1, int destY1, int destX2, int destY2,
    const int opacity[4]) { }

  virtual void renderToScreenAsObject(const GraphicsObject& rp) {}
  virtual void renderToScreenAsObject(const GraphicsObject& rp, 
                                      const GraphicsObjectOverride& override) {}

  virtual int numPatterns() const { return 1; }
  virtual const GrpRect& getPattern(int pattNo) const 
  {
    static GrpRect rect;
    return rect;
  };

  virtual void rawRenderQuad(const int srcCoords[8], 
                             const int destCoords[8],
                             const int opacity[4]) { }

  virtual void fill(int r, int g, int b, int alpha) = 0;
  virtual void fill(int r, int g, int b, int alpha, int x, int y, 
                    int width, int height) = 0;

  virtual void getDCPixel(int x, int y, int& r, int& g, int& b) { }

  virtual Surface* clone() const = 0;
};

#endif 
