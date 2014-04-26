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
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
//
// -----------------------------------------------------------------------

#include "systems/base/selection_element.h"

#include "machine/rlmachine.h"
#include "systems/base/graphics_system.h"
#include "systems/base/sound_system.h"
#include "systems/base/surface.h"
#include "systems/base/system.h"
#include "systems/base/rect.h"

// -----------------------------------------------------------------------
// SelectionElement
// -----------------------------------------------------------------------
SelectionElement::SelectionElement(
    System& system,
    const std::shared_ptr<Surface>& normal_image,
    const std::shared_ptr<Surface>& highlighted_image,
    const std::function<void(int)>& selection_callback,
    int id,
    const Point& pos)
    : is_highlighted_(false),
      id_(id),
      pos_(pos),
      normal_image_(normal_image),
      highlighted_image_(highlighted_image),
      selection_callback_(selection_callback),
      system_(system) {}

SelectionElement::~SelectionElement() {}

void SelectionElement::SetSelectionCallback(
    const std::function<void(int)>& func) {
  selection_callback_ = func;
}

bool SelectionElement::IsHighlighted(const Point& p) {
  return Rect(pos_, normal_image_->GetSize()).Contains(p);
}

void SelectionElement::SetMousePosition(const Point& pos) {
  bool start_value = is_highlighted_;
  is_highlighted_ = IsHighlighted(pos);

  if (start_value != is_highlighted_) {
    system_.graphics().MarkScreenAsDirty(GUT_TEXTSYS);
    if (is_highlighted_ && system_.sound().HasSe(0))
      system_.sound().PlaySe(0);
  }
}

bool SelectionElement::HandleMouseClick(const Point& pos, bool pressed) {
  if (pressed == false && IsHighlighted(pos)) {
    // Released within the button
    if (selection_callback_)
      selection_callback_(id_);

    return true;
  } else {
    return false;
  }
}

void SelectionElement::Render() {
  std::shared_ptr<Surface> target;

  if (is_highlighted_)
    target = highlighted_image_;
  else
    target = normal_image_;

  Size s = target->GetSize();

  target->RenderToScreen(Rect(Point(0, 0), s), Rect(pos_, s), 255);
}
