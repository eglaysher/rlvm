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

#ifndef TEST_TEST_SYSTEM_TEST_TEXT_WINDOW_H_
#define TEST_TEST_SYSTEM_TEST_TEXT_WINDOW_H_

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include "systems/base/text_window.h"

class TestTextWindow : public TextWindow {
 public:
  TestTextWindow(System& system, int x);
  virtual ~TestTextWindow();

  // Overridden from TextWindow:
  virtual void SetFontColor(const std::vector<int>& colour_data) override;
  virtual std::shared_ptr<Surface> GetTextSurface() override;
  virtual std::shared_ptr<Surface> GetNameSurface() override;
  virtual bool DisplayCharacter(const std::string& current,
                                const std::string& next) override;

  virtual void RenderNameInBox(const std::string& utf8str);
  virtual void ClearWin() override;
  virtual void SetName(const std::string& utf8name,
                       const std::string& next_char) override;

  virtual void HardBrake() override;

  // To implement for real, instead of just recording in the mocklog.
  virtual void ResetIndentation() override;
  virtual void MarkRubyBegin() override;
  virtual void DisplayRubyText(const std::string& utf8str) override;

  std::string current_contents() const { return current_contents_; }

  virtual void AddSelectionItem(const std::string& utf8str, int selection_id) override {}

 private:
  std::string current_contents_;

  std::shared_ptr<Surface> name_surface_;
};

#endif  // TEST_TEST_SYSTEM_TEST_TEXT_WINDOW_H_
