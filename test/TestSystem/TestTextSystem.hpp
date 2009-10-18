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
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
//
// -----------------------------------------------------------------------

#ifndef TEST_TESTSYSTEM_TESTTEXTSYSTEM_HPP_
#define TEST_TESTSYSTEM_TESTTEXTSYSTEM_HPP_

#include "Systems/Base/TextSystem.hpp"
#include <boost/ptr_container/ptr_map.hpp>
#include <string>

class MockTextWindow;
class RLMachine;
class TextWindow;
class TestTextWindow;

class TestTextSystem : public TextSystem {
 public:
  TestTextSystem(System& system, Gameexe& gexe);
  ~TestTextSystem();

  void executeTextSystem(RLMachine& machine) { }
  void render(RLMachine& machine) { }

  MockTextWindow& getMockTextWindowAt(int text_window_number);
  virtual boost::shared_ptr<TextWindow> textWindow(int text_window_number);

  // We can safely ignore mouse clicks...for now.
  void setMousePosition(RLMachine& machine, const Point& pos) { }
  bool handleMouseClick(RLMachine& machine, const Point& pos,
                        bool pressed) { return false; }

  boost::shared_ptr<Surface> renderText(
      const std::string& utf8str, int size, int xspace,
      int yspace, const RGBColour& colour, RGBColour* shadow_colour) {
    return boost::shared_ptr<Surface>(); }
  boost::shared_ptr<Surface> renderUTF8Glyph(
      const std::string& current, int font_size, const RGBColour& colour,
      RGBColour* shadow_colour) { return boost::shared_ptr<Surface>(); }
};

#endif  // TEST_TESTSYSTEM_TESTTEXTSYSTEM_HPP_
