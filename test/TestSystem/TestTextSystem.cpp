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

#include "TestSystem/TestTextSystem.hpp"
#include "TestSystem/TestTextWindow.hpp"
#include "TestSystem/MockTextWindow.hpp"
#include "TestSystem/MockSurface.hpp"
#include "utf8cpp/utf8.h"

#include <string>
#include <boost/shared_ptr.hpp>

TestTextSystem::TestTextSystem(System& system, Gameexe& gexe)
    : TextSystem(system, gexe) {}

TestTextSystem::~TestTextSystem() { }

boost::shared_ptr<TextWindow> TestTextSystem::textWindow(int text_window_num) {
  WindowMap::iterator it = text_window_.find(text_window_num);
  if (it == text_window_.end()) {
    it = text_window_.insert(std::make_pair(
      text_window_num, boost::shared_ptr<TextWindow>(
          new ::testing::NiceMock<MockTextWindow>(
              ::testing::ByRef(system()), text_window_num)))).first;
  }

  return it->second;
}

Size TestTextSystem::renderGlyphOnto(
    const std::string& current,
    int font_size,
    const RGBColour& font_colour,
    const RGBColour* shadow_colour,
    int insertion_point_x,
    int insertion_point_y,
    const boost::shared_ptr<Surface>& destination) {
  // Keep track of the incoming data:
  rendered_glyps_.push_back(
      boost::make_tuple(current, insertion_point_x, insertion_point_y));

  return Size(20, 20);
}
