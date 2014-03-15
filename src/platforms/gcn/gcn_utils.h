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

#ifndef SRC_PLATFORMS_GCN_GCN_UTILS_H_
#define SRC_PLATFORMS_GCN_GCN_UTILS_H_

#include <guichan/image.hpp>
#include <guichan/rectangle.hpp>

#include "systems/base/rect.h"

enum ThemeImage {
  IMG_BUTTON_DISABLED = 0,
  IMG_BUTTONHI,
  IMG_BUTTON,
  IMG_BUTTONPRESS,
  IMG_DEEPBOX,
  IMG_HSCROLL_LEFT_DEFAULT,
  IMG_HSCROLL_LEFT_PRESSED,
  IMG_HSCROLL_RIGHT_DEFAULT,
  IMG_HSCROLL_RIGHT_PRESSED,
  IMG_VSCROLL_DOWN_DEFAULT,
  IMG_VSCROLL_DOWN_PRESSED,
  IMG_VSCROLL_GREY,
  IMG_VSCROLL_UP_DEFAULT,
  IMG_VSCROLL_UP_PRESSED
};

gcn::Image* getThemeImage(enum ThemeImage img);

inline gcn::Rectangle rectConvert(const Rect& obj) {
  return gcn::Rectangle(obj.x(), obj.y(), obj.x2(), obj.y2());
}

#endif  // SRC_PLATFORMS_GCN_GCN_UTILS_H_
