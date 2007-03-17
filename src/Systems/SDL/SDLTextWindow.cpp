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
#include "Systems/Base/GraphicsSystem.hpp"
#include "Systems/SDL/SDLTextWindow.hpp"
#include "Systems/SDL/SDLSurface.hpp"

#include "MachineBase/RLMachine.hpp"
#include "libReallive/gameexe.h"

#include <SDL/SDL_opengl.h>
#include <SDL/SDL_ttf.h>

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

  m_font = TTF_OpenFont("/Users/elliot/msgothic.ttc", fontSizeInPixels());
  if(m_font == NULL)
  {
    ostringstream oss;
    oss << "Error loading font: " << TTF_GetError();
    throw libReallive::Error(oss.str());
  }

  TTF_SetFontStyle(m_font, TTF_STYLE_NORMAL);

  clearWin();
}

// -----------------------------------------------------------------------

SDLTextWindow::~SDLTextWindow()
{
  TTF_CloseFont(m_font);
}

// -----------------------------------------------------------------------

void SDLTextWindow::clearWin()
{
  m_insertionPointX = 0;
  m_insertionPointY = 0;

  // Allocate the text window surface
  m_surface.reset(new SDLSurface(windowWidth(), windowHeight()));
}

// -----------------------------------------------------------------------

void SDLTextWindow::displayText(RLMachine& machine, const std::string& utf8str)
{
  SDL_Color color = {255, 255, 255};
//  SDL_Color color = {0, 0, 0};
  SDL_Surface* tmp =
    TTF_RenderUTF8_Blended(m_font, utf8str.c_str(), color);

  // If the width of this glyph plus the spacing will put us over the
  // edge of the window, then line increment.
  if(m_insertionPointX + tmp->w + m_xSpacing > windowWidth() )
  {
    m_insertionPointX = 0;
    m_insertionPointY += (tmp->h + m_ySpacing + m_rubySize);
  }

  // Render glyph to surface
  int w = tmp->w;
  int h = tmp->h;
  m_surface->blitFROMSurface(tmp,
                            0, 0, w, h,
                            m_insertionPointX, m_insertionPointY,
                            m_insertionPointX + w, m_insertionPointY + h);

  // Move the insertion point forward one character
  m_insertionPointX += m_fontSizeInPixels + m_xSpacing;

//  m_surface.reset(new SDLSurface(tmp));

//  std::cerr << "Textout: " << utf8str << std::endl;

  machine.system().graphics().markScreenAsDirty();
}

// -----------------------------------------------------------------------

void SDLTextWindow::render(RLMachine& machine)
{
  if(m_surface)
  {
    int width = m_surface->width();
    int height = m_surface->height();
//    cout << "W: " << width << ", H: " << height << endl;

    int x = x1(machine);
    int y = y1(machine);

//     cerr << "{" << 0 << ", " << 0 << ", " << width << ", "
//          << height << "} - {" << x << ", " << y << ", "
//          << x + width << ", " << y + wi
    m_surface->renderToScreen(
      0, 0, width, height,
      x, y, x + width, y + height, 
      255);
  }

//   glBegin(GL_QUADS);
//   {
//     cerr << "RGBA: " << r() << ", " << g() << "," << b() << "," << alpha() << endl;
//     glColor4ub(r(), g(), b(), alpha());
//     cerr << "{" << x1(machine) << "," << y1(machine) << "," << x2(machine)
//          << "," << y2(machine) << "}" << endl;
//     glVertex2i(x1(machine), y1(machine));
//     glVertex2i(x2(machine), y1(machine));
//     glVertex2i(x2(machine), y2(machine));
//     glVertex2i(x1(machine), y2(machine));
//   }
//   glEnd();
}

// -----------------------------------------------------------------------

// void SDLTextWindow::setVal(Gameexe& gexe, int window, const std::string& key,
//                            TextWindowIntSetter setter)
// {
//   if(gexe("WINDOW", window, key).exists())
//     (this->setter)(gexe("WINDOW", window, key));
// }

