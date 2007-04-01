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

#include "Precompiled.hpp"

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
#include <boost/bind.hpp>
#include "Utilities.h"

#include "Modules/utf8.h"
#include "Modules/cp932toUnicode.hpp"
#include "Modules/TextoutLongOperation.hpp"

#include <iostream>
#include <vector>

using namespace std;
using namespace boost;

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

  setNameMod(window("NAME_MOD").to_int(0));

  setWindowWaku(machine, gexe, window("WAKU_SETNO"));

  string filename = findFontFile("msgothic.ttc");
  cerr << "font file: " << filename << endl;
  m_font = TTF_OpenFont(filename.c_str(), fontSizeInPixels());
//"/home/elliot//msgothic.ttc", fontSizeInPixels());
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
  m_currentIndentationInPixels = 0;
  m_currentLineNumber = 0;

  // Allocate the text window surface
  m_surface.reset(new SDLSurface(windowWidth(), windowHeight()));
  m_surface->fill(0, 0, 0, 0);
}

// -----------------------------------------------------------------------

bool SDLTextWindow::displayChar(RLMachine& machine,
                                const std::string& current,
                                const std::string& next)
{
  // If this text page is already full, save some time and reject
  // early.
  if(isFull())
    return false;

  setVisible(true);

  SDL_Color color = {255, 255, 255};
  int curCodepoint = codepoint(current);
  int nextCodepoint = codepoint(next);

  cerr << current << "(" << isKinsoku(curCodepoint) << ") : " << next
       << "(" << isKinsoku(nextCodepoint) << ")" << endl;

  // U+3010 (LEFT BLACK LENTICULAR BRACKET) and U+3011 (RIGHT BLACK
  // LENTICULAR BRACKET) should be handled before this
  // function. Otherwise, it's an error.
  if(curCodepoint == 0x3010 || curCodepoint == 0x3011)
  {
    throw "Bug in parser; \{name} construct should be handled before displayChar";
  }

  SDL_Surface* tmp =
    TTF_RenderUTF8_Blended(m_font, current.c_str(), color);

  // If the width of this glyph plus the spacing will put us over the
  // edge of the window, then line increment.
  //
  // If the current character will fit on this line, and it is NOT
  // in this set, then we should additionally check the next
  // character.  If that IS in this set and will not fit on the
  // current line, then we break the line before the current
  // character instead, to prevent the next character being stranded
  // at the start of a line.
  //
  bool charWillFitOnLine = m_insertionPointX + tmp->w + m_xSpacing <=
    windowWidth();
  bool nextCharWillFitOnLine = m_insertionPointX + 2*(tmp->w + m_xSpacing) <=
    windowWidth();
  if(!charWillFitOnLine || 
     (charWillFitOnLine && !isKinsoku(curCodepoint) &&
      !nextCharWillFitOnLine && isKinsoku(nextCodepoint)))
  {
    hardBrake();

    if(isFull())
      return false;
  }

  // Render glyph to surface
  int w = tmp->w;
  int h = tmp->h;
  m_surface->blitFROMSurface(
    tmp, 0, 0, w, h,
    m_insertionPointX, m_insertionPointY,
    m_insertionPointX + w, m_insertionPointY + h,
    255);

  // Move the insertion point forward one character
  m_insertionPointX += m_fontSizeInPixels + m_xSpacing;

  machine.system().graphics().markScreenAsDirty();

  return true;
}

// -----------------------------------------------------------------------

bool SDLTextWindow::isFull() const
{
  return m_currentLineNumber >= m_yWindowSizeInChars;
}

// -----------------------------------------------------------------------

bool SDLTextWindow::setIndentation()
{
  m_currentIndentationInPixels = m_insertionPointX;
}

// -----------------------------------------------------------------------

void SDLTextWindow::setName(RLMachine& machine, const std::string& utf8name,
                            const std::string& nextChar)
{
  if(m_nameMod == 0)
  {
    // Display the name in one pass
    printTextToFunction(bind(&SDLTextWindow::displayChar, ref(*this), 
                             ref(machine), _1, _2),
                        utf8name, nextChar);
    setIndentation();

    // Check to see if we set the indentation after the 
    string::const_iterator it = nextChar.begin();
    int nextCodepoint = utf8::next(it, nextChar.end());
    if(nextCodepoint == 0x300C || nextCodepoint == 0x300E || 
       nextCodepoint == 0xFF08)
    {
      m_currentIndentationInPixels = m_insertionPointX + m_fontSizeInPixels + 
        m_xSpacing;
    }
  }
  else if(m_nameMod == 1)
  {
    throw "NAME_MOD=1 is unsupported.";
  }
  else if(m_nameMod == 2)
  {
    // Don't print anything for now. Needs to be more robust.
  }
  else
  {
    throw "Invalid";
  }
}

// -----------------------------------------------------------------------

void SDLTextWindow::hardBrake()
{
  m_insertionPointX = m_currentIndentationInPixels;
  m_insertionPointY += (m_fontSizeInPixels + m_ySpacing + m_rubySize);
  m_currentLineNumber++;
}

// -----------------------------------------------------------------------

void SDLTextWindow::resetIndentation()
{
  m_currentIndentationInPixels = 0;
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

    if(m_wakuBacking)
    {
      int backingWidth = m_wakuBacking->width();
      int backingHeight = m_wakuBacking->height();
      m_wakuBacking->renderToScreenAsColorMask(0, 0, backingWidth, backingHeight,
                                               boxX, boxY, boxX + backingWidth,
                                               boxY + backingHeight,
                                               m_r, m_g, m_b, m_alpha, m_filter);
    }

    if(m_wakuMain)
    {
      int mainWidth = m_wakuMain->width();
      int mainHeight = m_wakuMain->height();
      m_wakuMain->renderToScreen(0, 0, mainWidth, mainHeight,
                                 boxX, boxY, boxX + mainWidth,
                                 boxY + mainHeight, 255);
    }

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
  if(name != "")
  {
    GraphicsSystem& gs = machine.system().graphics();
    SDLSurface* s = dynamic_cast<SDLSurface*>(
      gs.loadSurfaceFromFile(findFile(machine, name)));
    m_wakuMain.reset(s);
  }
  else
    m_wakuMain.reset();
}

// -----------------------------------------------------------------------

void SDLTextWindow::setWakuBacking(RLMachine& machine, const std::string& name)
{
  if(name != "")
  {
    GraphicsSystem& gs = machine.system().graphics();
    SDLSurface* s = dynamic_cast<SDLSurface*>(
      gs.loadSurfaceFromFile(findFile(machine, name)));
    s->setIsMask(true);
    m_wakuBacking.reset(s);
  }
  else
    m_wakuBacking.reset();
}

// -----------------------------------------------------------------------

