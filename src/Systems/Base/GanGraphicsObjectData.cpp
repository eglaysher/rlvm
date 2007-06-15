// This file is part of RLVM, a RealLive virtual machine clone.
//
// -----------------------------------------------------------------------
//
// Copyright (C) 2007 Elliot Glaysher
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

#include "Systems/Base/System.hpp"
#include "Systems/Base/EventSystem.hpp"
#include "Systems/Base/GanGraphicsObjectData.hpp"
#include "Systems/Base/Surface.hpp"
#include "Systems/Base/GraphicsSystem.hpp"

#include "Utilities.h"
#include "libReallive/defs.h"

#include "MachineBase/RLMachine.hpp"

#include <fstream>

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

// -----------------------------------------------------------------------
// GanGraphicsObjectData
// -----------------------------------------------------------------------
GanGraphicsObjectData::GanGraphicsObjectData(
  RLMachine& machine, const std::string& ganFile, 
  const shared_ptr<Surface>& incomingImage)
  : m_currentlyPlaying(false), m_currentFrame(-1), 
    image(incomingImage)
{
  ifstream ifs(ganFile.c_str(), ifstream::in | ifstream::binary);
  if(!ifs)
  {
    ostringstream oss;
    oss << "Could not open file \"" << ganFile << "\".";
    throw rlvm::Exception(oss.str());
  }

  int fileSize = 0;
  scoped_array<char> ganData;
  if(loadFileData(ifs, ganData, fileSize))
  {
    ostringstream oss;
    oss << "Could not read the contents of \"" << ganFile << "\"";
    throw rlvm::Exception(oss.str());
  }

  testFileMagic(ganFile, ganData, fileSize);
  readData(machine, ganFile, ganData, fileSize);
}

// -----------------------------------------------------------------------

GanGraphicsObjectData::~GanGraphicsObjectData()
{}

// -----------------------------------------------------------------------

void GanGraphicsObjectData::testFileMagic(
  const std::string& fileName,
  scoped_array<char>& ganData, int fileSize)
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
  scoped_array<char>& ganData, int fileSize)
{
  const char* data = ganData.get();
  int fileNameLength = read_i32(data + 0xc);
  string rawFileName = data + 0x10;

  // Strings should be NULL terminated.
  data = data + 0x10 + fileNameLength - 1;
  if(*data != 0)
    throwBadFormat(fileName, "Incorrect filename length in GAN header");
  data++;

  // Now that we have the filename, load that file (restricting things
  // so that it only search
//   string filePath = findFile(machine, rawFileName, IMAGE_FILETYPES);
//   image.reset(machine.system().graphics().loadSurfaceFromFile(filePath));

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
//    cerr << "Number of frames in the first set: " << frameCount << endl;
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

/// @todo This implementation ignores all the esoteric rendering 
//        options that come from renderToScreenAsObject(). Later, we
//        may need to do something about this.
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

      int xPos1 = go.x() + go.xAdjustmentSum() + frame.x;
      int yPos1 = go.y() + go.yAdjustmentSum() + frame.y;
      int xPos2 = int(xPos1 + (rect.x2 - rect.x1) * (go.width() / 100.0f));
      int yPos2 = int(yPos1 + (rect.y2 - rect.y1) * (go.height() / 100.0f));
      image->renderToScreen(rect.x1, rect.y1, rect.x2, rect.y2,
                            xPos1, yPos1, xPos2, yPos2,
                            frame.alpha);
    }
  }
}

// -----------------------------------------------------------------------
  
int GanGraphicsObjectData::pixelWidth(
  const GraphicsObject& renderingProperties)
{
  throw std::runtime_error("Unimplemented: GanGraphicsObjectData::pixelWidth");
}

// -----------------------------------------------------------------------

int GanGraphicsObjectData::pixelHeight(
  const GraphicsObject& renderingProperties)
{
  throw std::runtime_error("Unimplemented: GanGraphicsObjectData::pixelHeight");
}

// -----------------------------------------------------------------------

GraphicsObjectData* GanGraphicsObjectData::clone() const
{
  return new GanGraphicsObjectData(*this);
}

// -----------------------------------------------------------------------

void GanGraphicsObjectData::execute(RLMachine& machine)
{
  if(m_currentlyPlaying)
  {
    unsigned int currentTime = machine.system().event().getTicks();
    unsigned int timeSinceLastFrameChange = 
      currentTime - m_timeAtLastFrameChange;

    const vector<Frame>& currentSet = animationSets.at(m_currentSet);
    int frameTime = currentSet[m_currentFrame].time;
    if(timeSinceLastFrameChange > frameTime)
    {
      m_currentFrame++;
      if(m_currentFrame == currentSet.size())
      {
        m_currentFrame--;
        endAnimation();
      }

      m_timeAtLastFrameChange = currentTime;
    }
  }
}

// -----------------------------------------------------------------------

void GanGraphicsObjectData::endAnimation()
{
  // Set first, because we may deallocate this by one of our actions
  m_currentlyPlaying = false;

  switch(afterAnimation())
  {
  case AFTER_NONE:
    break;
  case AFTER_CLEAR:
    if(ownedBy())
      ownedBy()->deleteObject();
    break;
  }
}

// ------------------------------------ [ AnimatedObjectData interface ]
bool GanGraphicsObjectData::isPlaying() const 
{
  return m_currentlyPlaying;
}

// -----------------------------------------------------------------------

void GanGraphicsObjectData::playSet(RLMachine& machine, int set) 
{
  m_currentlyPlaying = true;
  m_currentSet = set;
  m_currentFrame = 0;
  m_timeAtLastFrameChange = machine.system().event().getTicks();
}
