// This file is part of RLVM, a RealLive virtual machine clone.
//
// -----------------------------------------------------------------------
//
// Copyright (C) 2007 Elliot Glaysher
// Copyright (C) 2000 Kazunori Ueno(JAGARL) <jagarl@creator.club.ne.jp>
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
 * @file   AnmGraphicsObjectData.cpp
 * @date   Sun Jun  3 13:04:19 2007
 * 
 * @brief  In memory representation of an ANM file.
 * 
 * The code in this file has been modified from the file anm.cc in
 * Jagarl's xkanon project.
 *
 * @todo Reading the data into memory and then acessing it by byte
 *       offset isn't secure; there needs to be some sort of check
 *       against the length of the array if we're going to do that.
 */

#include "Utilities.h"
#include "MachineBase/RLMachine.hpp"
#include "Systems/Base/AnmGraphicsObjectData.hpp"
#include "Systems/Base/Surface.hpp"
#include "Systems/Base/System.hpp"
#include "Systems/Base/GraphicsSystem.hpp"
#include "Systems/Base/EventSystem.hpp"
#include "libReallive/defs.h"
#include "Systems/Base/GraphicsObject.hpp"

#include <iterator>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include <iostream>

using boost::scoped_array;
using boost::shared_ptr;

using libReallive::read_i32;

using std::cerr;
using std::endl;
using std::ostream_iterator;
using std::ios;
using std::ifstream;
using std::ostringstream;
using std::string;
using std::vector;

// -----------------------------------------------------------------------

// Constants
static const int ANM_MAGIC_SIZE = 12;
static const char ANM_MAGIC[ANM_MAGIC_SIZE] = 
{'A','N','M','3','2',0,0,0,0,1,0,0};

// -----------------------------------------------------------------------

AnmGraphicsObjectData::AnmGraphicsObjectData(
  RLMachine& machine, const std::string& file)
{
  ifstream ifs(file.c_str(), ifstream::in | ifstream::binary);
  if(!ifs)
  {
    ostringstream oss;
    oss << "Could not open file \"" << file << "\".";
    throw rlvm::Exception(oss.str());
  }

  int fileSize = 0;
  scoped_array<char> anmData;
  if(loadFileData(ifs, anmData, fileSize))
  {
    ostringstream oss;
    oss << "Could not read the contents of \"" << file << "\"";
    throw rlvm::Exception(oss.str());
  }

  if(testFileMagic(anmData))
  {
    ostringstream oss;
    oss << "File \"" << file << "\" does not appear to be in ANM format.";
    throw rlvm::Exception(oss.str());
  }

  loadAnmFileFromData(machine, anmData);
}

// -----------------------------------------------------------------------

AnmGraphicsObjectData::~AnmGraphicsObjectData()
{}

// -----------------------------------------------------------------------

bool AnmGraphicsObjectData::testFileMagic(scoped_array<char>& anmData)
{
  return memcmp(anmData.get(), ANM_MAGIC, ANM_MAGIC_SIZE) != 0;
}

// -----------------------------------------------------------------------

void AnmGraphicsObjectData::loadAnmFileFromData(
  RLMachine& machine, scoped_array<char>& anmData)
{
  const char* data = anmData.get();

  // Read the header
  int frames_len = read_i32(data + 0x8c);
  int framelist_len = read_i32(data + 0x90);
  int animationSet_len = read_i32(data + 0x94);
  if(animationSet_len < 0)
    throw rlvm::Exception("Impossible value for animationSet_len in ANM file.");

//   cerr << "animationSet_len: " << animationSet_len << endl
//        << "framelist_len: " << framelist_len << endl
//        << "frames_len: " << frames_len << endl;

  // Read the corresponding image file we read from, and load the image.
  string rawFileName = data + 0x1c;
  string fileName = findFile(machine, rawFileName, IMAGE_FILETYPES);
  image = machine.system().graphics().loadSurfaceFromFile(fileName);
  
  // Read the frame list
  const char* buf = data + 0xb8;
  int width, height;
  getScreenSize(machine.system().gameexe(), width, height);
  for(int i = 0; i < frames_len; ++i)
  {
    Frame f;
    f.src_x1 = read_i32(buf);
    f.src_y1 = read_i32(buf+4);
    f.src_x2 = read_i32(buf+8);
    f.src_y2 = read_i32(buf+12);
    f.dest_x = read_i32(buf+16);
    f.dest_y = read_i32(buf+20);
    f.time = read_i32(buf+0x38);
    fixAxis(f, width, height);
    frames.push_back(f);

    buf += 0x60;
  }

  readIntegerList(data + 0xb8 + frames_len*0x60, 0x68, framelist_len, framelist);
  readIntegerList(data + 0xb8 + frames_len*0x60 + framelist_len*0x68,
                  0x78, animationSet_len, animationSet);
}

// -----------------------------------------------------------------------

