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

#ifndef SRC_LONGOPERATIONS_LOADGAMELONGOPERATION_HPP_
#define SRC_LONGOPERATIONS_LOADGAMELONGOPERATION_HPP_

#include "MachineBase/LongOperation.hpp"

class RLMachine;

// Clears the screen and resets the system, performs the specified load
// operation, and then fades back in. We let subclasses define what loading
// means.
//
// Internally, load is fairly complex; we very carefully time adding other
// LongOperations to the stack to perform sub tasks like clearing the screen
// etc because we can't rely on normal flow control because we're going to nuke
// the call stack.
struct LoadGameLongOperation : public LongOperation {
  // Sets the callstack up so that we will fade to black, clear the screen and
  // render, and then enter the next stage. This runs in the during our
  // caller's RLOperation.
  //
  // WARNING: This constructor adds itself to the machine's stack. If I come
  // back, trying to clean this up, make sure that fadeout on selecting a game
  // to load still works.
  LoadGameLongOperation(RLMachine& machine);

  // Actually loads the data and sets up a LongOperation to fade into our new
  // screen. By the time this method returns, the game state has been thawed
  // and the previous stack that ran us is invalid.
  virtual bool operator()(RLMachine& machine);

  // Load operation to be specified by subclasses.
  virtual void load(RLMachine& machine) = 0;
};

#endif  // SRC_LONGOPERATIONS_LOADGAMELONGOPERATION_HPP_
