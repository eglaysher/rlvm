// -*- Mode: C++; tab-width:2; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi:tw=80:et:ts=2:sts=2
//
// -----------------------------------------------------------------------
//
// This file is part of RLVM, a RealLive virtual machine clone.
//
// -----------------------------------------------------------------------
//
// Copyright (C) 2006 Elliot Glaysher
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

#include "Systems/Base/GraphicsObject.hpp"
#include "Systems/Base/GraphicsObjectData.hpp"
#include <iostream>

using namespace std;

// -----------------------------------------------------------------------
// GraphicsObjectData
// -----------------------------------------------------------------------

GraphicsObjectData::GraphicsObjectData()
  : after_animation_(AFTER_NONE), owned_by_(NULL), currently_playing_(false)
{}

// -----------------------------------------------------------------------

GraphicsObjectData::GraphicsObjectData(const GraphicsObjectData& obj)
  : after_animation_(obj.after_animation_), owned_by_(NULL),
    currently_playing_(obj.currently_playing_)
{}

// -----------------------------------------------------------------------

GraphicsObjectData::~GraphicsObjectData() { }

// -----------------------------------------------------------------------

void GraphicsObjectData::loopAnimation() { }

// -----------------------------------------------------------------------

void GraphicsObjectData::endAnimation()
{
  // Set first, because we may deallocate this by one of our actions
  currently_playing_ = false;

  switch(afterAnimation())
  {
  case AFTER_NONE:
    break;
  case AFTER_CLEAR:
    if(ownedBy())
      ownedBy()->deleteObject();
    break;
  case AFTER_LOOP:
  {
    // Reset from the beginning
    currently_playing_ = true;
    loopAnimation();
    break;
  }
  }
}

// -----------------------------------------------------------------------

void GraphicsObjectData::execute(RLMachine& machine) { }

// -----------------------------------------------------------------------

bool GraphicsObjectData::isAnimation() const
{
  return false;
}

// -----------------------------------------------------------------------

void GraphicsObjectData::playSet(RLMachine& machine, int set) { }

// -----------------------------------------------------------------------
// GraphicsObjectOverride
// -----------------------------------------------------------------------

GraphicsObjectOverride::GraphicsObjectOverride()
  : overrideSource(false), overrideDest(false),
    hasDestOffset(false), hasAlphaOverride(false),
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

void GraphicsObjectOverride::setOverrideDestination(
  int indstX1, int indstY1, int indstX2, int indstY2)
{
  overrideDest = true;
  dstX1 = indstX1; dstY1 = indstY1; dstX2 = indstX2; dstY2 = indstY2;
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
