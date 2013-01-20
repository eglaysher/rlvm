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
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
//
// -----------------------------------------------------------------------

// This code is heavily based off Haeleth's O'caml implementation
// (which translates binary GAN files to and from an XML
// representation), found at rldev/src/rlxml/gan.ml.

#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/serialization/scoped_ptr.hpp>

#include "Systems/Base/GanGraphicsObjectData.hpp"

#include <boost/serialization/export.hpp>
#include <boost/filesystem/fstream.hpp>
#include <string>
#include <vector>

#include "MachineBase/Serialization.hpp"
#include "Systems/Base/EventSystem.hpp"
#include "Systems/Base/GraphicsObject.hpp"
#include "Systems/Base/GraphicsSystem.hpp"
#include "Systems/Base/Surface.hpp"
#include "Systems/Base/System.hpp"
#include "Utilities/Exception.hpp"
#include "Utilities/File.hpp"
#include "libReallive/defs.h"

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

GanGraphicsObjectData::GanGraphicsObjectData(System& system)
    : system_(system),
      current_set_(-1),
      current_frame_(-1),
      time_at_last_frame_change_(0) {
}

GanGraphicsObjectData::GanGraphicsObjectData(
    System& system, const std::string& gan_file,
    const std::string& img_file)
    : system_(system),
      gan_filename_(gan_file),
      img_filename_(img_file),
      current_set_(-1),
      current_frame_(-1),
      time_at_last_frame_change_(0) {
  load();
}

GanGraphicsObjectData::~GanGraphicsObjectData() {}

void GanGraphicsObjectData::load() {
  image_ = system_.graphics().loadNonCGSurfaceFromFile(img_filename_);
  image_->EnsureUploaded();

  fs::path gan_file_path = system_.findFile(gan_filename_, GAN_FILETYPES);
  if (gan_file_path.empty()) {
    ostringstream oss;
    oss << "Could not find GAN file \"" << gan_filename_ << "\".";
    throw rlvm::Exception(oss.str());
  }

  int file_size = 0;
  scoped_array<char> gan_data;
  if (loadFileData(gan_file_path, gan_data, file_size)) {
    ostringstream oss;
    oss << "Could not read the contents of \"" << gan_file_path << "\"";
    throw rlvm::Exception(oss.str());
  }

  testFileMagic(gan_filename_, gan_data, file_size);
  readData(gan_filename_, gan_data, file_size);
}

void GanGraphicsObjectData::testFileMagic(
  const std::string& file_name,
  boost::scoped_array<char>& gan_data, int file_size) {
  const char* data = gan_data.get();
  int a = read_i32(data);
  int b = read_i32(data + 0x04);
  int c = read_i32(data + 0x08);

  if (a != 10000 || b != 10000 || c != 10100)
    throwBadFormat(file_name, "Incorrect GAN file magic");
}

void GanGraphicsObjectData::readData(
  const std::string& file_name,
  boost::scoped_array<char>& gan_data, int file_size) {
  const char* data = gan_data.get();
  int file_name_length = read_i32(data + 0xc);
  string raw_file_name = data + 0x10;

  // Strings should be NULL terminated.
  data = data + 0x10 + file_name_length - 1;
  if (*data != 0)
    throwBadFormat(file_name, "Incorrect filename length in GAN header");
  data++;

  int twenty_thousand = read_i32(data);
  if (twenty_thousand != 20000)
    throwBadFormat(file_name, "Expected start of GAN data section");
  data += 4;

  int number_of_sets = read_i32(data);
  data += 4;

  for (int i = 0; i < number_of_sets; ++i) {
    int start_of_ganset = read_i32(data);
    if (start_of_ganset != 0x7530)
      throwBadFormat(file_name, "Expected start of GAN set");

    data += 4;
    int frame_count = read_i32(data);
    if (frame_count < 0)
      throwBadFormat(file_name,
                     "Expected animation to contain at least one frame");
    data += 4;

    vector<Frame> animation_set;
    for (int j = 0; j < frame_count; ++j)
      animation_set.push_back(readSetFrame(file_name, data));
    animation_sets.push_back(animation_set);
  }
}

