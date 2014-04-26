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
// Copyright (C) 2000 Kazunori Ueno(JAGARL) <jagarl@creator.club.ne.jp>
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

// The code in this file has been modified from the file anm.cc in
// Jagarl's xkanon project.

#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/serialization/export.hpp>

#include "machine/serialization.h"
#include "systems/base/anm_graphics_object_data.h"

#include <iterator>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>

#include "libreallive/defs.h"
#include "systems/base/event_system.h"
#include "systems/base/graphics_object.h"
#include "systems/base/graphics_system.h"
#include "systems/base/surface.h"
#include "systems/base/system.h"
#include "utilities/exception.h"
#include "utilities/file.h"
#include "utilities/graphics.h"

using libreallive::read_i32;

namespace fs = boost::filesystem;

// -----------------------------------------------------------------------

// Constants
static const int ANM_MAGIC_SIZE = 12;
static const char ANM_MAGIC[ANM_MAGIC_SIZE] = {'A', 'N', 'M', '3', '2', 0,
                                               0,   0,   0,   1,   0,   0};

// -----------------------------------------------------------------------
// AnmGraphicsObjectData
// -----------------------------------------------------------------------

AnmGraphicsObjectData::AnmGraphicsObjectData(System& system)
    : system_(system), current_set_(-1) {}

AnmGraphicsObjectData::AnmGraphicsObjectData(System& system,
                                             const std::string& file)
    : system_(system), filename_(file), current_set_(-1) {
  LoadAnmFile();
}

AnmGraphicsObjectData::~AnmGraphicsObjectData() {}

bool AnmGraphicsObjectData::TestFileMagic(std::unique_ptr<char[]>& anm_data) {
  return memcmp(anm_data.get(), ANM_MAGIC, ANM_MAGIC_SIZE) != 0;
}

void AnmGraphicsObjectData::LoadAnmFile() {
  fs::path file = system_.FindFile(filename_, ANM_FILETYPES);
  if (file.empty()) {
    std::ostringstream oss;
    oss << "Could not find ANM file \"" << filename_ << "\".";
    throw rlvm::Exception(oss.str());
  }

  int file_size = 0;
  std::unique_ptr<char[]> anm_data;
  if (LoadFileData(file, anm_data, file_size)) {
    std::ostringstream oss;
    oss << "Could not read the contents of \"" << file << "\"";
    throw rlvm::Exception(oss.str());
  }

  if (TestFileMagic(anm_data)) {
    std::ostringstream oss;
    oss << "File \"" << file << "\" does not appear to be in ANM format.";
    throw rlvm::Exception(oss.str());
  }

  LoadAnmFileFromData(anm_data);
}

void AnmGraphicsObjectData::LoadAnmFileFromData(
    const std::unique_ptr<char[]>& anm_data) {
  const char* data = anm_data.get();

  // Read the header
  int frames_len = read_i32(data + 0x8c);
  int framelist_len = read_i32(data + 0x90);
  int animation_set_len = read_i32(data + 0x94);
  if (animation_set_len < 0) {
    throw rlvm::Exception(
        "Impossible value for animation_set_len in ANM file.");
  }

  // Read the corresponding image file we read from, and load the image.
  string raw_file_name = data + 0x1c;
  image_ = system_.graphics().GetSurfaceNamed(raw_file_name);
  image_->EnsureUploaded();

  // Read the frame list
  const char* buf = data + 0xb8;
  Size screen_size = GetScreenSize(system_.gameexe());
  for (int i = 0; i < frames_len; ++i) {
    Frame f;
    f.src_x1 = read_i32(buf);
    f.src_y1 = read_i32(buf + 4);
    f.src_x2 = read_i32(buf + 8);
    f.src_y2 = read_i32(buf + 12);
    f.dest_x = read_i32(buf + 16);
    f.dest_y = read_i32(buf + 20);
    f.time = read_i32(buf + 0x38);
    FixAxis(f, screen_size.width(), screen_size.height());
    frames.push_back(f);

    buf += 0x60;
  }

  ReadIntegerList(
      data + 0xb8 + frames_len * 0x60, 0x68, framelist_len, framelist_);
  ReadIntegerList(data + 0xb8 + frames_len * 0x60 + framelist_len * 0x68,
                  0x78,
                  animation_set_len,
                  animation_set_);
}

void AnmGraphicsObjectData::ReadIntegerList(
    const char* start,
    int offset,
    int iterations,
    std::vector<std::vector<int>>& dest) {
  for (int i = 0; i < iterations; ++i) {
    int list_length = read_i32(start + 4);
    const char* tmpbuf = start + 8;
    std::vector<int> intlist;
    for (int j = 0; j < list_length; ++j) {
      intlist.push_back(read_i32(tmpbuf));
      tmpbuf += 4;
    }
    dest.push_back(intlist);

    start += offset;
  }
}

void AnmGraphicsObjectData::FixAxis(Frame& frame, int width, int height) {
  if (frame.src_x1 > frame.src_x2) {  // swap
    int tmp = frame.src_x1;
    frame.src_x1 = frame.src_x2;
    frame.src_x2 = tmp;
  }
  if (frame.src_y1 > frame.src_y2) {  // swap
    int tmp = frame.src_y1;
    frame.src_y1 = frame.src_y2;
    frame.src_y2 = tmp;
  }
  // check screen size
  // int tmp_x = frame.dest_x, tmp_y = frame.dest_y;
  if (frame.dest_x + (frame.src_x2 - frame.src_x1 + 1) > width) {
    frame.src_x2 = frame.src_x1 + width - frame.dest_x;
  }
  if (frame.dest_y + (frame.src_y2 - frame.src_y1 + 1) > width) {
    frame.src_y2 = frame.src_y1 + width - frame.dest_y;
  }
}

