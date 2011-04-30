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

#ifndef TEST_TESTSYSTEM_TESTTEXTWINDOW_HPP_
#define TEST_TESTSYSTEM_TESTTEXTWINDOW_HPP_

#include <stdint.h>
#include <string>
#include <boost/shared_ptr.hpp>
#include "Systems/Base/TextWindow.hpp"

#include <vector>

class TestTextWindow : public TextWindow {
 public:
  TestTextWindow(System& system, int x);
  ~TestTextWindow();
  virtual void execute() {}

  virtual void setFontColor(const std::vector<int>& colour_data);

  virtual bool character(const std::string& current,
                         const std::string& next);
  virtual int charWidth(uint16_t codepoint) const { return 0; }

  virtual boost::shared_ptr<Surface> textSurface();
  virtual boost::shared_ptr<Surface> nameSurface();
  virtual void renderNameInBox(const std::string& utf8str);
  virtual void clearWin();
  virtual void setName(const std::string& utf8name,
                       const std::string& next_char);

  virtual void hardBrake();

  // To implement for real, instead of just recording in the mocklog.
  virtual void resetIndentation();
  virtual void markRubyBegin();
  virtual void displayRubyText(const std::string& utf8str);

  virtual bool isFull() const { return false; }

  std::string currentContents() const { return current_contents_; }

  virtual void addSelectionItem(const std::string& utf8str, int selection_id) {}

 private:
  std::string current_contents_;

  boost::shared_ptr<Surface> name_surface_;
};

#endif  // TEST_TESTSYSTEM_TESTTEXTWINDOW_HPP_
