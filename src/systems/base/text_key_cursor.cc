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

#include "systems/base/text_key_cursor.h"

#include <string>
#include <vector>

#include "libreallive/gameexe.h"
#include "systems/base/event_system.h"
#include "systems/base/graphics_system.h"
#include "systems/base/surface.h"
#include "systems/base/system.h"
#include "systems/base/text_window.h"

using std::endl;
using std::string;
using std::vector;

// -----------------------------------------------------------------------
// TextKeyCursor
// -----------------------------------------------------------------------
TextKeyCursor::TextKeyCursor(System& system, int in_curosr_number)
    : cursor_number_(in_curosr_number),
      current_frame_(0),
      last_time_frame_incremented_(system.event().GetTicks()),
      system_(system) {
  Gameexe& gexe = system.gameexe();
  GameexeInterpretObject cursor = gexe("CURSOR", in_curosr_number);

  if (cursor("NAME").Exists()) {
    SetCursorImage(system, cursor("NAME"));
    SetCursorSize(cursor("SIZE"));
    SetCursorFrameCount(cursor("CONT"));
    SetCursorFrameSpeed(cursor("SPEED"));
  }
}

// -----------------------------------------------------------------------

TextKeyCursor::~TextKeyCursor() {}

// -----------------------------------------------------------------------

void TextKeyCursor::Execute() {
  unsigned int cur_time = system_.event().GetTicks();

  if (cursor_image_ && last_time_frame_incremented_ + frame_speed_ < cur_time) {
    last_time_frame_incremented_ = cur_time;

    system_.graphics().MarkScreenAsDirty(GUT_TEXTSYS);

    current_frame_++;
    if (current_frame_ >= frame_count_)
      current_frame_ = 0;
  }
}

// -----------------------------------------------------------------------

void TextKeyCursor::Render(TextWindow& text_window, std::ostream* tree) {
  if (cursor_image_) {
    // Get the location to render from text_window
    Point keycur = text_window.KeycursorPosition(frame_size_);

    cursor_image_->RenderToScreen(
        Rect(Point(current_frame_ * frame_size_.width(), 0), frame_size_),
        Rect(keycur, frame_size_),
        255);

    if (tree) {
      *tree << "  Key Cursor #" << cursor_number_ << endl
            << "    Cursor name: " << cursor_image_file_ << endl
            << "    Cursor location: " << Rect(keycur, frame_size_) << endl;
    }
  }
}

// -----------------------------------------------------------------------

void TextKeyCursor::SetCursorImage(System& system, const std::string& name) {
  if (name != "") {
    cursor_image_ = system.graphics().GetSurfaceNamed(name);
    cursor_image_file_ = name;
  } else {
    cursor_image_.reset();
    cursor_image_file_ = "";
  }
}

// -----------------------------------------------------------------------

void TextKeyCursor::SetCursorSize(const std::vector<int>& image_size) {
  frame_size_ = Size(image_size.at(0), image_size.at(1));
}

// -----------------------------------------------------------------------

void TextKeyCursor::SetCursorFrameCount(const int frame_count) {
  frame_count_ = frame_count;
}

// -----------------------------------------------------------------------

void TextKeyCursor::SetCursorFrameSpeed(const int speed) {
  if (frame_count_)
    frame_speed_ = speed / frame_count_;
  else
    frame_speed_ = speed;
}
