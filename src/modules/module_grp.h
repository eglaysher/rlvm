// -*- Mode: C++; tab-width:2; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi:tw=80:et:ts=2:sts=2
//
// -----------------------------------------------------------------------
//
// This file is part of RLVM, a RealLive virtual machine clone.
//
// -----------------------------------------------------------------------
//
// Copyright (C) 2006, 2007 Elliot Glaysher
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

#ifndef SRC_MODULES_MODULE_GRP_H_
#define SRC_MODULES_MODULE_GRP_H_

#include <deque>
#include <string>
#include <vector>

#include "machine/mapped_rlmodule.h"
#include "machine/rloperation.h"

class GraphicsStackFrame;

// Contains functions for mod<1:33>, Grp.
class GrpModule : public MappedRLModule {
 public:
  GrpModule();
};

// An adapter for MappedRLModules that records the incoming command into the
// graphics stack.
RLOperation* GraphicsStackMappingFun(RLOperation* op);

// -----------------------------------------------------------------------

// Replays the new Graphics stack, string representations of reallive bytecode.
void ReplayGraphicsStackCommand(RLMachine& machine,
                                const std::deque<std::string>& stack);

// Replays the serialized graphics stack; this should put the graphics
// DCs in the same state as they were before the game was saved.
//
// If the command has no transition effect, we can safely call it, but
// we often have to call the implementation function so that we don't
// display all the transition effects that happened since the last
// stackTrunc on load.
void ReplayDepricatedGraphicsStackVector(
    RLMachine& machine,
    const std::vector<GraphicsStackFrame>& serializedStack);

#endif  // SRC_MODULES_MODULE_GRP_H_
