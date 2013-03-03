// -*- Mode: C++; tab-width:2; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi:tw=80:et:ts=2:sts=2
//
// -----------------------------------------------------------------------
//
// This file is part of RLVM, a RealLive virtual machine clone.
//
// -----------------------------------------------------------------------
//
// Copyright (C) 2006 Elliot Glaysher
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
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
//
// -----------------------------------------------------------------------

#include "Systems/Base/GraphicsObjectData.hpp"

#include <ostream>

#include "Systems/Base/GraphicsObject.hpp"
#include "Systems/Base/GraphicsObjectOfFile.hpp"
#include "Systems/Base/Surface.hpp"
#include "Systems/Base/Rect.hpp"

using namespace std;

// -----------------------------------------------------------------------
// GraphicsObjectData
// -----------------------------------------------------------------------

GraphicsObjectData::GraphicsObjectData()
  : after_animation_(AFTER_NONE), owned_by_(NULL), currently_playing_(false),
    animation_finished_(false) {
}

GraphicsObjectData::GraphicsObjectData(const GraphicsObjectData& obj)
  : after_animation_(obj.after_animation_), owned_by_(NULL),
    currently_playing_(obj.currently_playing_),
    animation_finished_(false) {
}

GraphicsObjectData::~GraphicsObjectData() {
}

void GraphicsObjectData::render(const GraphicsObject& go,
                                const GraphicsObject* parent,
                                std::ostream* tree) {
  boost::shared_ptr<const Surface> surface = currentSurface(go);
  if (surface) {
    Rect src = srcRect(go);
    Rect dst = dstRect(go, parent);
    int alpha = getRenderingAlpha(go, parent);

    if (go.buttonUsingOverides()) {
      // Tacked on side channel that lets a ButtonObjectSelectLongOperation
      // tweak the x/y coordinates of dst. There isn't really a better place to
      // put this. It can't go in dstRect() because the LongOperation also
      // consults the data from dstRect().
      dst = Rect(dst.origin() + Size(go.buttonXOffsetOverride(),
                                     go.buttonYOffsetOverride()),
                 dst.size());
    }

    if (tree) {
      objectInfo(*tree);
      *tree << "  Rendering " << src << " to " << dst << endl;
      if (parent) {
        *tree << "  Parent: ";
        PrintGraphicsObjectToTree(*parent, tree);
        *tree << endl;
      }

      *tree << "  Properties: ";
      if (alpha != 255)
        *tree << "(alpha=" << alpha << ") ";
      PrintGraphicsObjectToTree(go, tree);
      *tree << endl;
    }

    if (parent && parent->hasOwnClip()) {
      // In Little Busters, a parent clip rect is used to clip text scrolling
      // in the battle system. rlvm has the concept of parent objects badly
      // hacked in, and that means we can't directly apply the own clip
      // rect. Instead we have to calculate this in terms of the screen
      // coordinates and then apply that as a global clip rect.
      Point parent_start(parent->x() + parent->xAdjustmentSum(),
                         parent->y() + parent->yAdjustmentSum());
      Rect full_parent_clip =
          Rect(parent_start + parent->ownClipRect().origin(),
               parent->ownClipRect().size());

      Rect clipped_dest = dst.intersection(full_parent_clip);
      Rect inset = dst.getInsetRectangle(clipped_dest);
      dst = clipped_dest;
      src = src.applyInset(inset);

      if (tree) {
        *tree << "  Parent Own Clipping Rect: " << parent->ownClipRect()
              << endl
              << "  After clipping: " << src << " to " << dst << endl;
      }
    }

    if (go.hasOwnClip()) {
      dst = dst.applyInset(go.ownClipRect());
      src = src.applyInset(go.ownClipRect());

      if (tree) {
        *tree << "  Internal Clipping Rect: " << go.ownClipRect() << endl
              << "  After internal clipping: " << src << " to " << dst
              << endl;
      }
    }

    // Perform the object clipping.
    if (go.hasClip()) {
      Rect clipped_dest = dst.intersection(go.clipRect());

      // Do nothing if object falls wholly outside clip area
      if (clipped_dest.isEmpty())
        return;

      // Adjust the source rectangle
      Rect inset = dst.getInsetRectangle(clipped_dest);

      dst = clipped_dest;
      src = src.applyInset(inset);

      if (tree) {
        *tree << "  Clipping Rect: " << go.clipRect() << endl
              << "  After clipping: " << src << " to " << dst << endl;
      }
    }

    surface->renderToScreenAsObject(go, src, dst, alpha);
  }
}