void AnmGraphicsObjectData::readIntegerList(
  const char* start, int offset, int iterations,
  std::vector< std::vector<int> >& dest)
{
  for(int i = 0; i < iterations; ++i)
  {
    int listLength = read_i32(start + 4);
    const char* tmpbuf = start + 8;
    vector<int> intlist;
    for(int j = 0; j < listLength; ++j)
    {
      intlist.push_back(read_i32(tmpbuf));
      tmpbuf += 4;
    }
    dest.push_back(intlist);

    start += offset;
  }
}

// -----------------------------------------------------------------------

/// @todo make me work.
void AnmGraphicsObjectData::fixAxis(Frame& frame, int width, int height)
{
  if (frame.src_x1 > frame.src_x2) { // swap
    int tmp = frame.src_x1; frame.src_x1 = frame.src_x2; frame.src_x2 = tmp;
  }
  if (frame.src_y1 > frame.src_y2) { // swap
    int tmp = frame.src_y1; frame.src_y1 = frame.src_y2; frame.src_y2 = tmp;
  }
  // check screen size
  // int tmp_x = frame.dest_x, tmp_y = frame.dest_y;
  if (frame.dest_x + (frame.src_x2-frame.src_x1+1) > width) {
    frame.src_x2 = frame.src_x1 + width - frame.dest_x;
  }
  if (frame.dest_y + (frame.src_y2-frame.src_y1+1) > width) {
    frame.src_y2 = frame.src_y1 + width - frame.dest_y;
  }
}

// -----------------------------------------------------------------------

void AnmGraphicsObjectData::execute(RLMachine& machine)
{
  if(currentlyPlaying())
  {
    advanceFrame(machine);
  }
}

// -----------------------------------------------------------------------

void AnmGraphicsObjectData::render(
  RLMachine& machine, 
  const GraphicsObject& renderingProperties)
{
  // If we have a current frame, then let's render it.
  if(m_currentFrame != -1)
  {
    const Frame& frame = frames.at(m_currentFrame);

    GraphicsObjectOverride overrideData;
    overrideData.setOverrideSource(frame.src_x1, frame.src_y1,
                                   frame.src_x2, frame.src_y2);
    overrideData.setOverrideDestination(
      frame.dest_x, frame.dest_y,
      frame.dest_x + (frame.src_x2 - frame.src_x1),
      frame.dest_y + (frame.src_y2 - frame.src_y1));

    image->renderToScreenAsObject(renderingProperties, overrideData);
  }
}

// -----------------------------------------------------------------------

void AnmGraphicsObjectData::advanceFrame(RLMachine& machine)
{
  // Do things that advance the state
  int timeSinceLastFrameChange = 
    machine.system().event().getTicks() - m_timeAtLastFrameChange;
  bool done = false;

  while(currentlyPlaying() && !done)
  {
    if(timeSinceLastFrameChange > frames[m_currentFrame].time)
    {
      timeSinceLastFrameChange -= frames[m_currentFrame].time;
      m_timeAtLastFrameChange += frames[m_currentFrame].time;
      machine.system().graphics().markScreenForRefresh();

      m_curFrame++;
      if(m_curFrame == m_curFrameEnd)
      {
        m_curFrameSet++;
        if(m_curFrameSet == m_curFrameSetEnd)
          setCurrentlyPlaying(false);
        else
        {
          m_curFrame = framelist.at(*m_curFrameSet).begin();
          m_curFrameEnd = framelist.at(*m_curFrameSet).end();
          m_currentFrame = *m_curFrame;
        }
      }
      else
        m_currentFrame = *m_curFrame;
    }
    else
      done = true;
  }  
}

// -----------------------------------------------------------------------

/// I am not entirely sure these methods even make sense given the
/// context...
int AnmGraphicsObjectData::pixelWidth(RLMachine& machine,
									  const GraphicsObject& rp) 
{
  const Surface::GrpRect& rect = image->getPattern(rp.pattNo());
  int width = rect.x2 - rect.x1;
  return int((rp.width() / 100.0f) * width);
}

// -----------------------------------------------------------------------

int AnmGraphicsObjectData::pixelHeight(RLMachine& machine,
									   const GraphicsObject& rp)
{
  const Surface::GrpRect& rect = image->getPattern(rp.pattNo());
  int height = rect.y2 - rect.y1;
  return int((rp.height() / 100.0f) * height);
}

// -----------------------------------------------------------------------

GraphicsObjectData* AnmGraphicsObjectData::clone() const
{
  return new AnmGraphicsObjectData(*this);
}

// -----------------------------------------------------------------------

void AnmGraphicsObjectData::playSet(RLMachine& machine, int set)
{
  setCurrentlyPlaying(true);
  m_timeAtLastFrameChange = machine.system().event().getTicks();

  m_curFrameSet = animationSet.at(set).begin();
  m_curFrameSetEnd = animationSet.at(set).end();
  m_curFrame = framelist.at(*m_curFrameSet).begin();
  m_curFrameEnd = framelist.at(*m_curFrameSet).end();
  m_currentFrame = *m_curFrame;

  machine.system().graphics().markScreenForRefresh();
}
