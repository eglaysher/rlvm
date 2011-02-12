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
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
//
// -----------------------------------------------------------------------

#include "Systems/Base/GraphicsObjectData.hpp"

#include <ostream>

#include "Systems/Base/GraphicsObject.hpp"
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

void GraphicsObjectData::render(const GraphicsObject& go, std::ostream* tree) {
  boost::shared_ptr<Surface> surface = currentSurface(go);
  if (surface) {
    Rect src = srcRect(go);
    Rect dst = dstRect(go);
    int alpha = getRenderingAlpha(go);

    // TODO: Anyone attempting moving the clip area calculations here should
    // verify that it doesn't break the final pan scene of Yumemi in
    // Planetarian.
    if (tree) {
      objectInfo(*tree);
      *tree << "  Rendering " << src << " to " << dst;
      if (alpha != 255)
        *tree << " (alpha=" << alpha << ")";
      if (go.hasClip())
        *tree << " [Warning: Clip rectangle calculations not applied.]";
      *tree << endl;
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

Rect GraphicsObjectData::srcRect(const GraphicsObject& go) {
  return currentSurface(go)->getPattern(go.pattNo()).rect;
}

Point GraphicsObjectData::dstOrigin(const GraphicsObject& go) {
  boost::shared_ptr<Surface> surface = currentSurface(go);
  if (surface) {
    return Point(surface->getPattern(go.pattNo()).originX,
                 surface->getPattern(go.pattNo()).originY);
  }

  return Point();
}

Rect GraphicsObjectData::dstRect(const GraphicsObject& go) {
  Point origin = dstOrigin(go);
  Rect src = srcRect(go);

  int xPos1 = go.x() + go.xAdjustmentSum() - origin.x();
  int yPos1 = go.y() + go.yAdjustmentSum() - origin.y();
  int xPos2 = int(xPos1 + src.width() * (go.width() / 100.0f));
  int yPos2 = int(yPos1 + src.height() * (go.height() / 100.0f));

  return Rect::GRP(xPos1, yPos1, xPos2, yPos2);
}

int GraphicsObjectData::getRenderingAlpha(const GraphicsObject& go) {
  return go.alpha();
}

bool GraphicsObjectData::isAnimation() const {
  return false;
}

void GraphicsObjectData::playSet(int set) {
}

bool GraphicsObjectData::animationFinished() const {
  return animation_finished_;
}
