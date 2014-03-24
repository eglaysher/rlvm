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

#include "modules/module_shl.h"

LayeredShakingModule::LayeredShakingModule() : RLModule("Shl", 1, 12) {
  AddUnsupportedOpcode(0, 0, "ShakeLayersStop");
  AddUnsupportedOpcode(0, 1, "ShakeLayersStop");
  AddUnsupportedOpcode(0, 2, "ShakeLayersStop");

  AddUnsupportedOpcode(1100, 0, "ShakeLayers=DOWNUP");
  AddUnsupportedOpcode(1100, 1, "ShakeLayers=DOWNUP");
  AddUnsupportedOpcode(1101, 0, "ShakeLayers=RIGHTLEFT");
  AddUnsupportedOpcode(1101, 1, "ShakeLayers=RIGHTLEFT");

  AddUnsupportedOpcode(1102, 0, "ShakeLayers2D");
  AddUnsupportedOpcode(1102, 1, "ShakeLayers2D");

  AddUnsupportedOpcode(1200, 0, "ShakeLayers=UP");
  AddUnsupportedOpcode(1200, 1, "ShakeLayers=UP");
  AddUnsupportedOpcode(1201, 0, "ShakeLayers=DOWN");
  AddUnsupportedOpcode(1201, 1, "ShakeLayers=DOWN");
  AddUnsupportedOpcode(1202, 0, "ShakeLayers=LEFT");
  AddUnsupportedOpcode(1202, 1, "ShakeLayers=LEFT");
  AddUnsupportedOpcode(1203, 0, "ShakeLayers=RIGHT");
  AddUnsupportedOpcode(1203, 1, "ShakeLayers=RIGHT");

  AddUnsupportedOpcode(1300, 0, "ShakeLayersSpecEx");
  AddUnsupportedOpcode(1300, 1, "ShakeLayersSpecEx");

  AddUnsupportedOpcode(3100, 0, "ShakeLayersEx=DOWNUP");
  AddUnsupportedOpcode(3100, 1, "ShakeLayersEx=DOWNUP");
  AddUnsupportedOpcode(3101, 0, "ShakeLayersEx=RIGHTLEFT");
  AddUnsupportedOpcode(3101, 1, "ShakeLayersEx=RIGHTLEFT");

  AddUnsupportedOpcode(3102, 0, "ShakeLayers2DEx");
  AddUnsupportedOpcode(3102, 1, "ShakeLayers2DEx");

  AddUnsupportedOpcode(3200, 0, "ShakeLayersEx=UP");
  AddUnsupportedOpcode(3200, 1, "ShakeLayersEx=UP");
  AddUnsupportedOpcode(3201, 0, "ShakeLayersEx=DOWN");
  AddUnsupportedOpcode(3201, 1, "ShakeLayersEx=DOWN");
  AddUnsupportedOpcode(3202, 0, "ShakeLayersEx=LEFT");
  AddUnsupportedOpcode(3202, 1, "ShakeLayersEx=LEFT");
  AddUnsupportedOpcode(3203, 0, "ShakeLayersEx=RIGHT");
  AddUnsupportedOpcode(3203, 1, "ShakeLayersEx=RIGHT");

  AddUnsupportedOpcode(3300, 0, "ShakeLayersSpecEx");
  AddUnsupportedOpcode(3300, 1, "ShakeLayersSpecEx");
}
