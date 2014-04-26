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
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
// -----------------------------------------------------------------------

#include "systems/base/text_waku_type4.h"

#include <ostream>
#include <string>
#include <vector>

#include "systems/base/colour.h"
#include "systems/base/graphics_system.h"
#include "systems/base/surface.h"
#include "systems/base/system.h"
#include "systems/base/text_window.h"
#include "libreallive/gameexe.h"

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

TextWakuType4::TextWakuType4(System& system,
                             TextWindow& window,
                             int setno,
                             int no)
    : system_(system),
      window_(window),
      setno_(setno),
      no_(no),
      area_top_(0),
      area_bottom_(0),
      area_left_(0),
      area_right_(0) {
  GameexeInterpretObject waku(system_.gameexe()("WAKU", setno_, no_));
  SetWakuMain(waku("NAME").ToString(""));

  std::vector<int> area = waku("AREA");
  if (area.size() >= 1)
    area_top_ = area[0];
  if (area.size() >= 2)
    area_bottom_ = area[1];
  if (area.size() >= 3)
    area_left_ = area[2];
  if (area.size() >= 4)
    area_right_ = area[3];
}

TextWakuType4::~TextWakuType4() {}

void TextWakuType4::Execute() {}

void TextWakuType4::Render(std::ostream* tree,
                           Point box_location,
                           Size content_size) {
  if (tree) {
    *tree << "    Window Waku(" << setno_ << ", " << no_ << "):" << endl;
  }

  if (waku_main_) {
    // Calculate the location/area and render the filtered background.
    Point backing_point =
        box_location + Size(left_side.rect.width(), top_center.rect.height()) -
        Size(area_left_, area_top_);
    Size backing_size =
        content_size + Size(area_left_ + area_right_, area_top_ + area_bottom_);
    std::shared_ptr<Surface> backing = GetWakuBackingOfSize(backing_size);
    backing->RenderToScreenAsColorMask(backing->GetRect(),
                                       Rect(backing_point, backing_size),
                                       window_.colour(),
                                       window_.filter());

    // Calculate the total size of the waku decoration. We need this to get the
    // size of the non-corners correct.
    Size total_size =
        Size(left_side.rect.width(), top_center.rect.height()) + content_size +
        Size(right_side.rect.width(), bottom_center.rect.height());

    // Top row
    waku_main_->RenderToScreen(top_left.rect,
                               Rect(box_location, top_left.rect.size()));

    const Point top_center_p = box_location + Size(top_left.rect.width(), 0);
    int top_center_width =
        total_size.width() - top_left.rect.width() - top_right.rect.width();
    const Size top_center_s = Size(top_center_width, top_center.rect.height());
    waku_main_->RenderToScreen(top_center.rect,
                               Rect(top_center_p, top_center_s));

    const Point top_right_p = top_center_p + Size(top_center_width, 0);
    waku_main_->RenderToScreen(top_right.rect,
                               Rect(top_right_p, top_right.rect.size()));

    // Center row
    const Point left_side_p = box_location + Size(0, top_left.rect.height());
    int left_side_height = content_size.height();
    const Size left_side_s = Size(left_side.rect.width(), left_side_height);
    waku_main_->RenderToScreen(left_side.rect, Rect(left_side_p, left_side_s));

    const Point right_side_p =
        box_location + Size(total_size.width() - right_side.rect.width(),
                            top_right.rect.height());
    int right_side_height = content_size.height();
    const Size right_side_s = Size(right_side.rect.width(), right_side_height);
    waku_main_->RenderToScreen(right_side.rect,
                               Rect(right_side_p, right_side_s));

    // Bottom row
    const Point bottom_left_p = left_side_p + Size(0, left_side_height);
    waku_main_->RenderToScreen(bottom_left.rect,
                               Rect(bottom_left_p, bottom_left.rect.size()));

    // Sometimes |top_center_width| != |bottom_center_width| (for example, when
    // the decorations have a larger rounded corner on the top).
    const Point bottom_center_p =
        bottom_left_p + Size(bottom_left.rect.width(), 0);
    int bottom_center_width = total_size.width() - bottom_left.rect.width() -
                              bottom_right.rect.width();
    const Size bottom_center_s =
        Size(bottom_center_width, bottom_center.rect.height());
    waku_main_->RenderToScreen(bottom_center.rect,
                               Rect(bottom_center_p, bottom_center_s));

    const Point bottom_right_p = bottom_center_p + Size(bottom_center_width, 0);
    waku_main_->RenderToScreen(bottom_right.rect,
                               Rect(bottom_right_p, bottom_right.rect.size()));

    if (tree) {
      *tree << "      Main Area: " << Rect(box_location, content_size) << endl;
    }
  }
}

Size TextWakuType4::GetSize(const Size& text_surface) const {
  Size padding = Size(left_side.rect.width() + right_side.rect.width(),
                      top_center.rect.height() + bottom_center.rect.height());
  return text_surface + padding;
}

Point TextWakuType4::InsertionPoint(const Rect& waku_rect,
                                    const Size& padding,
                                    const Size& surface_size,
                                    bool center) const {
  Point insertion_point = waku_rect.origin();
  if (center) {
    int half_width = (waku_rect.width()) / 2;
    int half_text_width = surface_size.width() / 2;

    int half_height = (waku_rect.height()) / 2;
    int half_text_height = surface_size.height() / 2;

    insertion_point +=
        Point(half_width - half_text_width, half_height - half_text_height);
  } else {
    insertion_point += Point(padding.width(), padding.height());
  }

  return insertion_point;
}

void TextWakuType4::SetMousePosition(const Point& pos) {
  // Noop
}

bool TextWakuType4::HandleMouseClick(RLMachine& machine,
                                     const Point& pos,
                                     bool pressed) {
  // Noop; this window won't do anything with mouse clicks.
  return false;
}

void TextWakuType4::SetWakuMain(const std::string& name) {
  if (name != "") {
    waku_main_ = system_.graphics().GetSurfaceNamed(name);

    top_left = waku_main_->GetPattern(TOP_LEFT_CORNER);
    top_center = waku_main_->GetPattern(TOP_CENTER);
    top_right = waku_main_->GetPattern(TOP_RIGHT_CORNER);

    left_side = waku_main_->GetPattern(LEFT_SIDE);
    right_side = waku_main_->GetPattern(RIGHT_SIDE);

    bottom_left = waku_main_->GetPattern(BOTTOM_LEFT_CORNER);
    bottom_center = waku_main_->GetPattern(BOTTOM_CENTER);
    bottom_right = waku_main_->GetPattern(BOTTOM_RIGHT_CORNER);
  } else {
    waku_main_.reset();
  }
}

const std::shared_ptr<Surface>& TextWakuType4::GetWakuBackingOfSize(
    Size size) {
  if (!cached_backing_ || cached_backing_->GetSize() != size) {
    cached_backing_ = system_.graphics().BuildSurface(size);
    cached_backing_->Fill(RGBAColour::Black());
  }

  return cached_backing_;
}
