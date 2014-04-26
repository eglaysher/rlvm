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

#ifndef SRC_PLATFORMS_GCN_GCN_GRAPHICS_H_
#define SRC_PLATFORMS_GCN_GCN_GRAPHICS_H_

#include <guichan/opengl/openglgraphics.hpp>
#include <guichan/image.hpp>

#include <memory>

#include "base/notification_observer.h"
#include "base/notification_registrar.h"
#include "platforms/gcn/gcn_utils.h"
#include "systems/base/rect.h"


// 9 rectangles in an image. 4 corners, 4 sides and a middle area. The
// topology is as follows:
//
// <pre>
//  !-----!-----------------!-----!
//  !  0  !        1        !  2  !
//  !-----!-----------------!-----!
//  !  3  !        4        !  5  !
//  !-----!-----------------!-----!
//  !  6  !        7        !  8  !
//  !-----!-----------------!-----!
// </pre>
//
// Sections 0, 2, 6 and 8 will remain as is. 1, 3, 4, 5 and 7 will be
// repeated to fit the size of the widget.
//
// Originally from The Mana World; modified to use Guichan's standard image
// class, and to only keep one image and store regions into it.
struct ImageRect : public NotificationObserver {
  ImageRect(ThemeImage resource, const int xpos[], const int ypos[]);

  gcn::Image* image();

  // Symbolic access to the various parts.
  const Rect& topLeft() const { return rect_[0]; }
  const Rect& top() const { return rect_[1]; }
  const Rect& topRight() const { return rect_[2]; }
  const Rect& left() const { return rect_[3]; }
  const Rect& center() const { return rect_[4]; }
  const Rect& right() const { return rect_[5]; }
  const Rect& bottomLeft() const { return rect_[6]; }
  const Rect& bottom() const { return rect_[7]; }
  const Rect& bottomRight() const { return rect_[8]; }

 private:
  // NotificationObserver:
  virtual void Observe(NotificationType type,
                       const NotificationSource& source,
                       const NotificationDetails& details);

  ThemeImage resource_id_;
  Rect rect_[9];

  std::shared_ptr<gcn::Image> image_;

  NotificationRegistrar registrar_;
};

// Custom Graphics with extra draw primatives used within the widget code that
// I'm taking from TMW.
//
// The basic idea is taken from The Mana World; this is the same interface they
// expose in their drawing code. The difference here is that I'm stretching
// widgets with anti-aliasing instead of mirroring their internals back and
// forth.
class GCNGraphics : public gcn::OpenGLGraphics {
 public:
  // Initializes the graphics with the current screen resolution.
  GCNGraphics(int width, int height);
  ~GCNGraphics();

  // Draws a rectangle using images. 4 corner images, 4 side images and 1
  // image for the inside.
  void drawImageRect(int x, int y, int w, int h, ImageRect& imgRect);

 private:
  // Hack around OpenGLGraphics::drawImage to render the image (stretched)
  // into the target rectangle.
  void drawImageStretched(gcn::Image* image,
                          const Rect& source,
                          int dstX,
                          int dstY,
                          int height,
                          int width);

  void drawImageImpl(gcn::Image* image, const Rect& source, int dstX, int dstY);
};

#endif  // SRC_PLATFORMS_GCN_GCN_GRAPHICS_H_
