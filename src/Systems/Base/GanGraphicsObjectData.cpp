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

/**
 * @file   GanGraphicsObjectData.cpp
 * @author Elliot Glaysher
 * @date   Sun Jun 10 11:21:51 2007
 * 
 * @brief  In memory representation of a GAN animation file.
 * 
 * This code is heavily based off Haeleth's O'caml implementation
 * (which translates binary GAN files to and from an XML
 * representation), found at rldev/src/rlxml/gan.ml.
 */

#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/serialization/scoped_ptr.hpp>

#include "Systems/Base/System.hpp"
#include "Systems/Base/EventSystem.hpp"
#include "Systems/Base/GanGraphicsObjectData.hpp"
#include "Systems/Base/Surface.hpp"
#include "Systems/Base/GraphicsSystem.hpp"
#include "Systems/Base/GraphicsObject.hpp"
#include "MachineBase/Serialization.hpp"

#include "Utilities.h"
#include "libReallive/defs.h"

#include "MachineBase/RLMachine.hpp"

#include <boost/serialization/export.hpp>
#include <boost/filesystem/fstream.hpp>

// -----------------------------------------------------------------------

using libReallive::read_i32;
using boost::scoped_array;
using boost::shared_ptr;
using std::string;
using std::ifstream;
using std::ostringstream;
using std::cerr;
using std::endl;
using std::vector;

namespace fs = boost::filesystem;

// -----------------------------------------------------------------------
// GanGraphicsObjectData
// -----------------------------------------------------------------------

GanGraphicsObjectData::GanGraphicsObjectData()
  : m_currentSet(-1), m_currentFrame(-1), m_timeAtLastFrameChange(0)
{}

// -----------------------------------------------------------------------

GanGraphicsObjectData::GanGraphicsObjectData(
  RLMachine& machine, const std::string& ganFile, 
  const std::string& imgFile)
  : m_ganFilename(ganFile), m_imgFilename(imgFile), m_currentSet(-1),
    m_currentFrame(-1), m_timeAtLastFrameChange(0)
{
  load(machine);
}

// -----------------------------------------------------------------------

GanGraphicsObjectData::~GanGraphicsObjectData()
{}

// -----------------------------------------------------------------------

void GanGraphicsObjectData::load(RLMachine& machine)
{
  fs::path imgFilePath = findFile(machine, m_imgFilename, IMAGE_FILETYPES);
  fs::path ganFilePath = findFile(machine, m_ganFilename, GAN_FILETYPES);

  image = machine.system().graphics().loadSurfaceFromFile(imgFilePath);

  fs::ifstream ifs(ganFilePath, ifstream::in | ifstream::binary);
  if(!ifs)
  {
    ostringstream oss;
    oss << "Could not open file \"" << ganFilePath << "\".";
    throw rlvm::Exception(oss.str());
  }

  int fileSize = 0;
  scoped_array<char> ganData;
  if(loadFileData(ifs, ganData, fileSize))
  {
    ostringstream oss;
    oss << "Could not read the contents of \"" << ganFilePath << "\"";
    throw rlvm::Exception(oss.str());
  }

  testFileMagic(m_ganFilename, ganData, fileSize);
  readData(machine, m_ganFilename, ganData, fileSize);  
}

// -----------------------------------------------------------------------

void GanGraphicsObjectData::testFileMagic(
  const std::string& fileName,
  boost::scoped_array<char>& ganData, int fileSize)
{
  const char* data = ganData.get();
  int a = read_i32(data);
  int b = read_i32(data + 0x04);
  int c = read_i32(data + 0x08);

  if(a != 10000 || b != 10000 || c != 10100)
    throwBadFormat(fileName, "Incorrect GAN file magic");
}

// -----------------------------------------------------------------------

void GanGraphicsObjectData::readData(
  RLMachine& machine,
  const std::string& fileName,
  boost::scoped_array<char>& ganData, int fileSize)
{
  const char* data = ganData.get();
  int fileNameLength = read_i32(data + 0xc);
  string rawFileName = data + 0x10;

  // Strings should be NULL terminated.
  data = data + 0x10 + fileNameLength - 1;
  if(*data != 0)
    throwBadFormat(fileName, "Incorrect filename length in GAN header");
  data++;

  int twentyThousand = read_i32(data);
  if(twentyThousand != 20000)
    throwBadFormat(fileName, "Expected start of GAN data section");
  data += 4;

  int numberOfSets = read_i32(data);
  data += 4;
  
  for(int i = 0; i < numberOfSets; ++i)
  {
    int startOfGANset = read_i32(data);
    if(startOfGANset != 0x7530)
      throwBadFormat(fileName, "Expected start of GAN set");
    
    data += 4;
    int frameCount = read_i32(data);
    if(frameCount < 0)
      throwBadFormat(fileName, 
                     "Expected animation to contain at least one frame");
    data += 4;

    vector<Frame> animationSet;
    for(int j = 0; j < frameCount; ++j)
      animationSet.push_back(readSetFrame(fileName, data));
    animationSets.push_back(animationSet);
  }
}

// -----------------------------------------------------------------------

GanGraphicsObjectData::Frame 
GanGraphicsObjectData::readSetFrame(const std::string& fileName, 
                                    const char*& data)
{
  GanGraphicsObjectData::Frame frame;

  int tag = read_i32(data);
  data += 4;
  while(tag != 999999)
  {
    int value = read_i32(data);
    data += 4;

    switch(tag)
    {
    case 30100:
      frame.pattern = value;
      break;
    case 30101:
      frame.x = value;
      break;
    case 30102:
      frame.y = value;
      break;
    case 30103:
      frame.time = value;
      break;
    case 30104:
      frame.alpha = value;
      break;
    case 30105:
      frame.other = value;
      break;
    default:
    {
      ostringstream oss;
      oss << "Unknown GAN frame tag: " << tag;
      throwBadFormat(fileName, oss.str());
    }
    }

    tag = read_i32(data);
    data += 4;
  }

  return frame;
}

