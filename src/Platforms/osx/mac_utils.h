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

#ifndef SRC_PLATFORMS_OSX_MAC_UTILS_H_
#define SRC_PLATFORMS_OSX_MAC_UTILS_H_

class RLMachine;

// Utilities where C++ code would conflict with an Objective-C symbol.
namespace mac_utils {

// The Quartz SDL code gets confused when the right click menu takes over. We
// need to reset whatever state it's in. Returns whether we're using a custom
// cursor so the AppKit half of this can do it's part too.
void ResetCursor(RLMachine* machine);

// Pauses execution and then forces drawing of the next frame.
void PauseExecution(RLMachine& machine);

}  // namespace mac_utils;

#endif  // SRC_PLATFORMS_OSX_MAC_UTILS_H_
