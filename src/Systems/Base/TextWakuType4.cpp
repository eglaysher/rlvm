// -*- Mode: C++; tab-width:2; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi:tw=80:et:ts=2:sts=2
//
// -----------------------------------------------------------------------
//
// This file is part of RLVM, a RealLive virtual machine clone.
//
// -----------------------------------------------------------------------
//
// Copyright (C) 2009 Elliot Glaysher
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

#include "Systems/Base/TextWakuType4.hpp"

#include <iostream>
#include <string>

#include "Systems/Base/Colour.hpp"
#include "Systems/Base/GraphicsSystem.hpp"
#include "Systems/Base/Surface.hpp"
#include "Systems/Base/System.hpp"
#include "Systems/Base/TextWindow.hpp"
#include "libReallive/gameexe.h"

using std::endl;

// A listing of all the g00 regions in type 4 wakus. The ranges to the right
// are from the file smw01a.g00 in CLANNAD_FV.
enum WakuPart {
  TOP_LEFT_CORNER,      // <region x1="0" y1="0" x2="3" y2="3"/>
  TOP_CENTER,           // <region x1="4" y1="0" x2="19" y2="3"/>
  TOP_RIGHT_CORNER,     // <region x1="20" y1="0" x2="23" y2="3"/>
  LEFT_SIDE,            // <region x1="0" y1="4" x2="1" y2="5"/>
  UNKNOWN_1,            // <region x1="0" y1="4" x2="1" y2="5"/>
  UNKNOWN_2,            // <region x1="0" y1="4" x2="1" y2="5"/>
  RIGHT_SIDE,           // <region x1="22" y1="4" x2="23" y2="5"/>
  UNKNOWN_3,            // <region x1="22" y1="4" x2="23" y2="5"/>
  UNKNOWN_4,            // <region x1="22" y1="4" x2="23" y2="5"/>
  BOTTOM_LEFT_CORNER,   // <region x1="0" y1="20" x2="3" y2="23"/>
  BOTTOM_CENTER,        // <region x1="4" y1="20" x2="19" y2="23"/>
  BOTTOM_RIGHT_CORNER,  // <region x1="20" y1="20" x2="23" y2="23"/>
};

TextWakuType4::TextWakuType4(System& system, TextWindow& window, int setno,
                             int no)
    : system_(system), window_(window), setno_(setno), no_(no) {
  GameexeInterpretObject waku(system_.gameexe()("WAKU", setno_, no_));
  setWakuMain(waku("NAME").to_string(""));
}

// -----------------------------------------------------------------------

TextWakuType4::~TextWakuType4() {
}

// -----------------------------------------------------------------------

void TextWakuType4::execute() {
}

// -----------------------------------------------------------------------

void TextWakuType4::render(std::ostream* tree, Point box_location,
                           Size namebox_size) {
  if (tree) {
    *tree << "    Window Waku(" << setno_ << ", " << no_ << "):" << endl;
  }

  if (waku_main_) {
    boost::shared_ptr<Surface> backing = getWakuBackingOfSize(namebox_size);
    backing->renderToScreenAsColorMask(
        backing->rect(),
        Rect(box_location, namebox_size),
        window_.colour(), window_.filter());

    // Top row
    waku_main_->renderToScreen(top_left.rect,
                               Rect(box_location, top_left.rect.size()));

    const Point top_center_p = box_location + Size(top_left.rect.width(), 0);
    int top_center_width = namebox_size.width() - top_left.rect.width() -
                           top_right.rect.width();
    const Size top_center_s = Size(top_center_width, top_center.rect.height());
    waku_main_->renderToScreen(top_center.rect,
                               Rect(top_center_p, top_center_s));

    const Point top_right_p = top_center_p + Size(top_center_width, 0);
    waku_main_->renderToScreen(top_right.rect,
                               Rect(top_right_p, top_right.rect.size()));

    // Center row
    const Point left_side_p = box_location + Size(0, top_left.rect.height());
    int left_side_height = namebox_size.height() - top_left.rect.height() -
                           bottom_left.rect.height();
    const Size left_side_s = Size(left_side.rect.width(), left_side_height);
    waku_main_->renderToScreen(left_side.rect,
                               Rect(left_side_p, left_side_s));

    const Point right_side_p =
        box_location +
        Size(namebox_size.width() - right_side.rect.width(),
             top_right.rect.height());
    int right_side_height = namebox_size.height() - top_right.rect.height() -
                            bottom_right.rect.height();
    const Size right_side_s = Size(right_side.rect.width(), right_side_height);
    waku_main_->renderToScreen(right_side.rect,
                               Rect(right_side_p, right_side_s));

    // Bottom row
    const Point bottom_left_p = left_side_p + Size(0, left_side_height);
    waku_main_->renderToScreen(bottom_left.rect,
                               Rect(bottom_left_p, bottom_left.rect.size()));

    // While sanity dictates that top_center_width should equal
    // bottom_center_width, keep in mind that we are dealing with RealLive
    // here.
    const Point bottom_center_p = bottom_left_p +
                                  Size(bottom_left.rect.width(), 0);
    int bottom_center_width = namebox_size.width() - bottom_left.rect.width() -
                           bottom_right.rect.width();
    const Size bottom_center_s = Size(bottom_center_width,
                                      bottom_center.rect.height());
    waku_main_->renderToScreen(bottom_center.rect,
                               Rect(bottom_center_p, bottom_center_s));

    const Point bottom_right_p = bottom_center_p + Size(bottom_center_width, 0);
    waku_main_->renderToScreen(bottom_right.rect,
                               Rect(bottom_right_p, bottom_right.rect.size()));

    if (tree) {
      *tree << "      Main Area: " << Rect(box_location, namebox_size)
            << endl;
    }
  }
}

// -----------------------------------------------------------------------

bool TextWakuType4::getSize(Size& out) const {
  out = Size();
  return false;
}

// -----------------------------------------------------------------------

void TextWakuType4::setMousePosition(const Point& pos) {
  // Noop
}

// -----------------------------------------------------------------------

bool TextWakuType4::handleMouseClick(RLMachine& machine, const Point& pos,
                                     bool pressed) {
  // Noop; this window won't do anything with mouse clicks.
  return false;
}

// -----------------------------------------------------------------------

void TextWakuType4::setWakuMain(const std::string& name) {
  if (name != "") {
    waku_main_ = system_.graphics().loadNonCGSurfaceFromFile(name);

    top_left = waku_main_->getPattern(TOP_LEFT_CORNER);
    top_center = waku_main_->getPattern(TOP_CENTER);
    top_right = waku_main_->getPattern(TOP_RIGHT_CORNER);

    left_side = waku_main_->getPattern(LEFT_SIDE);
    right_side = waku_main_->getPattern(RIGHT_SIDE);

    bottom_left = waku_main_->getPattern(BOTTOM_LEFT_CORNER);
    bottom_center = waku_main_->getPattern(BOTTOM_CENTER);
    bottom_right = waku_main_->getPattern(BOTTOM_RIGHT_CORNER);
  } else {
    waku_main_.reset();
  }
}

// -----------------------------------------------------------------------

const boost::shared_ptr<Surface>& TextWakuType4::getWakuBackingOfSize(
    Size size) {
  if (!cached_backing_ || cached_backing_->size() != size) {
    cached_backing_ = system_.graphics().buildSurface(size);
    cached_backing_->fill(RGBAColour::Black());
  }

  return cached_backing_;
}
