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

#include "Precompiled.hpp"

// -----------------------------------------------------------------------

#include "TextKeyCursor.hpp"
#include "Utilities.h"

#include "libReallive/gameexe.h"
#include "MachineBase/RLMachine.hpp"
#include "Systems/Base/System.hpp"
#include "Systems/Base/GraphicsSystem.hpp"
#include "Systems/Base/Surface.hpp"
#include "Systems/Base/EventSystem.hpp"
#include "Systems/Base/TextWindow.hpp"

#include <string>

using namespace std;

// -----------------------------------------------------------------------
// TextKeyCursor
// -----------------------------------------------------------------------
TextKeyCursor::TextKeyCursor(RLMachine& machine, int in_curosr_number)
  : cursor_number_(in_curosr_number), current_frame_(0),
    last_time_frame_incremented_(machine.system().event().getTicks())
{
  Gameexe& gexe = machine.system().gameexe();
  GameexeInterpretObject cursor = gexe("CURSOR", in_curosr_number);

  setCursorImage(machine, cursor("NAME"));
  setCursorSize(cursor("SIZE"));
  setCursorFrameCount(cursor("CONT"));
  setCursorFrameSpeed(cursor("SPEED"));
}

// -----------------------------------------------------------------------

TextKeyCursor::~TextKeyCursor()
{}

// -----------------------------------------------------------------------

void TextKeyCursor::execute(RLMachine& machine)
{
  unsigned int cur_time = machine.system().event().getTicks();

  if(last_time_frame_incremented_ + frame_speed_ < cur_time)
  {
    last_time_frame_incremented_ = cur_time;

    machine.system().graphics().markScreenAsDirty(GUT_TEXTSYS);

    current_frame_++;
    if(current_frame_ >= frame_count_)
      current_frame_ = 0;
  }
}

// -----------------------------------------------------------------------

void TextKeyCursor::render(RLMachine& machine, TextWindow& text_window)
{
  if(cursor_image_) {
    // Get the location to render from text_window
    Point keycur = text_window.keycursorPosition();

    cursor_image_->renderToScreen(
      Rect(Point(current_frame_ * frame_size_.width(), 0), frame_size_),
      Rect(keycur, frame_size_),
      255);
  }
}

// -----------------------------------------------------------------------

void TextKeyCursor::setCursorImage(RLMachine& machine,
                                   const std::string& name)
{
  if(name != "")
  {
    GraphicsSystem& gs = machine.system().graphics();
    cursor_image_ =
      gs.loadSurfaceFromFile(findFile(machine, name));
  }
  else
    cursor_image_.reset();
}

// -----------------------------------------------------------------------

void TextKeyCursor::setCursorSize(const std::vector<int>& image_size)
{
  frame_size_ = Size(image_size.at(0), image_size.at(1));
}

// -----------------------------------------------------------------------

void TextKeyCursor::setCursorFrameCount(const int frame_count)
{
  frame_count_ = frame_count;
}

// -----------------------------------------------------------------------

void TextKeyCursor::setCursorFrameSpeed(const int speed)
{
  // What's the divider in this statement?
  frame_speed_ = speed / 50;
}