// -----------------------------------------------------------------------

void GanGraphicsObjectData::throwBadFormat(
  const std::string& fileName, const std::string& error)
{
  ostringstream oss;
  oss << "File \"" << fileName
      << "\" does not appear to be in GAN format: "
      << error;
  throw rlvm::Exception(oss.str());
}

// -----------------------------------------------------------------------
// -----------------------------------------------------------------------
// -----------------------------------------------------------------------

void GanGraphicsObjectData::render(
  RLMachine& machine, 
  const GraphicsObject& go)
{
  if(m_currentSet != -1 && m_currentFrame != -1)
  {
    const Frame& frame = animationSets.at(m_currentSet).at(m_currentFrame);

    // First, we figure out the pattern to get the image source
    if(frame.pattern != -1)
    {
      const Surface::GrpRect& rect = image->getPattern(frame.pattern);

      // Groan. Now I can't really test this.
      GraphicsObjectOverride overrideData;
      overrideData.setOverrideSource(rect.x1, rect.y1, rect.x2, rect.y2);
      overrideData.setDestOffset(frame.x, frame.y);

      // Calculate the combination of our frame alpha with the current
      // object alpha
      overrideData.setAlphaOverride(
        int(((frame.alpha/256.0f) * (go.alpha() / 256.0f)) * 256));

      image->renderToScreenAsObject(go, overrideData);
    }
  }
}

// -----------------------------------------------------------------------
  
int GanGraphicsObjectData::pixelWidth(
  RLMachine& machine,
  const GraphicsObject& renderingProperties)
{
  if(m_currentSet != -1 && m_currentFrame != -1)
  {
    const Frame& frame = animationSets.at(m_currentSet).at(m_currentFrame);
    if(frame.pattern != -1)
    {
      const Surface::GrpRect& rect = image->getPattern(frame.pattern);

      return rect.x2 - rect.x1;
    }
  }

  // return the width of the first set, first frame
  return 0;
}

// -----------------------------------------------------------------------

int GanGraphicsObjectData::pixelHeight(
  RLMachine& machine, 
  const GraphicsObject& renderingProperties)
{
  if(m_currentSet != -1 && m_currentFrame != -1)
  {
    const Frame& frame = animationSets.at(m_currentSet).at(m_currentFrame);
    if(frame.pattern != -1)
    {
      const Surface::GrpRect& rect = image->getPattern(frame.pattern);

      return rect.y2 - rect.y1;
    }
  }

  // return the width of the first set, first frame
  return 0;
}

// -----------------------------------------------------------------------

GraphicsObjectData* GanGraphicsObjectData::clone() const
{
  return new GanGraphicsObjectData(*this);
}

// -----------------------------------------------------------------------

void GanGraphicsObjectData::execute(RLMachine& machine)
{
  if(currentlyPlaying() && m_currentFrame >= 0)
  {
    unsigned int currentTime = machine.system().event().getTicks();
    unsigned int timeSinceLastFrameChange = 
      currentTime - m_timeAtLastFrameChange;

    const vector<Frame>& currentSet = animationSets.at(m_currentSet);
    unsigned int frameTime = (unsigned int)(currentSet[m_currentFrame].time);
    if(timeSinceLastFrameChange > frameTime)
    {
      m_currentFrame++;
      if(size_t(m_currentFrame) == currentSet.size())
      {
        m_currentFrame--;
        endAnimation();
      }

      m_timeAtLastFrameChange = currentTime;
      machine.system().graphics().markScreenAsDirty(GUT_DISPLAY_OBJ);
    }
  }
}

// -----------------------------------------------------------------------

void GanGraphicsObjectData::loopAnimation()
{
  m_currentFrame = 0;
}

// -----------------------------------------------------------------------

void GanGraphicsObjectData::playSet(RLMachine& machine, int set) 
{
  setCurrentlyPlaying(true);
  m_currentSet = set;
  m_currentFrame = 0;
  m_timeAtLastFrameChange = machine.system().event().getTicks();
  machine.system().graphics().markScreenAsDirty(GUT_DISPLAY_OBJ);
}

// -----------------------------------------------------------------------

template<class Archive>
void GanGraphicsObjectData::load(Archive& ar, unsigned int version)
{
  ar & boost::serialization::base_object<GraphicsObjectData>(*this)
    & m_ganFilename & m_imgFilename & m_currentSet
    & m_currentFrame & m_timeAtLastFrameChange;

  load(*Serialization::g_currentMachine);
}

// -----------------------------------------------------------------------

template<class Archive>
void GanGraphicsObjectData::save(Archive& ar, unsigned int version) const
{
  ar & boost::serialization::base_object<GraphicsObjectData>(*this)
    & m_ganFilename & m_imgFilename & m_currentSet
    & m_currentFrame & m_timeAtLastFrameChange;
}

// -----------------------------------------------------------------------

// Explicit instantiations for text archives (since we hide the
// implementation)

template void GanGraphicsObjectData::save<boost::archive::text_oarchive>(
  boost::archive::text_oarchive & ar, unsigned int version) const;

template void GanGraphicsObjectData::load<boost::archive::text_iarchive>(
  boost::archive::text_iarchive & ar, unsigned int version);

// -----------------------------------------------------------------------

BOOST_CLASS_EXPORT(GanGraphicsObjectData);
