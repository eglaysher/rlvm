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

#ifndef SRC_SYSTEMS_BASE_TEXTKEYCURSOR_HPP_
#define SRC_SYSTEMS_BASE_TEXTKEYCURSOR_HPP_

#include "Systems/Base/Rect.hpp"

#include <string>
#include <vector>
#include <boost/shared_ptr.hpp>

class Surface;
class System;
class TextWindow;

// Represents a key cursor, the little icon after a block of text telling the
// user to click to advance.
class TextKeyCursor {
 public:
  // Creates a Key Cursor object based off of the properties in
  // \#CURSOR.curosr_number. This will load all image files, and data
  // from the Gameexe.
  TextKeyCursor(System& system, int curosr_number);
  ~TextKeyCursor();

  // Updates the key cursor properties during the System::execute()
  // phase. This should run once every game loop while a key cursor is
  // displayed on the screen.
  void execute();

  // Render this key cursor to the specified window, which owns
  // positional information.
  void render(TextWindow& text_window, std::ostream* tree);

  // Returns which cursor we are.
  int cursorNumber() const { return cursor_number_; }

  // Convenience setters which set properties directly from
  // Gameexe.ini data.
  void setCursorImage(System& system, const std::string& file);
  void setCursorSize(const std::vector<int>& image_size);
  void setCursorFrameCount(const int frame_count);
  void setCursorFrameSpeed(const int frame_speed);

 private:
  // Keep track of which cursor we are as an optimization
  int cursor_number_;

  // The surface containing the cursor
  boost::shared_ptr<Surface> cursor_image_;

  // The cursor image file
  std::string cursor_image_file_;

  // The size of the cursor
  Size frame_size_;

  // Number of frames in this cursor
  int frame_count_;

  // Current frame being displayed
  int current_frame_;

  // How long an individual frame should be displayed
  int frame_speed_;

  // The last time current_frame_ was incremented in ticks
  unsigned int last_time_frame_incremented_;

  System& system_;
};

#endif  // SRC_SYSTEMS_BASE_TEXTKEYCURSOR_HPP_
