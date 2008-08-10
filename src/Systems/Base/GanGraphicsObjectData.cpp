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
  : current_set_(-1), current_frame_(-1), time_at_last_frame_change_(0)
{}

// -----------------------------------------------------------------------

GanGraphicsObjectData::GanGraphicsObjectData(
  RLMachine& machine, const std::string& gan_file,
  const std::string& img_file)
  : gan_filename_(gan_file), img_filename_(img_file), current_set_(-1),
    current_frame_(-1), time_at_last_frame_change_(0)
{
  load(machine);
}

// -----------------------------------------------------------------------

GanGraphicsObjectData::~GanGraphicsObjectData()
{}

// -----------------------------------------------------------------------

void GanGraphicsObjectData::load(RLMachine& machine)
{
  image = machine.system().graphics().loadSurfaceFromFile(
    machine, img_filename_);

  fs::path gan_file_path = findFile(machine, gan_filename_, GAN_FILETYPES);
  fs::ifstream ifs(gan_file_path, ifstream::in | ifstream::binary);
  if(!ifs)
  {
    ostringstream oss;
    oss << "Could not open file \"" << gan_file_path << "\".";
    throw rlvm::Exception(oss.str());
  }

  int file_size = 0;
  scoped_array<char> gan_data;
  if(loadFileData(ifs, gan_data, file_size))
  {
    ostringstream oss;
    oss << "Could not read the contents of \"" << gan_file_path << "\"";
    throw rlvm::Exception(oss.str());
  }

  testFileMagic(gan_filename_, gan_data, file_size);
  readData(machine, gan_filename_, gan_data, file_size);
}

// -----------------------------------------------------------------------

void GanGraphicsObjectData::testFileMagic(
  const std::string& file_name,
  boost::scoped_array<char>& gan_data, int file_size)
{
  const char* data = gan_data.get();
  int a = read_i32(data);
  int b = read_i32(data + 0x04);
  int c = read_i32(data + 0x08);

  if(a != 10000 || b != 10000 || c != 10100)
    throwBadFormat(file_name, "Incorrect GAN file magic");
}

// -----------------------------------------------------------------------

void GanGraphicsObjectData::readData(
  RLMachine& machine,
  const std::string& file_name,
  boost::scoped_array<char>& gan_data, int file_size)
{
  const char* data = gan_data.get();
  int file_name_length = read_i32(data + 0xc);
  string raw_file_name = data + 0x10;

  // Strings should be NULL terminated.
  data = data + 0x10 + file_name_length - 1;
  if(*data != 0)
    throwBadFormat(file_name, "Incorrect filename length in GAN header");
  data++;

  int twenty_thousand = read_i32(data);
  if(twenty_thousand != 20000)
    throwBadFormat(file_name, "Expected start of GAN data section");
  data += 4;

  int number_of_sets = read_i32(data);
  data += 4;

  for(int i = 0; i < number_of_sets; ++i)
  {
    int start_of_ganset = read_i32(data);
    if(start_of_ganset != 0x7530)
      throwBadFormat(file_name, "Expected start of GAN set");

    data += 4;
    int frame_count = read_i32(data);
    if(frame_count < 0)
      throwBadFormat(file_name,
                     "Expected animation to contain at least one frame");
    data += 4;

    vector<Frame> animation_set;
    for(int j = 0; j < frame_count; ++j)
      animation_set.push_back(readSetFrame(file_name, data));
    animation_sets.push_back(animation_set);
  }
}

// -----------------------------------------------------------------------

GanGraphicsObjectData::Frame
GanGraphicsObjectData::readSetFrame(const std::string& file_name,
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
      throwBadFormat(file_name, oss.str());
    }
    }

    tag = read_i32(data);
    data += 4;
  }

  return frame;
}

// -----------------------------------------------------------------------

