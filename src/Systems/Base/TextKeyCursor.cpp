// -*- Mode: C++; tab-width:2; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi:tw=80:et:ts=2:sts=2
//
// -----------------------------------------------------------------------
//
// This file is part of RLVM, a RealLive virtual machine clone.
//
// -----------------------------------------------------------------------
//
// Copyright (C) 2007 Elliot Glaysher
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
//  
// -----------------------------------------------------------------------

#include "Precompiled.hpp"

// -----------------------------------------------------------------------

#include "TextKeyCursor.hpp"
#include "Utilities.h"

#include "libReallive/gameexe.h"
#include "MachineBase/RLMachine.hpp"
#include "Systems/Base/System.hpp"
#include "Systems/Base/GraphicsSystem.hpp"
#include "Systems/Base/Surface.hpp"
#include "Systems/Base/EventSystem.hpp"
#include "Systems/Base/TextWindow.hpp"

#include <string>

using namespace std;

// -----------------------------------------------------------------------
// TextKeyCursor
// -----------------------------------------------------------------------
TextKeyCursor::TextKeyCursor(RLMachine& machine, int inCurosrNumber)
  : m_cursorNumber(inCurosrNumber), m_currentFrame(0), 
    m_lastTimeFrameIncremented(machine.system().event().getTicks())
{
  Gameexe& gexe = machine.system().gameexe();
  GameexeInterpretObject cursor = gexe("CURSOR", inCurosrNumber);

  setCursorImage(machine, cursor("NAME"));
  setCursorSize(cursor("SIZE"));
  setCursorFrameCount(cursor("CONT"));
  setCursorFrameSpeed(cursor("SPEED"));
}

// -----------------------------------------------------------------------

TextKeyCursor::~TextKeyCursor()
{}

// -----------------------------------------------------------------------

void TextKeyCursor::execute(RLMachine& machine)
{
  unsigned int curTime = machine.system().event().getTicks();

  if(m_lastTimeFrameIncremented + m_frameSpeed < curTime)
  {
    m_lastTimeFrameIncremented = curTime;

    machine.system().graphics().markScreenAsDirty(GUT_TEXTSYS);

    m_currentFrame++;
    if(m_currentFrame >= m_frameCount)
      m_currentFrame = 0;
  }
}

// -----------------------------------------------------------------------

void TextKeyCursor::render(RLMachine& machine, TextWindow& textWindow)
{
  // Get the location to render from textWindow
  int keycurX = textWindow.keycursorX();
  int keycurY = textWindow.keycursorY();

  m_cursorImage->renderToScreen(
    m_currentFrame * m_frameXSize,
    0,
    (m_currentFrame + 1) * m_frameXSize,
    m_frameYSize,
    keycurX, keycurY, keycurX + m_frameXSize, keycurY + m_frameYSize,
    255);
}

// -----------------------------------------------------------------------

void TextKeyCursor::setCursorImage(RLMachine& machine, 
                                   const std::string& name)
{
  if(name != "")
  {
    GraphicsSystem& gs = machine.system().graphics();
    m_cursorImage = 
      gs.loadSurfaceFromFile(findFile(machine, name));
  }
  else
    m_cursorImage.reset();
}

// -----------------------------------------------------------------------

void TextKeyCursor::setCursorSize(const std::vector<int>& imageSize)
{
  m_frameXSize = imageSize.at(0);
  m_frameYSize = imageSize.at(1);
}

// -----------------------------------------------------------------------

void TextKeyCursor::setCursorFrameCount(const int frameCount)
{
  m_frameCount = frameCount;
}

// -----------------------------------------------------------------------

void TextKeyCursor::setCursorFrameSpeed(const int speed)
{
  // What's the divider in this statement? 
  m_frameSpeed = speed / 50;
}
