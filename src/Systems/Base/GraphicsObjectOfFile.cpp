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

#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/serialization/scoped_ptr.hpp>
#include <boost/serialization/export.hpp>

#include "Systems/Base/GraphicsSystem.hpp"
#include "Systems/Base/GraphicsObject.hpp"
#include "Systems/Base/System.hpp"
#include "Systems/Base/Surface.hpp"
#include "Systems/Base/GraphicsObjectOfFile.hpp"
#include "Systems/Base/EventSystem.hpp"
#include "MachineBase/RLMachine.hpp"
#include "MachineBase/Serialization.hpp"
#include <boost/shared_ptr.hpp>
#include <string>
#include <iostream>
#include "Utilities.h"

using namespace std;
namespace fs = boost::filesystem;

// -----------------------------------------------------------------------

GraphicsObjectOfFile::GraphicsObjectOfFile()
  : m_filename(""),
    m_frameTime(0),
    m_currentFrame(0),
    m_timeAtLastFrameChange(0)
{
}

// -----------------------------------------------------------------------

GraphicsObjectOfFile::GraphicsObjectOfFile(
  const GraphicsObjectOfFile& obj)
  : GraphicsObjectData(obj),
    m_filename(obj.m_filename),
    m_surface(obj.m_surface), 
    m_frameTime(obj.m_frameTime),
    m_currentFrame(obj.m_currentFrame),
    m_timeAtLastFrameChange(obj.m_timeAtLastFrameChange)
{}

// -----------------------------------------------------------------------

GraphicsObjectOfFile::GraphicsObjectOfFile(
  RLMachine& machine, const std::string& filename)
  : m_filename(filename),
    m_frameTime(0),
    m_currentFrame(0),
    m_timeAtLastFrameChange(0)
{
  loadFile(machine);
}

// -----------------------------------------------------------------------

void GraphicsObjectOfFile::loadFile(RLMachine& machine)
{
  fs::path fullPath = findFile(machine, m_filename);
  m_surface = machine.system().graphics().loadSurfaceFromFile(fullPath);
}

// -----------------------------------------------------------------------

void GraphicsObjectOfFile::render(RLMachine& machine, const GraphicsObject& rp)
{
  if(currentlyPlaying())
  {
    const Surface::GrpRect& rect = m_surface->getPattern(m_currentFrame);

    GraphicsObjectOverride overrideData;
    overrideData.setOverrideSource(rect.x1, rect.y1, rect.x2, rect.y2);

    m_surface->renderToScreenAsObject(rp, overrideData);
  }
  else
    m_surface->renderToScreenAsObject(rp);
}

// -----------------------------------------------------------------------

int GraphicsObjectOfFile::pixelWidth(RLMachine& machine, const GraphicsObject& rp)
{
  const Surface::GrpRect& rect = m_surface->getPattern(rp.pattNo());
  int width = rect.x2 - rect.x1;
  return int((rp.width() / 100.0f) * width);
}

// -----------------------------------------------------------------------

int GraphicsObjectOfFile::pixelHeight(RLMachine& machine, const GraphicsObject& rp)
{
  const Surface::GrpRect& rect = m_surface->getPattern(rp.pattNo());
  int height = rect.y2 - rect.y1;
  return int((rp.height() / 100.0f) * height);
}

// -----------------------------------------------------------------------

GraphicsObjectData* GraphicsObjectOfFile::clone() const 
{
  return new GraphicsObjectOfFile(*this);
}

// -----------------------------------------------------------------------

void GraphicsObjectOfFile::execute(RLMachine& machine)
{
  if(currentlyPlaying())
  {
    unsigned int currentTime = machine.system().event().getTicks();
    unsigned int timeSinceLastFrameChange = 
      currentTime - m_timeAtLastFrameChange;

    while(timeSinceLastFrameChange > m_frameTime)
    {
      m_currentFrame++;
      if(m_currentFrame == m_surface->numPatterns())
      {
        m_currentFrame--;
        endAnimation();
      }

      m_timeAtLastFrameChange += m_frameTime;
      timeSinceLastFrameChange = currentTime - m_timeAtLastFrameChange;
      machine.system().graphics().markScreenAsDirty(GUT_DISPLAY_OBJ);
    }
  }
}

// -----------------------------------------------------------------------

bool GraphicsObjectOfFile::isAnimation() const
{
  return m_surface->numPatterns();
}

// -----------------------------------------------------------------------

void GraphicsObjectOfFile::loopAnimation()
{
  m_currentFrame = 0;
}

// -----------------------------------------------------------------------

void GraphicsObjectOfFile::playSet(RLMachine& machine, int frameTime)
{
  setCurrentlyPlaying(true);
  m_frameTime = frameTime;
  m_currentFrame = 0;

  m_timeAtLastFrameChange = machine.system().event().getTicks();
  machine.system().graphics().markScreenAsDirty(GUT_DISPLAY_OBJ);
}

// -----------------------------------------------------------------------

template<class Archive>
void GraphicsObjectOfFile::load(Archive& ar, unsigned int version)
{
  ar & boost::serialization::base_object<GraphicsObjectData>(*this)
    & m_filename & m_frameTime & m_currentFrame & m_timeAtLastFrameChange;

  loadFile(*Serialization::g_currentMachine);
}

// -----------------------------------------------------------------------

template<class Archive>
void GraphicsObjectOfFile::save(Archive& ar, unsigned int version) const
{
  ar & boost::serialization::base_object<GraphicsObjectData>(*this)
    & m_filename & m_frameTime & m_currentFrame & m_timeAtLastFrameChange;
}

// -----------------------------------------------------------------------

BOOST_CLASS_EXPORT(GraphicsObjectOfFile);

// -----------------------------------------------------------------------

// Explicit instantiations for text archives (since we hide the
// implementation)

template void GraphicsObjectOfFile::save<boost::archive::text_oarchive>(
  boost::archive::text_oarchive & ar, unsigned int version) const;

template void GraphicsObjectOfFile::load<boost::archive::text_iarchive>(
  boost::archive::text_iarchive & ar, unsigned int version);