GanGraphicsObjectData::Frame
GanGraphicsObjectData::readSetFrame(const std::string& file_name,
                                    const char*& data) {
  GanGraphicsObjectData::Frame frame;

  int tag = read_i32(data);
  data += 4;
  while (tag != 999999) {
    int value = read_i32(data);
    data += 4;

    switch (tag) {
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
    default: {
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

void GanGraphicsObjectData::throwBadFormat(
    const std::string& file_name, const std::string& error) {
  ostringstream oss;
  oss << "File \"" << file_name
      << "\" does not appear to be in GAN format: "
      << error;
  throw rlvm::Exception(oss.str());
}

int GanGraphicsObjectData::pixelWidth(
  const GraphicsObject& rendering_properties) {
  if (current_set_ != -1 && current_frame_ != -1) {
    const Frame& frame = animation_sets.at(current_set_).at(current_frame_);
    if (frame.pattern != -1) {
      const Surface::GrpRect& rect = image_->getPattern(frame.pattern);
      return int(rendering_properties.getWidthScaleFactor() *
                 rect.rect.width());
    }
  }

  return 0;
}

int GanGraphicsObjectData::pixelHeight(
  const GraphicsObject& rendering_properties) {
  if (current_set_ != -1 && current_frame_ != -1) {
    const Frame& frame = animation_sets.at(current_set_).at(current_frame_);
    if (frame.pattern != -1) {
      const Surface::GrpRect& rect = image_->getPattern(frame.pattern);
      return int(rendering_properties.getHeightScaleFactor() *
                 rect.rect.height());
    }
  }

  return 0;
}

GraphicsObjectData* GanGraphicsObjectData::clone() const {
  return new GanGraphicsObjectData(*this);
}

void GanGraphicsObjectData::execute(RLMachine& machine) {
  if (currentlyPlaying() && current_frame_ >= 0) {
    unsigned int current_time = system_.event().getTicks();
    unsigned int time_since_last_frame_change =
      current_time - time_at_last_frame_change_;

    const vector<Frame>& current_set = animation_sets.at(current_set_);
    unsigned int frame_time = (unsigned int)(current_set[current_frame_].time);
    if (time_since_last_frame_change > frame_time) {
      current_frame_++;
      if (size_t(current_frame_) == current_set.size()) {
        current_frame_--;
        // endAnimation() can delete this, so it needs to be the last thing
        // done in this code path...
        endAnimation();
      } else {
        time_at_last_frame_change_ = current_time;
        system_.graphics().markScreenAsDirty(GUT_DISPLAY_OBJ);
      }
    }
  }
}

void GanGraphicsObjectData::loopAnimation() {
  current_frame_ = 0;
}

boost::shared_ptr<const Surface> GanGraphicsObjectData::currentSurface(
  const GraphicsObject& go) {
  if (current_set_ != -1 && current_frame_ != -1) {
    const Frame& frame = animation_sets.at(current_set_).at(current_frame_);

    if (frame.pattern != -1) {
      // We are currently rendering an animation AND the current frame says to
      // render something to the screen.
      return image_;
    }
  }

  return boost::shared_ptr<const Surface>();
}

Rect GanGraphicsObjectData::srcRect(const GraphicsObject& go) {
  const Frame& frame = animation_sets.at(current_set_).at(current_frame_);
  if (frame.pattern != -1) {
    return image_->getPattern(frame.pattern).rect;
  }

  return Rect();
}

Point GanGraphicsObjectData::dstOrigin(const GraphicsObject& go) {
  const Frame& frame = animation_sets.at(current_set_).at(current_frame_);
  return GraphicsObjectData::dstOrigin(go) - Size(frame.x, frame.y);
}

int GanGraphicsObjectData::getRenderingAlpha(const GraphicsObject& go) {
  const Frame& frame = animation_sets.at(current_set_).at(current_frame_);
  if (frame.pattern != -1) {
    // Calculate the combination of our frame alpha with the current object
    // alpha.
    return int(((frame.alpha/256.0f) * (go.computedAlpha() / 256.0f)) * 256);
  } else {
    // Should never happen.
    return go.computedAlpha();
  }
}

void GanGraphicsObjectData::objectInfo(std::ostream& tree) {
  tree << "  GAN file: " << gan_filename_ << " (Using image: "
       << img_filename_ << ")" << endl;
}

void GanGraphicsObjectData::playSet(int set) {
  setCurrentlyPlaying(true);
  current_set_ = set;
  current_frame_ = 0;
  time_at_last_frame_change_ = system_.event().getTicks();
  system_.graphics().markScreenAsDirty(GUT_DISPLAY_OBJ);
}

template<class Archive>
void GanGraphicsObjectData::load(Archive& ar, unsigned int version) {
  ar & boost::serialization::base_object<GraphicsObjectData>(*this)
    & gan_filename_ & img_filename_ & current_set_
    & current_frame_ & time_at_last_frame_change_;

  load();
}

template<class Archive>
void GanGraphicsObjectData::save(Archive& ar, unsigned int version) const {
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
