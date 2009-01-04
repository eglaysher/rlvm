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

#ifndef __NullTextWindow_hpp__
#define __NullTextWindow_hpp__

#include <string>
#include "Systems/Base/TextWindow.hpp"

#include "NullSystem/MockLog.hpp"

class NullTextWindow : public TextWindow
{
public:
  NullTextWindow(System& system, int x);
  ~NullTextWindow();
  virtual void execute() {}

  virtual void setFontColor(const std::vector<int>& color_data);

  // We don't test graphics in the null system, so don't really
  // implement the waku parts.
  virtual void render(std::ostream* tree) {}

  virtual bool displayChar(const std::string& current,
                           const std::string& next);
  virtual int charWidth(unsigned short codepoint) const { return 0; }

  virtual void clearWin();
  virtual void setName(const std::string& utf8name,
                       const std::string& next_char);
  virtual void setNameWithoutDisplay(const std::string& utf8name) { }

  virtual void hardBrake();

  // To implement for real, instead of just recording in the mocklog.
  virtual void resetIndentation();
  virtual void markRubyBegin();
  virtual void displayRubyText(const std::string& utf8str);

  virtual bool isFull() const { return false; }

  std::string currentContents() const { return current_contents_; }

  virtual void addSelectionItem(const std::string& utf8str) {}

  MockLog& log() { return text_window_log_; }

private:
  std::string current_contents_;

  /// Record all method calls here
  MockLog text_window_log_;
};

#endif
