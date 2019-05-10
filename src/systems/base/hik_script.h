// -*- Mode: C++; tab-width:2; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi:tw=80:et:ts=2:sts=2
//
// -----------------------------------------------------------------------
//
// This file is part of RLVM, a RealLive virtual machine clone.
//
// -----------------------------------------------------------------------
//
// Copyright (C) 2009 Elliot Glaysher
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
// -----------------------------------------------------------------------

#ifndef SRC_SYSTEMS_BASE_HIK_SCRIPT_H_
#define SRC_SYSTEMS_BASE_HIK_SCRIPT_H_

#include <boost/filesystem.hpp>

#include <memory>
#include <string>
#include <vector>

#include "systems/base/rect.h"

class System;
class Surface;

// Class that parses and executes HIK files.
class HIKScript {
 public:
  HIKScript(System& system, const boost::filesystem::path& file);
  ~HIKScript();

  // Loads our data from a HIK file.
  void LoadHikFile(System& system, const boost::filesystem::path& file);

  // Make sure all graphics data is ready to be presented to the user.
  void EnsureUploaded();

  // The contents of the 40000 keys which define an individual frame.
  struct Frame {
    int opacity;
    std::string image;
    std::shared_ptr<const Surface> surface;

    int grp_pattern;
    int frame_length_ms;
  };

  // The contents of the 30000 keys. I used to call this structure Unknowns;
  // "Animation" is a tentative name as it contains individual Frames that are
  // played in sequence.
  struct Animation {
    int use_multiframe_animation;

    // The number of frames as reported by the HIK file. Used for error
    // checking.
    int number_of_frames;

    // All frames to display.
    std::vector<Frame> frames;

    // IDEA: This is the animation number in the layer to move to next when
    // all frames in this animation are played out.
    int i_30101;
    // Unknown
    int i_30102;

    // The sum of all |frame_length_ms| in frames.
    int total_time;
  };

  // The contents of the 20000 keys.
  struct Layer {
    Point top_offset;

    bool use_scrolling;
    Point start_point;
    Point end_point;
    int x_scroll_time_ms;
    int y_scroll_time_ms;

    bool use_clip_area;
    Rect clip_area;

    // Number of unknowns as reported by the HIK file on disk.
    int number_of_animations;

    std::vector<Animation> animations;
  };

  // Returns the HIK layer data.
  const std::vector<Layer>& layers() const { return layers_; }
  const Size& size() const { return size_of_hik_; }

 private:
  // Returns the current structure being operated on, throwing on logic
  // errors. Only to be used during parsing of the file.
  Animation& CurrentAnimation();
  Layer& CurrentLayer();
  Frame& CurrentFrame();

  // Each graphics component in the HIK script.
  std::vector<Layer> layers_;

  // The number of layers as reported by the HIK file. Used for error checking.
  int number_of_layers_;

  // Size of the hik graphic as reported by the hik.
  Size size_of_hik_;
};

#endif  // SRC_SYSTEMS_BASE_HIK_SCRIPT_H_
