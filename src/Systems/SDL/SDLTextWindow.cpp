// This file is part of RLVM, a RealLive virtual machine clone.
//
// -----------------------------------------------------------------------
//
// Copyright (C) 2006 El Riot
//  
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
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
//  
// -----------------------------------------------------------------------

/**
 * @file   SDLTextWindow.cpp
 * @author Elliot Glaysher
 * @date   Wed Mar  7 22:11:17 2007
 * 
 * @brief  
 */

#include "Systems/Base/System.hpp"
#include "Systems/SDL/SDLTextWindow.hpp"

#include "MachineBase/RLMachine.hpp"
#include "libReallive/gameexe.h"

#include <SDL/SDL_opengl.h>

#include <boost/function.hpp>

#include <iostream>
#include <vector>

using namespace std;

// -----------------------------------------------------------------------

//void doFunc(Gameexe& gexe, int window, 

// -----------------------------------------------------------------------

SDLTextWindow::SDLTextWindow(RLMachine& machine, int windowNum)
{
  Gameexe& gexe = machine.system().gameexe();

  // Base form for everything to follow.
  GameexeInterpretObject window(gexe("WINDOW", windowNum));

  // Handle: #WINDOW.index.ATTR_MOD, #WINDOW_ATTR, #WINDOW.index.ATTR
  if(window("ATTR_MOD") == 0)
    setRGBAF(gexe("WINDOW_ATTR"));
  else if(window("ATTR_MOD") == 1)
    setRGBAF(window("ATTR"));

  setFontSizeInPixels(window("MOJI_SIZE"));
  setWindowSizeInCharacters(window("MOJI_CNT"));
  setSpacingBetweenCharacters(window("MOJI_REP"));
  setRubyTextSize(window("LUBY_SIZE"));
  setTextboxPadding(window("MOJI_POS"));

  setWindowPosition(window("POS"));

  cerr << "Building whatever!" << endl;
}

// -----------------------------------------------------------------------

void SDLTextWindow::render(RLMachine& machine)
{
  glBegin(GL_QUADS);
  {
    cerr << "RGBA: " << r() << ", " << g() << "," << b() << "," << alpha() << endl;
    glColor4ub(r(), g(), b(), alpha());
    cerr << "{" << x1(machine) << "," << y1(machine) << "," << x2(machine)
         << "," << y2(machine) << "}" << endl;
    glVertex2i(x1(machine), y1(machine));
    glVertex2i(x2(machine), y1(machine));
    glVertex2i(x2(machine), y2(machine));
    glVertex2i(x1(machine), y2(machine));
  }
  glEnd();
}

// -----------------------------------------------------------------------

// void SDLTextWindow::setVal(Gameexe& gexe, int window, const std::string& key,
//                            TextWindowIntSetter setter)
// {
//   if(gexe("WINDOW", window, key).exists())
//     (this->setter)(gexe("WINDOW", window, key));
// }

