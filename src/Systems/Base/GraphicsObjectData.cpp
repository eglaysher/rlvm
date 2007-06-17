// This file is part of RLVM, a RealLive virtual machine clone.
//
// -----------------------------------------------------------------------
//
// Copyright (C) 2006 Elliot Glaysher
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

#include "Systems/Base/GraphicsObjectData.hpp"

// -----------------------------------------------------------------------
// GraphicsObjectData
// -----------------------------------------------------------------------

GraphicsObjectData::~GraphicsObjectData() { }

// -----------------------------------------------------------------------

bool GraphicsObjectData::isAnimation() const
{   
  return false; 
}

// -----------------------------------------------------------------------
// AnimatedObjectData
// -----------------------------------------------------------------------

AnimatedObjectData::AnimatedObjectData()
  : m_afterAnimation(AFTER_NONE), m_objectToCleanupOn(NULL)
{}

// -----------------------------------------------------------------------

AnimatedObjectData::~AnimatedObjectData()
{}

// -----------------------------------------------------------------------

bool AnimatedObjectData::isAnimation() const
{
  return true;
}

// -----------------------------------------------------------------------
// GraphicsObjectOverride
// -----------------------------------------------------------------------

GraphicsObjectOverride::GraphicsObjectOverride()
  : overrideSource(false), hasDestOffset(false), hasAlphaOverride(false), 
    srcX1(-1), srcY1(-1), srcX2(-1), srcY2(-1),
    dstX(-1), dstY(-1), alpha(-1)
{
}

// -----------------------------------------------------------------------

void GraphicsObjectOverride::setOverrideSource(
  int insrcX1, int insrcY1, int insrcX2, int insrcY2)
{
  overrideSource = true;
  srcX1 = insrcX1; srcY1 = insrcY1; srcX2 = insrcX2; srcY2 = insrcY2;
}

// -----------------------------------------------------------------------

void GraphicsObjectOverride::setDestOffset(int indstX, int indstY)
{
  hasDestOffset = true;
  dstX = indstX; dstY = indstY;
}

// -----------------------------------------------------------------------

void GraphicsObjectOverride::setAlphaOverride(int inalpha)
{
  hasAlphaOverride = true;
  alpha = inalpha;
}
