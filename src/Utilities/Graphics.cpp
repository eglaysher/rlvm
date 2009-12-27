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

#include "Utilities/Graphics.hpp"

#include <sstream>
#include <vector>
#include <boost/assign/list_of.hpp>

#include "MachineBase/RLMachine.hpp"
#include "Systems/Base/Rect.hpp"
#include "Systems/Base/System.hpp"
#include "Systems/Base/SystemError.hpp"
#include "libReallive/gameexe.h"

using namespace std;
using namespace boost::assign;

// -----------------------------------------------------------------------

std::vector<int> getSELEffect(RLMachine& machine, int selNum) {
  Gameexe& gexe = machine.system().gameexe();
  vector<int> selEffect;

  if (gexe("SEL", selNum).exists()) {
    selEffect = gexe("SEL", selNum).to_intVector();
    grpToRecCoordinates(selEffect[0], selEffect[1],
                        selEffect[2], selEffect[3]);
  } else if (gexe("SELR", selNum).exists()) {
    selEffect = gexe("SELR", selNum).to_intVector();
  } else {
    // Can't find the specified #SEL effect. See if there's a #SEL.000 effect:
    if (gexe("SEL", 0).exists()) {
      selEffect = gexe("SEL", 0).to_intVector();
      grpToRecCoordinates(selEffect[0], selEffect[1],
                          selEffect[2], selEffect[3]);
    } else if (gexe("SELR", 0).exists()) {
      selEffect = gexe("SELR", 0).to_intVector();
    } else {
      // Crap! Couldn't fall back on the default one either, so instead return
      // a SEL vector that is a screenwide, short fade because we absolutely
      // can't fail here.
      Size screen = getScreenSize(gexe);
      selEffect = list_of(0)(0)(screen.width())(screen.height())(0)(0)(1000)
                  (000)(0)(0)(0)(0)(0)(0)(255)(0);
    }
  }

  return selEffect;
}

// -----------------------------------------------------------------------

void getSELPointAndRect(RLMachine& machine, int selNum, Rect& rect,
                        Point& point) {
  vector<int> selEffect = getSELEffect(machine, selNum);
  rect = Rect::REC(selEffect[0], selEffect[1], selEffect[2], selEffect[3]);
  point = Point(selEffect[4], selEffect[5]);
}

// -----------------------------------------------------------------------

Size getScreenSize(Gameexe& gameexe) {
  int graphicsMode = gameexe("SCREENSIZE_MOD");
  if (graphicsMode == 0) {
    return Size(640, 480);
  } else if (graphicsMode == 1) {
    return Size(800, 600);
  } else {
    ostringstream oss;
    oss << "Illegal #SCREENSIZE_MOD value: " << graphicsMode << endl;
    throw SystemError(oss.str());
  }
}

// -----------------------------------------------------------------------

void clamp(float& var, float min, float max) {
  if (var < min)
    var = min;
  else if (var > max)
    var = max;
}
