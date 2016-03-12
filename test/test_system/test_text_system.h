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

#ifndef TEST_TEST_SYSTEM_TEST_TEXT_SYSTEM_H_
#define TEST_TEST_SYSTEM_TEST_TEXT_SYSTEM_H_

#include "systems/base/rect.h"
#include "systems/base/text_system.h"

#include <string>
#include <tuple>

class MockTextWindow;
class RLMachine;
class TextWindow;
class TestTextWindow;

class TestTextSystem : public TextSystem {
 public:
  TestTextSystem(System& system, Gameexe& gexe);
  virtual ~TestTextSystem();

  // Overridden from TextSystem:
  virtual std::shared_ptr<TextWindow> GetTextWindow(
      int text_window_number) override;
  virtual Size RenderGlyphOnto(const std::string& current,
                               int font_size,
                               bool italic,
                               const RGBColour& font_colour,
                               const RGBColour* shadow_colour,
                               int insertion_point_x,
                               int insertion_point_y,
                               const std::shared_ptr<Surface>& destination) override;
  virtual int GetCharWidth(int size, uint16_t codepoint) override;
  bool FontIsMonospaced() override;

  const std::vector<std::tuple<std::string, int, int>>& glyphs() {
    return rendered_glyps_;
  }

 private:
  std::vector<std::tuple<std::string, int, int>> rendered_glyps_;
};

#endif  // TEST_TEST_SYSTEM_TEST_TEXT_SYSTEM_H_
