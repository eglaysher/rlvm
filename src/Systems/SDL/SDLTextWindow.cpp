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
#include "Utilities.h"

#include "Modules/utf8.h"

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

  // INDENT_USE appears to default to on. See the first scene in the
  // game with Nagisa, paying attention to indentation; then check the
  // Gameexe.ini.
  setUseIndentation(window("INDENT_USE").to_int(1));

  setWindowWaku(machine, gexe, window("WAKU_SETNO"));

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
  m_surface->fill(0, 0, 0, 0);
}

// -----------------------------------------------------------------------

void SDLTextWindow::displayText(RLMachine& machine, const std::string& utf8str)
{
  setVisible(true);
  SDL_Color color = {255, 255, 255};
//  SDL_Color color = {0, 0, 0};

  // Iterate over each incoming character to display (we do this
  // instead of rendering the entire string so that we can perform
  // indentation, et cetera.)
  string::const_iterator cur = utf8str.begin();
  string::const_iterator tmp = cur;
  string::const_iterator end = utf8str.end();
  for(;tmp != end; cur = tmp)
  {
    int codepoint = utf8::next(tmp, end);
    string ch(cur, tmp);

//    cerr << ch << ":" << codepoint << endl;

    // For now, ignore U+3010 (LEFT BLACK LENTICULAR BRACKET) and
    // U+3011 (RIGHT BLACK LENTICULAR BRACKET). When I come back and
    // do name boxes, I'll have to change this.
    if(codepoint == 0x3010)
      continue;
    if(codepoint == 0x3011)
      continue;

    SDL_Surface* tmp =
      TTF_RenderUTF8_Blended(m_font, ch.c_str(), color);

    // If the width of this glyph plus the spacing will put us over the
    // edge of the window, then line increment.
    //
    // @todo Kinsoku not implemented.  The kinsoku feature is as
    // follows: the following characters are considered `special' when
    // wrapping lines automatically:
    //
    // { 0x0021, 0x0022, 0x0027, 0x0029, 0x002c, 0x002e, 0x003a,
    //   0x003b, 0x003e, 0x003f, 0x005d, 0x007d, 0x2019, 0x201d,
    //   0x2025, 0x2026, 0x3001, 0x3002, 0x3009, 0x300b, 0x300d,
    //   0x300f, 0x3011, 0x301f, 0x3041, 0x3043, 0x3045, 0x3047,
    //   0x3049, 0x3063, 0x3083, 0x3085, 0x3087, 0x308e, 0x30a1,
    //   0x30a3, 0x30a5, 0x30a7, 0x30a9, 0x30c3, 0x30e3, 0x30e5,
    //   0x30e7, 0x30ee, 0x30f5, 0x30f6, 0x30fb, 0x30fc, 0xff01,
    //   0xff09, 0xff0c, 0xff0e, 0xff1a, 0xff1b, 0xff1f, 0xff3d,
    //   0xff5d, 0xff5e, 0xff61, 0xff63, 0xff64, 0xff65, 0xff67,
    //   0xff68, 0xff69, 0xff6a, 0xff6b, 0xff6c, 0xff6d, 0xff6e,
    //   0xff6f, 0xff70, 0xff9e, 0xff9f }
    //
    // If the current character will fit on this line, and it is NOT
    // in this set, then we should additionally check the next
    // character.  If that IS in this set and will not fit on the
    // current line, then we break the line before the current
    // character instead, to prevent the next character being stranded
    // at the start of a line.
    //
    if(m_insertionPointX + tmp->w + m_xSpacing > windowWidth() )
    {
//      cerr << "Going onto new line..." << endl;
      m_insertionPointX = m_currentIndentationInPixels;
      m_insertionPointY += (tmp->h + m_ySpacing + m_rubySize);
    }

    // Render glyph to surface
    int w = tmp->w;
    int h = tmp->h;
    m_surface->blitFROMSurface(tmp,
                               0, 0, w, h,
                               m_insertionPointX, m_insertionPointY,
                               m_insertionPointX + w, m_insertionPointY + h,
                               255);

    // Move the insertion point forward one character
    m_insertionPointX += m_fontSizeInPixels + m_xSpacing;

    // After the insertion point has been moved, check if this is a
    // special indentation mark
    if(m_useIndentation)
    {
      // If this is U+300C (LEFT CORNER BRACKET), and we're set to do
      // indentation, then set the indentation
      if(codepoint == 0x300C)
        m_currentIndentationInPixels = m_insertionPointX;

      // If this is U+300D (RIGHT CORNER BRACKET), reset the indentation
      if(codepoint == 0x300D)
        m_currentIndentationInPixels = 0;
    }
  }
  machine.system().graphics().markScreenAsDirty();
}

// -----------------------------------------------------------------------

/** 
 * @todo Make this pass the #WINDOW_ATTR color off wile rendering the
 *       wakuBacking.
 */
void SDLTextWindow::render(RLMachine& machine)
{
  if(m_surface && isVisible())
  {
    GraphicsSystem& gs = machine.system().graphics();

    int width = m_surface->width();
    int height = m_surface->height();

    int boxX = boxX1();
    int boxY = boxY1();

    int backingWidth = m_wakuBacking->width();
    int backingHeight = m_wakuBacking->height();
    m_wakuBacking->renderToScreenAsColorMask(0, 0, backingWidth, backingHeight,
                                             boxX, boxY, boxX + backingWidth,
                                             boxY + backingHeight,
                                             m_r, m_g, m_b, m_alpha);

    int mainWidth = m_wakuMain->width();
    int mainHeight = m_wakuMain->height();
    m_wakuMain->renderToScreen(0, 0, mainWidth, mainHeight,
                                  boxX, boxY, boxX + mainWidth,
                                  boxY + mainHeight, 255);

//     cerr << "{" << 0 << ", " << 0 << ", " << width << ", "
//          << height << "} - {" << x << ", " << y << ", "
//          << x + width << ", " << y + wi

    int x = textX1(machine);
    int y = textY1(machine);
    m_surface->renderToScreen(
      0, 0, width, height,
      x, y, x + width, y + height, 
      255);
  }
}

// -----------------------------------------------------------------------

void SDLTextWindow::setWakuMain(RLMachine& machine, const std::string& name)
{
  GraphicsSystem& gs = machine.system().graphics();
  SDLSurface* s = dynamic_cast<SDLSurface*>(
    gs.loadSurfaceFromFile(findFile(machine, name)));
  m_wakuMain.reset(s);
}

// -----------------------------------------------------------------------

void SDLTextWindow::setWakuBacking(RLMachine& machine, const std::string& name)
{
  GraphicsSystem& gs = machine.system().graphics();
  SDLSurface* s = dynamic_cast<SDLSurface*>(
    gs.loadSurfaceFromFile(findFile(machine, name)));
  s->setIsMask(true);
  m_wakuBacking.reset(s);
}
