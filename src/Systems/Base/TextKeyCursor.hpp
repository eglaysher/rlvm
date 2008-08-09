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

#ifndef __TextKeyCursor_hpp__
#define __TextKeyCursor_hpp__

#include "Systems/Base/Rect.hpp"

#include <boost/shared_ptr.hpp>

class RLMachine;
class Surface;
class TextWindow;

/**
 * Represents a key cursor.
 */
class TextKeyCursor
{
private:
  /// Keep track of which cursor we are as an optimization
  int cursor_number_;

  /// The surface containing the cursor
  boost::shared_ptr<Surface> cursor_image_;

  /// The size of the cursor
  Size frame_size_;

  /// Number of frames in this cursor
  int frame_count_;

  /// Current frame being displayed
  int current_frame_;

  /// How long an individual frame should be displayed
  int frame_speed_;

  /// The last time current_frame_ was incremented in ticks
  unsigned int last_time_frame_incremented_;

public:
  /**
   * Creates a Key Cursor object based off of the properties in
   * \#CURSOR.curosrNumber. This will load all image files, and data
   * from the Gameexe.
   */
  TextKeyCursor(RLMachine& machine, int curosrNumber);

  ~TextKeyCursor();

  /**
   * Updates the key cursor properties during the System::execute()
   * phase. This should run once every game loop while a key cursor is
   * displayed on the screen.
   */
  void execute(RLMachine& machine);

  /**
   * Render this key cursor to the specified window, which owns
   * positional information.
   */
  void render(RLMachine& machine, TextWindow& textWindow);

  /**
   * @name Accessors
   *
   * @{
   */
  int cursorNumber() const { return cursor_number_; }
  // @}


  /**
   * @name Gameexe setters
   *
   * Convenience setters which set properties directly from
   * Gameexe.ini data.
   *
   * @{
   */
  void setCursorImage(RLMachine& machine, const std::string& file);
  void setCursorSize(const std::vector<int>& imageSize);
  void setCursorFrameCount(const int frameCount);
  void setCursorFrameSpeed(const int frameSpeed);
  /// @}

};

#endif