void AnmGraphicsObjectData::Execute(RLMachine& machine) {
  if (is_currently_playing())
    AdvanceFrame();
}

bool AnmGraphicsObjectData::IsAnimation() const {
  return true;
}

void AnmGraphicsObjectData::AdvanceFrame() {
  // Do things that advance the state
  int time_since_last_frame_change =
      system_.event().GetTicks() - time_at_last_frame_change_;
  bool done = false;

  while (is_currently_playing() && !done) {
    if (time_since_last_frame_change > frames[current_frame_].time) {
      time_since_last_frame_change -= frames[current_frame_].time;
      time_at_last_frame_change_ += frames[current_frame_].time;
      system_.graphics().MarkScreenAsDirty(GUT_DISPLAY_OBJ);

      cur_frame_++;
      if (cur_frame_ == cur_frame_end_) {
        cur_frame_set_++;
        if (cur_frame_set_ == cur_frame_set_end_) {
          set_is_currently_playing(false);
        } else {
          cur_frame_ = framelist_.at(*cur_frame_set_).begin();
          cur_frame_end_ = framelist_.at(*cur_frame_set_).end();
          current_frame_ = *cur_frame_;
        }
      } else {
        current_frame_ = *cur_frame_;
      }
    } else {
      done = true;
    }
  }
}

// I am not entirely sure these methods even make sense given the
// context...
int AnmGraphicsObjectData::PixelWidth(const GraphicsObject& rp) {
  const Surface::GrpRect& rect = image_->GetPattern(rp.GetPattNo());
  int width = rect.rect.width();
  return int(rp.GetWidthScaleFactor() * width);
}

int AnmGraphicsObjectData::PixelHeight(const GraphicsObject& rp) {
  const Surface::GrpRect& rect = image_->GetPattern(rp.GetPattNo());
  int height = rect.rect.height();
  return int(rp.GetHeightScaleFactor() * height);
}

GraphicsObjectData* AnmGraphicsObjectData::Clone() const {
  return new AnmGraphicsObjectData(*this);
}

void AnmGraphicsObjectData::PlaySet(int set) {
  set_is_currently_playing(true);
  time_at_last_frame_change_ = system_.event().GetTicks();

  cur_frame_set_ = animation_set_.at(set).begin();
  cur_frame_set_end_ = animation_set_.at(set).end();
  cur_frame_ = framelist_.at(*cur_frame_set_).begin();
  cur_frame_end_ = framelist_.at(*cur_frame_set_).end();
  current_frame_ = *cur_frame_;

  system_.graphics().MarkScreenAsDirty(GUT_DISPLAY_OBJ);
}

std::shared_ptr<const Surface> AnmGraphicsObjectData::CurrentSurface(
    const GraphicsObject& rp) {
  return image_;
}

Rect AnmGraphicsObjectData::SrcRect(const GraphicsObject& go) {
  if (current_frame_ != -1) {
    const Frame& frame = frames.at(current_frame_);
    return Rect::GRP(frame.src_x1, frame.src_y1, frame.src_x2, frame.src_y2);
  }

  return Rect();
}

Rect AnmGraphicsObjectData::DstRect(const GraphicsObject& go,
                                    const GraphicsObject* parent) {
  if (current_frame_ != -1) {
    // TODO(erg): Should this account for either |go| or |parent|?
    const Frame& frame = frames.at(current_frame_);
    return Rect::REC(frame.dest_x,
                     frame.dest_y,
                     (frame.src_x2 - frame.src_x1),
                     (frame.src_y2 - frame.src_y1));
  }

  return Rect();
}

void AnmGraphicsObjectData::ObjectInfo(std::ostream& tree) {
  tree << "  ANM file: " << filename_ << std::endl;
}

template <class Archive>
void AnmGraphicsObjectData::load(Archive& ar, unsigned int version) {
  ar& boost::serialization::base_object<GraphicsObjectData>(*this);

  ar& filename_;

  // Reconstruct the ANM data from whatever file was linked.
  LoadAnmFile();

  // Now load the rest of the data.
  ar& currently_playing_& current_set_;

  // Reconstruct the cur_* variables from their
  int cur_frame_set, current_frame;
  ar& cur_frame_set& current_frame;

  cur_frame_set_ = animation_set_.at(current_set_).begin();
  advance(cur_frame_set_, cur_frame_set);
  cur_frame_set_end_ = animation_set_.at(current_set_).end();

  cur_frame_ = framelist_.at(*cur_frame_set_).begin();
  advance(cur_frame_, current_frame);
  cur_frame_end_ = framelist_.at(*cur_frame_set_).end();
}

template <class Archive>
void AnmGraphicsObjectData::save(Archive& ar, unsigned int version) const {
  ar& boost::serialization::base_object<GraphicsObjectData>(*this);
  ar& filename_& currently_playing_& current_set_;

  // Figure out what set we're playing, which
  int cur_frame_set =
      distance(animation_set_.at(current_set_).begin(), cur_frame_set_);
  int current_frame =
      distance(framelist_.at(*cur_frame_set_).begin(), cur_frame_);

  ar& cur_frame_set& current_frame;
}

// -----------------------------------------------------------------------

template void AnmGraphicsObjectData::save<boost::archive::text_oarchive>(
    boost::archive::text_oarchive& ar,
    unsigned int version) const;

template void AnmGraphicsObjectData::load<boost::archive::text_iarchive>(
    boost::archive::text_iarchive& ar,
    unsigned int version);

BOOST_CLASS_EXPORT(AnmGraphicsObjectData);