void GraphicsObjectData::loopAnimation() {
}

void GraphicsObjectData::endAnimation() {
  // Set first, because we may deallocate this by one of our actions
  currently_playing_ = false;

  switch (afterAnimation()) {
  case AFTER_NONE:
    animation_finished_ = true;
    break;
  case AFTER_CLEAR:
    if (ownedBy())
      ownedBy()->deleteObject();
    break;
  case AFTER_LOOP: {
    // Reset from the beginning
    currently_playing_ = true;
    loopAnimation();
    break;
  }
  }
}

void GraphicsObjectData::PrintGraphicsObjectToTree(const GraphicsObject& go,
                                                   std::ostream* tree) {
  if (go.mono())
    *tree << "(mono) ";
  if (go.invert())
    *tree << "(invert) ";
  if (go.light())
    *tree << "(light=" << go.light() << ") ";
  if (go.tint() != RGBColour::Black())
    *tree << "(tint=" << go.tint() << ") ";
  if (go.colour() != RGBAColour::Clear())
    *tree << "(colour=" << go.colour() << ") ";
}

Rect GraphicsObjectData::srcRect(const GraphicsObject& go) {
  return currentSurface(go)->getPattern(go.pattNo()).rect;
}

Point GraphicsObjectData::dstOrigin(const GraphicsObject& go) {
  boost::shared_ptr<const Surface> surface = currentSurface(go);
  if (surface) {
    return Point(surface->getPattern(go.pattNo()).originX,
                 surface->getPattern(go.pattNo()).originY);
  }

  return Point();
}

Rect GraphicsObjectData::dstRect(const GraphicsObject& go,
                                 const GraphicsObject* parent) {
  Point origin = dstOrigin(go);
  Rect src = srcRect(go);

  int center_x = go.x() + go.xAdjustmentSum() - origin.x() +
                 (src.width() / 2.0f);
  int center_y = go.y() + go.yAdjustmentSum() - origin.y() +
                 (src.height() / 2.0f);

  float second_factor_x = 1.0f;
  float second_factor_y = 1.0f;
  if (parent) {
    center_x += parent->x() + parent->xAdjustmentSum();
    center_y += parent->y() + parent->yAdjustmentSum();

    second_factor_x = parent->getWidthScaleFactor();
    second_factor_y = parent->getHeightScaleFactor();
  }

  int half_real_width = (src.width() * second_factor_x *
                         go.getWidthScaleFactor()) / 2.0f;
  int half_real_height = (src.height() * second_factor_y *
                          go.getHeightScaleFactor()) / 2.0f;

  int xPos1 = center_x - half_real_width;
  int yPos1 = center_y - half_real_height;
  int xPos2 = center_x + half_real_width;
  int yPos2 = center_y + half_real_height;

  return Rect::GRP(xPos1, yPos1, xPos2, yPos2);
}

int GraphicsObjectData::getRenderingAlpha(const GraphicsObject& go,
                                          const GraphicsObject* parent) {
  if (!parent) {
    return go.computedAlpha();
  } else {
    return int((parent->computedAlpha() / 255.0f) *
               (go.computedAlpha() / 255.0f) * 255);
  }
}

bool GraphicsObjectData::isAnimation() const {
  return false;
}

void GraphicsObjectData::playSet(int set) {
}

bool GraphicsObjectData::animationFinished() const {
  return animation_finished_;
}
