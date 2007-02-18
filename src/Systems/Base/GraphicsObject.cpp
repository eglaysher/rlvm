// This file is part of RLVM, a RealLive virutal machine clone.
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

#include "GraphicsObject.hpp"

#include <iostream>
#include <algorithm>

using namespace std;

GraphicsObject::GraphicsObject()
  : m_visible(false), 
    m_x(0), m_y(0), 
    m_whateverAdjustVertOperatesOn(0),
    m_originX(0), m_originY(0),
    m_repOriginX(0), m_repOriginY(0),

    // Width and height are percentages
    m_width(100), m_height(100), 

    m_rotation(0),

    m_pattNo(0), m_alpha(255), m_mono(0), m_invert(0),
    // Do the rest later.
    m_tintR(255), m_tintG(255), m_tintB(255), 
    m_colourR(255), m_colourG(255), m_colourB(255), m_colourLevel(255),
    m_wipeCopy(0)
{
  // Regretfully, we can't do this in the initializer list.
  fill(m_adjustX, m_adjustX + 8, 0);
  fill(m_adjustY, m_adjustY + 8, 0);
}

GraphicsObject::GraphicsObject(const GraphicsObject& rhs)
  :  m_visible(rhs.m_visible), m_x(rhs.m_x), m_y(rhs.m_y),
    m_whateverAdjustVertOperatesOn(rhs.m_whateverAdjustVertOperatesOn),
    m_originX(rhs.m_originX), m_originY(rhs.m_originY), 
    m_repOriginX(rhs.m_repOriginX), m_repOriginY(rhs.m_repOriginY),
    m_width(rhs.m_width), m_rotation(rhs.m_rotation),
    m_pattNo(rhs.m_pattNo), m_alpha(rhs.m_alpha),
    m_mono(rhs.m_mono), m_invert(rhs.m_invert),
    m_light(rhs.m_light), m_tintR(rhs.m_tintR),
    m_tintG(rhs.m_tintG), m_tintB(rhs.m_tintB),
    m_colourR(rhs.m_colourR), m_colourG(rhs.m_colourG),
    m_colourB(rhs.m_colourB), m_colourLevel(rhs.m_colourLevel),
    m_compositeMode(rhs.m_compositeMode),
    m_scrollRateX(rhs.m_scrollRateX),
     m_scrollRateY(rhs.m_scrollRateY), m_wipeCopy(0)
{   
  if(rhs.m_objectData)
    m_objectData.reset(rhs.m_objectData->clone());

  copy(rhs.m_adjustX, rhs.m_adjustX + 8, m_adjustX);
  copy(rhs.m_adjustY, rhs.m_adjustY + 8, m_adjustY);
}

// -----------------------------------------------------------------------

GraphicsObject& GraphicsObject::operator=(const GraphicsObject& rhs)
{
  if(this != &rhs)
  {
    cerr << "Copying graphics object!" << endl;

    m_visible = rhs.m_visible;
    m_x = rhs.m_x;
    m_y = rhs.m_y;

    copy(rhs.m_adjustX, rhs.m_adjustX + 8, m_adjustX);
    copy(rhs.m_adjustY, rhs.m_adjustY + 8, m_adjustY);

    m_whateverAdjustVertOperatesOn = rhs.m_whateverAdjustVertOperatesOn;
    m_originX = rhs.m_originX;
    m_originY = rhs.m_originY;
    m_repOriginX = rhs.m_repOriginX;
    m_repOriginY = rhs.m_repOriginY;
    m_width = rhs.m_width;
    m_rotation = rhs.m_rotation;

    m_pattNo = rhs.m_pattNo;
    m_alpha = rhs.m_alpha;
    m_mono = rhs.m_mono;
    m_invert = rhs.m_invert;
    m_light = rhs.m_light;
    m_tintR = rhs.m_tintR;
    m_tintG = rhs.m_tintG;
    m_tintB = rhs.m_tintB;

    m_colourR = rhs.m_colourR;
    m_colourG = rhs.m_colourG;
    m_colourB = rhs.m_colourB;
    m_colourLevel = rhs.m_colourLevel;

    m_compositeMode = rhs.m_compositeMode;
    m_scrollRateX = rhs.m_scrollRateX;
    m_scrollRateY = rhs.m_scrollRateY;

    m_wipeCopy = rhs.m_wipeCopy;

    if(rhs.m_objectData)
      m_objectData.reset(rhs.m_objectData->clone());
  }

  return *this;
}

// -----------------------------------------------------------------------

void GraphicsObject::render(RLMachine& machine) {
  if(m_objectData && visible()) {
//    cerr << "Rendering object!" << endl;
    m_objectData->render(machine, *this);
  }
//  else {
//    cerr << "NOT rendering object!" << endl;
//  }
}

// -----------------------------------------------------------------------

void GraphicsObject::deleteObject()
{
  m_objectData.reset();
}

// -----------------------------------------------------------------------

void GraphicsObject::clearObject()
{
  *this = GraphicsObject();
}
