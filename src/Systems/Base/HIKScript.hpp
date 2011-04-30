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

#ifndef SRC_SYSTEMS_BASE_HIKSCRIPT_HPP_
#define SRC_SYSTEMS_BASE_HIKSCRIPT_HPP_

#include <string>
#include <vector>

#include <boost/filesystem.hpp>
#include <boost/shared_ptr.hpp>

#include "Systems/Base/Rect.hpp"

class RLMachine;
class System;
class Surface;

// Class that parses and executes HIK files.
class HIKScript {
 public:
  HIKScript(System& system, const boost::filesystem::path& file);
  ~HIKScript();

  void loadHikFile(const boost::filesystem::path& file);

  // Run once per tick.
  void execute(RLMachine& machine);

  void render(std::ostream* os);

  // RL bytecode controlled offsets from the top left corner of the source
  // image.
  void set_x_offset(int offset) { x_offset_ = offset; }
  void set_y_offset(int offset) { y_offset_ = offset; }

 private:
  // The contents of the 40000 keys which define an individual frame.
  struct Frame {
    int opacity;
    std::string image;
    boost::shared_ptr<Surface> surface;

    int grp_pattern;
    int frame_length_ms;
  };

  // The contents of the 30000 keys. I used to call this structure Unkowns;
  // "Animation" is a tentative name as it contains individual Frames that are
  // played in sequence.
  struct Animation {
    int use_multiframe_animation;

    // The number of frames as reported by the HIK file. Used for error
    // checking.
    int number_of_frames;

    // All frames to display.
    std::vector<Frame> frames;

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

  // Returns the current structure being operated on, throwing on logic errors.
  Animation& currentAnimation();
  Layer& currentLayer();
  Frame& currentFrame();

  System& system_;

  // Each graphics component in the HIK script.
  std::vector<Layer> layers_;

  // The number of layers as reported by the HIK file. Used for error checking.
  int number_of_layers_;

  // Size of the hik graphic as reported by the hik.
  Size size_of_hik_;

  // Time when this HIK script was loaded (in ms since startup). Used for
  // animation.
  int creation_time_;

  // Bytecode controllable offset.
  int x_offset_;
  int y_offset_;
};

#endif  // SRC_SYSTEMS_BASE_HIKSCRIPT_HPP_
