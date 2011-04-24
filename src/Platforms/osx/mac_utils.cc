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

#include "Platforms/osx/mac_utils.h"

#include <SDL/SDL.h>

#include "MachineBase/LongOperation.hpp"
#include "MachineBase/RLMachine.hpp"
#include "Systems/Base/System.hpp"
#include "Systems/Base/GraphicsSystem.hpp"

namespace {

struct CursorResetter : public LongOperation {
 public:
  bool operator()(RLMachine& machine) {
    bool custom = machine.system().graphics().useCustomCursor();

    // Cocoa has put mouse cursor in a bad state. I could fix this if I had
    // access to the SDL_Quartz layer, but I don't so flipping the cursor bit
    // back and forth also works.
    SDL_ShowCursor(custom ? SDL_ENABLE : SDL_DISABLE);
    SDL_ShowCursor(custom ? SDL_DISABLE : SDL_ENABLE);

    return true;
  }
};

}  // namespace

namespace mac_utils {

void ResetCursor(RLMachine* machine) {
  machine->pushLongOperation(new CursorResetter);
}

void PauseExecution(RLMachine& machine) {
  machine.pauseExecution();
  // On OSX, we need to manually draw the next frame to hide the cursor
  machine.system().graphics().refresh(NULL);
}

}  // namespace mac_utils;
