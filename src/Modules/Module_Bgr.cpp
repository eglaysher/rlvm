// -*- Mode: C++; tab-width:2; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi:tw=80:et:ts=2:sts=2
//
// -----------------------------------------------------------------------
//
// This file is part of RLVM, a RealLive virtual machine clone.
//
// -----------------------------------------------------------------------
//
// Copyright (C) 2009 Elliot Glaysher
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

#include "Modules/Module_Bgr.hpp"

#include "MachineBase/RLOperation.hpp"
#include "MachineBase/RLOperation/Argc_T.hpp"
#include "MachineBase/RLOperation/Complex_T.hpp"
#include "MachineBase/RLOperation/Special_T.hpp"
#include "Modules/Module_Grp.hpp"

#include <string>

// -----------------------------------------------------------------------

// TODO(erg): Figure out for real how the Bgr module works. For AIR, I'm just
// forwarding bgrMulti to an implementation in Module_Grp.cpp because I don't
// understand how these two systems interlock.

// -----------------------------------------------------------------------

BgrModule::BgrModule()
    : RLModule("Bgr", 1, 40) {
  addUnsupportedOpcode(10, 0, "bgrLoadHaikei");
  addUnsupportedOpcode(10, 1, "bgrLoadHaikei");
  addUnsupportedOpcode(10, 2, "bgrLoadHaikei");

  addUnsupportedOpcode(100, 0, "bgrMulti");
  addOpcode(100, 1, "bgrMulti", makeBgrMulti1());
}
