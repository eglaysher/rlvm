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
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
// -----------------------------------------------------------------------

#include "utilities/graphics.h"

#include <iostream>
#include <sstream>
#include <vector>

#include "libreallive/gameexe.h"
#include "machine/rlmachine.h"
#include "systems/base/rect.h"
#include "systems/base/system.h"
#include "systems/base/system_error.h"

std::vector<int> GetSELEffect(RLMachine& machine, int selNum) {
  Gameexe& gexe = machine.system().gameexe();
  std::vector<int> selEffect;

  if (gexe("SEL", selNum).Exists()) {
    selEffect = gexe("SEL", selNum).ToIntVector();
    grp_to_rec_coordinates(selEffect[0], selEffect[1], selEffect[2], selEffect[3]);
  } else if (gexe("SELR", selNum).Exists()) {
    selEffect = gexe("SELR", selNum).ToIntVector();
  } else {
    // Can't find the specified #SEL effect. See if there's a #SEL.000 effect:
    if (gexe("SEL", 0).Exists()) {
      selEffect = gexe("SEL", 0).ToIntVector();
      grp_to_rec_coordinates(
          selEffect[0], selEffect[1], selEffect[2], selEffect[3]);
    } else if (gexe("SELR", 0).Exists()) {
      selEffect = gexe("SELR", 0).ToIntVector();
    } else {
      // Crap! Couldn't fall back on the default one either, so instead return
      // a SEL vector that is a screenwide, short fade because we absolutely
      // can't fail here.
      Size screen = GetScreenSize(gexe);
      selEffect = {0, 0, screen.width(), screen.height(), 0, 0, 1000, 000,
                   0, 0, 0,              0,               0, 0, 255,  0};
    }
  }

  return selEffect;
}

void GetSELPointAndRect(RLMachine& machine,
                        int selNum,
                        Rect& rect,
                        Point& point) {
  std::vector<int> selEffect = GetSELEffect(machine, selNum);
  rect = Rect::REC(selEffect[0], selEffect[1], selEffect[2], selEffect[3]);
  point = Point(selEffect[4], selEffect[5]);
}

Size GetScreenSize(Gameexe& gameexe) {
  std::vector<int> graphicsMode = gameexe("SCREENSIZE_MOD");
  if (graphicsMode.size()) {
    if (graphicsMode[0] == 0) {
      return Size(640, 480);
    } else if (graphicsMode[0] == 1) {
      return Size(800, 600);
    } else if (graphicsMode[0] == 999 && graphicsMode.size() >= 3) {
      return Size(graphicsMode[1], graphicsMode[2]);
    } else {
      std::ostringstream oss;
      oss << "Illegal #SCREENSIZE_MOD value: " << graphicsMode[0];
      throw SystemError(oss.str());
    }
  }

  std::ostringstream oss;
  oss << "Missing #SCREENSIZE_MOD key";
  throw SystemError(oss.str());
}

void Clamp(float& var, float min, float max) {
  if (var < min)
    var = min;
  else if (var > max)
    var = max;
}

void ClipDestination(const Rect& clip_rect, Rect& src, Rect& dest) {
  Rect intersection = clip_rect.Intersection(dest);
  if (intersection.size().is_empty()) {
    src = Rect();
    dest = Rect();
    return;
  }

  // TODO(erg): Doesn't deal with clipping the right side because we don't
  // really want an intersection here; we want it clipped to.

  if (src.size() == dest.size()) {
    Size top_left_offset = intersection.origin() - dest.origin();
    dest = intersection;
    src = Rect(src.origin() + top_left_offset, intersection.size());
  } else {
    std::cerr << "Doesn't deal with different sizes in ClipDestination!"
              << std::endl;
  }
}
