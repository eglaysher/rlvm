// -*- Mode: C++; tab-width:2; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi:tw=80:et:ts=2:sts=2
//
// -----------------------------------------------------------------------
//
// This file is part of RLVM, a RealLive virtual machine clone.
//
// -----------------------------------------------------------------------
//
// Copyright (C) 2008 Elliot Glaysher
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

#include "Precompiled.hpp"

// -----------------------------------------------------------------------

#include "Modules/Module_Shk.hpp"

// -----------------------------------------------------------------------

ShakingModule::ShakingModule()
  : RLModule("Shk", 1, 13)
{
  addUnsupportedOpcode(0, 0, "ShakeStop");
  addUnsupportedOpcode(0, 1, "ShakeStop");

  addUnsupportedOpcode(1100, 0, "ShakeScreen=DOWNUP");
  addUnsupportedOpcode(1101, 0, "ShakeScreen=RIGHTLEFT");
  addUnsupportedOpcode(1102, 0, "ShakeScreen2D");
  addUnsupportedOpcode(1200, 0, "ShakeScreen=UP");
  addUnsupportedOpcode(1201, 0, "ShakeScreen=DOWN");
  addUnsupportedOpcode(1202, 0, "ShakeScreen=LEFT");
  addUnsupportedOpcode(1203, 0, "ShakeScreen=RIGHT");
  addUnsupportedOpcode(1300, 0, "ShakeSpec");
  addUnsupportedOpcode(1400, 0, "ShakeScreen=ZOOM");

  addUnsupportedOpcode(3100, 0, "ShakeScreenEx=DOWNUP");
  addUnsupportedOpcode(3101, 0, "ShakeScreenEx=RIGHTLEFT");
  addUnsupportedOpcode(3102, 0, "ShakeScreen2DEx");
  addUnsupportedOpcode(3200, 0, "ShakeScreenEx=UP");
  addUnsupportedOpcode(3201, 0, "ShakeScreenEx=DOWN");
  addUnsupportedOpcode(3202, 0, "ShakeScreenEx=LEFT");
  addUnsupportedOpcode(3203, 0, "ShakeScreenEx=RIGHT");
  addUnsupportedOpcode(3300, 0, "ShakeSpecEx");
  addUnsupportedOpcode(3400, 0, "ShakeScreenEx=ZOOM");
}