void GanGraphicsObjectData::throwBadFormat(
  const std::string& file_name, const std::string& error)
{
  ostringstream oss;
  oss << "File \"" << file_name
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
  if(current_set_ != -1 && current_frame_ != -1)
  {
    const Frame& frame = animation_sets.at(current_set_).at(current_frame_);

    // First, we figure out the pattern to get the image source
    if(frame.pattern != -1)
    {
      const Surface::GrpRect& rect = image->getPattern(frame.pattern);

      // Groan. Now I can't really test this.
      GraphicsObjectOverride override_data;
      // POINT
      override_data.setOverrideSource(rect.rect.x(), rect.rect.y(),
                                     rect.rect.x2(), rect.rect.y2());
      override_data.setDestOffset(frame.x, frame.y);

      // Calculate the combination of our frame alpha with the current
      // object alpha
      override_data.setAlphaOverride(
        int(((frame.alpha/256.0f) * (go.alpha() / 256.0f)) * 256));

      image->renderToScreenAsObject(go, override_data);
    }
  }
}

// -----------------------------------------------------------------------

int GanGraphicsObjectData::pixelWidth(
  RLMachine& machine,
  const GraphicsObject& rendering_properties)
{
  if(current_set_ != -1 && current_frame_ != -1)
  {
    const Frame& frame = animation_sets.at(current_set_).at(current_frame_);
    if(frame.pattern != -1)
    {
      const Surface::GrpRect& rect = image->getPattern(frame.pattern);
      return rect.rect.width();
    }
  }

  // return the width of the first set, first frame
  return 0;
}

// -----------------------------------------------------------------------

int GanGraphicsObjectData::pixelHeight(
  RLMachine& machine,
  const GraphicsObject& rendering_properties)
{
  if(current_set_ != -1 && current_frame_ != -1)
  {
    const Frame& frame = animation_sets.at(current_set_).at(current_frame_);
    if(frame.pattern != -1)
    {
      const Surface::GrpRect& rect = image->getPattern(frame.pattern);
      return rect.rect.height();
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
  if(currentlyPlaying() && current_frame_ >= 0)
  {
    unsigned int current_time = machine.system().event().getTicks();
    unsigned int time_since_last_frame_change =
      current_time - time_at_last_frame_change_;

    const vector<Frame>& current_set = animation_sets.at(current_set_);
    unsigned int frame_time = (unsigned int)(current_set[current_frame_].time);
    if(time_since_last_frame_change > frame_time)
    {
      current_frame_++;
      if(size_t(current_frame_) == current_set.size())
      {
        current_frame_--;
        endAnimation();
      }

      time_at_last_frame_change_ = current_time;
      machine.system().graphics().markScreenAsDirty(GUT_DISPLAY_OBJ);
    }
  }
}

// -----------------------------------------------------------------------

void GanGraphicsObjectData::loopAnimation()
{
  current_frame_ = 0;
}

// -----------------------------------------------------------------------

void GanGraphicsObjectData::playSet(RLMachine& machine, int set)
{
  setCurrentlyPlaying(true);
  current_set_ = set;
  current_frame_ = 0;
  time_at_last_frame_change_ = machine.system().event().getTicks();
  machine.system().graphics().markScreenAsDirty(GUT_DISPLAY_OBJ);
}

// -----------------------------------------------------------------------

template<class Archive>
void GanGraphicsObjectData::load(Archive& ar, unsigned int version)
{
  ar & boost::serialization::base_object<GraphicsObjectData>(*this)
    & gan_filename_ & img_filename_ & current_set_
    & current_frame_ & time_at_last_frame_change_;

  load(*Serialization::g_current_machine);
}

// -----------------------------------------------------------------------

template<class Archive>
void GanGraphicsObjectData::save(Archive& ar, unsigned int version) const
{
  ar & boost::serialization::base_object<GraphicsObjectData>(*this)
    & gan_filename_ & img_filename_ & current_set_
    & current_frame_ & time_at_last_frame_change_;
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
