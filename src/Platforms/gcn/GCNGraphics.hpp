// -*- Mode: C++; tab-width:2; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi:tw=80:et:ts=2:sts=2
//
// -----------------------------------------------------------------------
//
// This file is part of RLVM, a RealLive virtual machine clone.
//
// -----------------------------------------------------------------------
//
// Copyright (C) 2008 Elliot Glaysher
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
// -----------------------------------------------------------------------

#ifndef __GCNGraphics_hpp__
#define __GCNGraphics_hpp__

#include <boost/shared_ptr.hpp>
#include <guichan/opengl/openglgraphics.hpp>
#include <guichan/image.hpp>

#include "Systems/Base/Rect.hpp"

/**
 * 9 rectangles in an image. 4 corners, 4 sides and a middle area. The
 * topology is as follows:
 *
 * <pre>
 *  !-----!-----------------!-----!
 *  !  0  !        1        !  2  !
 *  !-----!-----------------!-----!
 *  !  3  !        4        !  5  !
 *  !-----!-----------------!-----!
 *  !  6  !        7        !  8  !
 *  !-----!-----------------!-----!
 * </pre>
 *
 * Sections 0, 2, 6 and 8 will remain as is. 1, 3, 4, 5 and 7 will be
 * repeated to fit the size of the widget.
 *
 * Originally from The Mana World; modified to use Guichan's standard image
 * class, and to only keep one image and store regions into it.
 */
struct ImageRect {
  boost::shared_ptr<gcn::Image> image;

  /**
   * Initializes all of the Rect classes based on two four-tuples representing
   * the corner coordinates.
   */
  void setCoordinates(const int xpos[], const int ypos[]);

  Rect rect[9];

  /// Symbolic access to the various parts.
  const Rect& topLeft() const { return rect[0]; }
  const Rect& top() const { return rect[1]; }
  const Rect& topRight() const { return rect[2]; }
  const Rect& left() const { return rect[3]; }
  const Rect& center() const { return rect[4]; }
  const Rect& right() const { return rect[5]; }
  const Rect& bottomLeft() const { return rect[6]; }
  const Rect& bottom() const { return rect[7]; }
  const Rect& bottomRight() const { return rect[8]; }
};

/**
 * Custom Grpahics with extra draw primatives used within the widget code that
 * I'm taking from TMW.
 *
 * The basic idea is taken from The Mana World; this is the same interface they
 * expose in their drawing code. The difference here is that I'm stretching
 * widgets with anti-aliasing instead of mirroring their internals back and
 * forth.
 */
class GCNGraphics : public gcn::OpenGLGraphics {
public:
  /// Initializes the graphics with the current screen resolution.
  GCNGraphics(int width, int height);

  ~GCNGraphics();

  /**
   * Draws a rectangle using images. 4 corner images, 4 side images and 1
   * image for the inside.
   */
  void drawImageRect(int x, int y, int w, int h, const ImageRect &imgRect);

private:
  /// Hack around OpenGLGraphics::drawImage to render the image (stretched)
  /// into the target rectangle.
  void drawImageStretched(gcn::Image* image,
                          const Rect& source,
                          int dstX, int dstY, int height, int width);

  void drawImage(gcn::Image* image, const Rect& source, int dstX, int dstY);
};

#endif
