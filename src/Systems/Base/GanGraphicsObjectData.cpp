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

#include "Systems/Base/GanGraphicsObjectData.hpp"

#include "Utilities.h"
#include "libReallive/defs.h"

#include <fstream>

// -----------------------------------------------------------------------
using libReallive::read_i32;
using boost::scoped_array;
using std::string;
using std::ifstream;
using std::ostringstream;
using std::cerr;
using std::endl;


// -----------------------------------------------------------------------
// GanGraphicsObjectData
// -----------------------------------------------------------------------
GanGraphicsObjectData::GanGraphicsObjectData(
  RLMachine& machine, const std::string& file)
  : m_currentlyPlaying(false)
{
  ifstream ifs(file.c_str(), ifstream::in | ifstream::binary);
  if(!ifs)
  {
    ostringstream oss;
    oss << "Could not open file \"" << file << "\".";
    throw rlvm::Exception(oss.str());
  }

  int fileSize = 0;
  scoped_array<char> ganData;
  if(loadFileData(ifs, ganData, fileSize))
  {
    ostringstream oss;
    oss << "Could not read the contents of \"" << file << "\"";
    throw rlvm::Exception(oss.str());
  }

  testFileMagic(file, ganData, fileSize);
  readData(file, ganData, fileSize);
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
  const std::string& fileName,
  scoped_array<char>& ganData, int fileSize)
{
  const char* data = ganData.get();
  int fileNameLength = read_i32(data + 0xc);
  string name = data + 0x10;

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
  
//   for(int i = 0; i < numberOfSets; ++i)
//   {
//     data += 4;
//     int startOfGANset = read_i32(data);
//     if(startOfGANset != 0x7530)
//       throwBadFormat(fileName, "Expected start of GAN set");
    
//     data += 4;
//     int frameCount = read_i32(data);
//     if(frameCount < 0)
//       throwBadFormat(fileName, 
//                      "Expected animation to contain at least one frame");

// //    read_set_frames(data,   );
//   }

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
  const GraphicsObject& renderingProperties)
{
}

// -----------------------------------------------------------------------
  
int GanGraphicsObjectData::pixelWidth(
  const GraphicsObject& renderingProperties)
{
}

// -----------------------------------------------------------------------

int GanGraphicsObjectData::pixelHeight(
  const GraphicsObject& renderingProperties)
{
}

// -----------------------------------------------------------------------

GraphicsObjectData* GanGraphicsObjectData::clone() const
{

}

// -----------------------------------------------------------------------

void GanGraphicsObjectData::execute(RLMachine& machine)
{
}

// ------------------------------------ [ AnimatedObjectData interface ]
bool GanGraphicsObjectData::isPlaying() const {}
void GanGraphicsObjectData::playSet(RLMachine& machine, int set) {}
