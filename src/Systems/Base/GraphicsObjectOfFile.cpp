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

#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/serialization/scoped_ptr.hpp>
#include <boost/serialization/export.hpp>

#include "Systems/Base/GraphicsObjectOfFile.hpp"

#include <boost/shared_ptr.hpp>
#include <string>
#include <iostream>

#include "MachineBase/Serialization.hpp"
#include "Systems/Base/EventSystem.hpp"
#include "Systems/Base/GraphicsObject.hpp"
#include "Systems/Base/GraphicsSystem.hpp"
#include "Systems/Base/Surface.hpp"
#include "Systems/Base/System.hpp"
#include "Utilities/File.hpp"


using namespace std;
namespace fs = boost::filesystem;

// -----------------------------------------------------------------------

GraphicsObjectOfFile::GraphicsObjectOfFile(System& system)
    : system_(system),
      filename_(""),
      frame_time_(0),
      current_frame_(0),
      time_at_last_frame_change_(0) {
}

// -----------------------------------------------------------------------

GraphicsObjectOfFile::GraphicsObjectOfFile(
  const GraphicsObjectOfFile& obj)
  : GraphicsObjectData(obj),
    system_(obj.system_),
    filename_(obj.filename_),
    surface_(obj.surface_),
    frame_time_(obj.frame_time_),
    current_frame_(obj.current_frame_),
    time_at_last_frame_change_(obj.time_at_last_frame_change_) {}

// -----------------------------------------------------------------------

GraphicsObjectOfFile::GraphicsObjectOfFile(
    System& system, const std::string& filename)
    : system_(system),
      filename_(filename),
      frame_time_(0),
      current_frame_(0),
      time_at_last_frame_change_(0) {
  loadFile();
}

// -----------------------------------------------------------------------

GraphicsObjectOfFile::~GraphicsObjectOfFile() {}

// -----------------------------------------------------------------------

void GraphicsObjectOfFile::loadFile() {
  surface_ = system_.graphics().loadNonCGSurfaceFromFile(filename_);
  surface_->EnsureUploaded();
}

// -----------------------------------------------------------------------

int GraphicsObjectOfFile::pixelWidth(const GraphicsObject& rp) {
  const Surface::GrpRect& rect = surface_->getPattern(rp.pattNo());
  int width = rect.rect.width();
  return int(rp.getWidthScaleFactor() * width);
}

// -----------------------------------------------------------------------

int GraphicsObjectOfFile::pixelHeight(const GraphicsObject& rp) {
  const Surface::GrpRect& rect = surface_->getPattern(rp.pattNo());
  int height = rect.rect.height();
  return int(rp.getHeightScaleFactor() * height);
}

// -----------------------------------------------------------------------

GraphicsObjectData* GraphicsObjectOfFile::clone() const {
  return new GraphicsObjectOfFile(*this);
}

// -----------------------------------------------------------------------

void GraphicsObjectOfFile::execute(RLMachine& machine) {
  if (currentlyPlaying()) {
    unsigned int current_time = system_.event().getTicks();
    unsigned int time_since_last_frame_change =
      current_time - time_at_last_frame_change_;

    while (time_since_last_frame_change > frame_time_) {
      current_frame_++;
      if (current_frame_ == surface_->numPatterns()) {
        current_frame_--;
        endAnimation();
      }

      time_at_last_frame_change_ += frame_time_;
      time_since_last_frame_change = current_time - time_at_last_frame_change_;
      system_.graphics().markScreenAsDirty(GUT_DISPLAY_OBJ);
    }
  }
}

// -----------------------------------------------------------------------

bool GraphicsObjectOfFile::isAnimation() const {
  return surface_->numPatterns();
}

// -----------------------------------------------------------------------

void GraphicsObjectOfFile::loopAnimation() {
  current_frame_ = 0;
}

// -----------------------------------------------------------------------

boost::shared_ptr<const Surface> GraphicsObjectOfFile::currentSurface(
  const GraphicsObject& rp) {
  return surface_;
}

// -----------------------------------------------------------------------

Rect GraphicsObjectOfFile::srcRect(const GraphicsObject& go) {
  if (time_at_last_frame_change_ != 0) {
    // If we've ever been treated as an animation, we need to continue acting
    // as an animation even if we've stopped.
    return surface_->getPattern(current_frame_).rect;
  }

  return GraphicsObjectData::srcRect(go);
}

// -----------------------------------------------------------------------

void GraphicsObjectOfFile::objectInfo(std::ostream& tree) {
  tree << "  Image: " << filename_ << endl;
}

// -----------------------------------------------------------------------

void GraphicsObjectOfFile::playSet(int frame_time) {
  setCurrentlyPlaying(true);
  frame_time_ = frame_time;
  current_frame_ = 0;

  if (frame_time_ == 0) {
    cerr << "WARNING: GraphicsObjectOfFile::playSet(0) is invalid;"
         << " this is probably going to cause a graphical glitch..."
         << endl;
    frame_time_ = 10;
  }

  time_at_last_frame_change_ = system_.event().getTicks();
  system_.graphics().markScreenAsDirty(GUT_DISPLAY_OBJ);
}

// -----------------------------------------------------------------------

template<class Archive>
void GraphicsObjectOfFile::load(Archive& ar, unsigned int version) {
  ar & boost::serialization::base_object<GraphicsObjectData>(*this)
    & filename_ & frame_time_ & current_frame_ & time_at_last_frame_change_;

  loadFile();

  // Saving |time_at_last_frame_change_| as part of the format is obviously a
  // mistake, but is now baked into the file format. Ask the clock for a more
  // suitable value.
  if (time_at_last_frame_change_ != 0) {
    time_at_last_frame_change_ = system_.event().getTicks();
    system_.graphics().markScreenAsDirty(GUT_DISPLAY_OBJ);
  }
}

// -----------------------------------------------------------------------

template<class Archive>
void GraphicsObjectOfFile::save(Archive& ar, unsigned int version) const {
  ar & boost::serialization::base_object<GraphicsObjectData>(*this)
    & filename_ & frame_time_ & current_frame_ & time_at_last_frame_change_;
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
