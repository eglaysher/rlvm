// -*- Mode: C++; tab-width:2; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi:tw=80:et:ts=2:sts=2
//
// -----------------------------------------------------------------------
//
// This file is part of RLVM, a RealLive virtual machine clone.
//
// -----------------------------------------------------------------------
//
// Copyright (C) 2011 Elliot Glaysher
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
// -----------------------------------------------------------------------

#include "long_operations/load_game_long_operation.h"

#include <memory>

#include "effects/fade_effect.h"
#include "machine/rlmachine.h"
#include "systems/base/colour.h"
#include "systems/base/graphics_system.h"
#include "systems/base/surface.h"
#include "systems/base/system.h"

LoadGameLongOperation::LoadGameLongOperation(RLMachine& machine) {
  // Render the current state of the screen
  GraphicsSystem& graphics = machine.system().graphics();

  std::shared_ptr<Surface> currentWindow = graphics.RenderToSurface();
  Size s = currentWindow->GetSize();

  // Blank dc0 (because we won't be using it anyway) for the image
  // we're going to render to
  std::shared_ptr<Surface> dc0 = graphics.GetDC(0);
  dc0->Fill(RGBAColour::Black());

  machine.PushLongOperation(this);
  machine.PushLongOperation(
      new FadeEffect(machine, dc0, currentWindow, s, 250));

  // We have our before and after images to use as a transition now. Reset the
  // system to prevent a brief flash of the previous contents of the screen for
  // whatever number of user preceivable milliseconds.
  machine.system().Reset();
}

bool LoadGameLongOperation::operator()(RLMachine& machine) {
  Load(machine);
  // Warning: the stack has now been nuked and |this| is an invalid.

  // Render the current state of the screen
  GraphicsSystem& graphics = machine.system().graphics();

  std::shared_ptr<Surface> currentWindow = graphics.RenderToSurface();
  Size s = currentWindow->GetSize();

  // Blank dc0 (because we won't be using it anyway) for the image
  // we're going to render to
  std::shared_ptr<Surface> blankScreen = graphics.BuildSurface(s);
  blankScreen->Fill(RGBAColour::Black());

  machine.PushLongOperation(
      new FadeEffect(machine, currentWindow, blankScreen, s, 250));

  // At this point, the stack has been nuked, and this current
  // object has already been deleted, leaving an invalid
  // *this. Returning false is the correct thing to do since
  // *returning true will pop an unrelated stack frame.
  return false;
}
