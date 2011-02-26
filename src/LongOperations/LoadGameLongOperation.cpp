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
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
// -----------------------------------------------------------------------

#include "LongOperations/LoadGameLongOperation.hpp"

#include <boost/shared_ptr.hpp>

#include "Effects/FadeEffect.hpp"
#include "MachineBase/RLMachine.hpp"
#include "Systems/Base/Colour.hpp"
#include "Systems/Base/GraphicsSystem.hpp"
#include "Systems/Base/Surface.hpp"
#include "Systems/Base/System.hpp"

LoadGameLongOperation::LoadGameLongOperation(RLMachine& machine) {
  // Render the current state of the screen
  GraphicsSystem& graphics = machine.system().graphics();

  boost::shared_ptr<Surface> currentWindow = graphics.renderToSurface();
  Size s = currentWindow->size();

  // Blank dc0 (because we won't be using it anyway) for the image
  // we're going to render to
  boost::shared_ptr<Surface> dc0 = graphics.getDC(0);
  dc0->fill(RGBAColour::Black());

  machine.pushLongOperation(this);
  machine.pushLongOperation(
      new FadeEffect(machine, dc0, currentWindow, s, 250));

  // We have our before and after images to use as a transition now. Reset the
  // system to prevent a brief flash of the previous contents of the screen for
  // whatever number of user preceivable milliseconds.
  machine.system().reset();
}

bool LoadGameLongOperation::operator()(RLMachine& machine) {
  load(machine);
  // Warning: the stack has now been nuked and |this| is an invalid.

  // Render the current state of the screen
  GraphicsSystem& graphics = machine.system().graphics();

  boost::shared_ptr<Surface> currentWindow = graphics.renderToSurface();
  Size s = currentWindow->size();

  // Blank dc0 (because we won't be using it anyway) for the image
  // we're going to render to
  boost::shared_ptr<Surface> blankScreen = graphics.buildSurface(s);
  blankScreen->fill(RGBAColour::Black());

  machine.pushLongOperation(
      new FadeEffect(machine, currentWindow, blankScreen, s, 250));

  // At this point, the stack has been nuked, and this current
  // object has already been deleted, leaving an invalid
  // *this. Returning false is the correct thing to do since
  // *returning true will pop an unrelated stack frame.
  return false;
}
