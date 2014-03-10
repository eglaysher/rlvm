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

#include "platforms/gcn/gcn_graphics.h"

#include <guichan/exception.hpp>
#include <guichan/image.hpp>
#include <guichan/opengl/openglimage.hpp>

#include "base/notification_service.h"
#include "platforms/gcn/gcn_utils.h"

// -----------------------------------------------------------------------
// ImageRect
// -----------------------------------------------------------------------
ImageRect::ImageRect(ThemeImage resource_id, const int xpos[], const int ypos[])
    : resource_id_(resource_id) {
  int id = 0;
  for (int y = 0; y < 3; ++y) {
    for (int x = 0; x < 3; ++x) {
      rect_[id] = Rect::REC(
          xpos[x], ypos[y], xpos[x + 1] - xpos[x], ypos[y + 1] - ypos[y]);
      id++;
    }
  }

  registrar_.Add(this,
                 NotificationType::FULLSCREEN_STATE_CHANGED,
                 NotificationService::AllSources());
}

gcn::Image* ImageRect::image() {
  if (!image_)
    image_.reset(getThemeImage(resource_id_));

  return image_.get();
}

void ImageRect::Observe(NotificationType type,
                        const NotificationSource& source,
                        const NotificationDetails& details) {
  image_.reset();
}

// -----------------------------------------------------------------------
// GCNGraphics
// -----------------------------------------------------------------------
GCNGraphics::GCNGraphics(int width, int height)
    : gcn::OpenGLGraphics(width, height) {}

// -----------------------------------------------------------------------

GCNGraphics::~GCNGraphics() {}

// -----------------------------------------------------------------------

void GCNGraphics::drawImageRect(int x, int y, int w, int h, ImageRect& i) {
  pushClipArea(gcn::Rectangle(x, y, w, h));

  gcn::Image* image = i.image();

  // Draw the center area
  drawImageStretched(image,
                     i.center(),
                     i.topLeft().width(),
                     i.topLeft().height(),
                     w - i.topLeft().width() - i.topRight().width(),
                     h - i.topLeft().height() - i.bottomLeft().height());

  // Draw the sides
  drawImageStretched(image,
                     i.top(),
                     i.left().width(),
                     0,
                     w - i.left().width() - i.right().width(),
                     i.top().height());
  drawImageStretched(image,
                     i.bottom(),
                     i.left().width(),
                     h - i.bottom().height(),
                     w - i.left().width() - i.right().width(),
                     i.bottom().height());
  drawImageStretched(image,
                     i.left(),
                     0,
                     i.top().height(),
                     i.left().width(),
                     h - i.top().height() - i.bottom().height());
  drawImageStretched(image,
                     i.right(),
                     w - i.right().width(),
                     i.top().height(),
                     i.right().width(),
                     h - i.top().height() - i.bottom().height());

  // Draw the corners
  drawImageImpl(image, i.topLeft(), 0, 0);
  drawImageImpl(image, i.topRight(), w - i.topRight().width(), 0);
  drawImageImpl(image, i.bottomLeft(), 0, h - i.bottomLeft().height());
  drawImageImpl(image,
                i.bottomRight(),
                w - i.bottomRight().width(),
                h - i.bottomRight().height());

  popClipArea();
}

// -----------------------------------------------------------------------

void GCNGraphics::drawImageStretched(gcn::Image* image,
                                     const Rect& source,
                                     int dstX,
                                     int dstY,
                                     int width,
                                     int height) {
  const gcn::OpenGLImage* srcImage =
      dynamic_cast<const gcn::OpenGLImage*>(image);

  if (srcImage == NULL) {
    throw GCN_EXCEPTION(
        "Trying to draw an image of unknown format, must be "
        "an OpenGLImage.");
  }

  if (mClipStack.empty()) {
    throw GCN_EXCEPTION(
        "Clip stack is empty, perhaps you called a draw "
        "function outside of _beginDraw() and _endDraw()?");
  }

  const gcn::ClipRectangle& top = mClipStack.top();

  dstX += top.xOffset;
  dstY += top.yOffset;

  // Find OpenGL texture coordinates
  float texX1 = source.x() / (float)srcImage->getTextureWidth();
  float texY1 = source.y() / (float)srcImage->getTextureHeight();
  float texX2 = source.x2() / (float)srcImage->getTextureWidth();
  float texY2 = source.y2() / (float)srcImage->getTextureHeight();

  glBindTexture(GL_TEXTURE_2D, srcImage->getTextureHandle());

  glEnable(GL_TEXTURE_2D);
  glEnable(GL_BLEND);

  // Draw a textured quad -- the image
  glBegin(GL_QUADS);
  glTexCoord2f(texX1, texY1);
  glVertex3i(dstX, dstY, 0);

  glTexCoord2f(texX1, texY2);
  glVertex3i(dstX, dstY + height, 0);

  glTexCoord2f(texX2, texY2);
  glVertex3i(dstX + width, dstY + height, 0);

  glTexCoord2f(texX2, texY1);
  glVertex3i(dstX + width, dstY, 0);
  glEnd();
  glDisable(GL_TEXTURE_2D);
}

// -----------------------------------------------------------------------

void GCNGraphics::drawImageImpl(gcn::Image* image,
                                const Rect& source,
                                int dstX,
                                int dstY) {
  OpenGLGraphics::drawImage(image,
                            source.x(),
                            source.y(),
                            dstX,
                            dstY,
                            source.width(),
                            source.height());
}
