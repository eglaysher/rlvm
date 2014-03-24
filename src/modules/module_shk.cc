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
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
// -----------------------------------------------------------------------

#include "modules/module_shk.h"

ShakingModule::ShakingModule() : RLModule("Shk", 1, 13) {
  AddUnsupportedOpcode(0, 0, "ShakeStop");
  AddUnsupportedOpcode(0, 1, "ShakeStop");

  AddUnsupportedOpcode(1100, 0, "ShakeScreen=DOWNUP");
  AddUnsupportedOpcode(1101, 0, "ShakeScreen=RIGHTLEFT");
  AddUnsupportedOpcode(1102, 0, "ShakeScreen2D");
  AddUnsupportedOpcode(1200, 0, "ShakeScreen=UP");
  AddUnsupportedOpcode(1201, 0, "ShakeScreen=DOWN");
  AddUnsupportedOpcode(1202, 0, "ShakeScreen=LEFT");
  AddUnsupportedOpcode(1203, 0, "ShakeScreen=RIGHT");
  AddUnsupportedOpcode(1300, 0, "ShakeSpec");
  AddUnsupportedOpcode(1400, 0, "ShakeScreen=ZOOM");

  AddUnsupportedOpcode(3100, 0, "ShakeScreenEx=DOWNUP");
  AddUnsupportedOpcode(3101, 0, "ShakeScreenEx=RIGHTLEFT");
  AddUnsupportedOpcode(3102, 0, "ShakeScreen2DEx");
  AddUnsupportedOpcode(3200, 0, "ShakeScreenEx=UP");
  AddUnsupportedOpcode(3201, 0, "ShakeScreenEx=DOWN");
  AddUnsupportedOpcode(3202, 0, "ShakeScreenEx=LEFT");
  AddUnsupportedOpcode(3203, 0, "ShakeScreenEx=RIGHT");
  AddUnsupportedOpcode(3300, 0, "ShakeSpecEx");
  AddUnsupportedOpcode(3400, 0, "ShakeScreenEx=ZOOM");
}
