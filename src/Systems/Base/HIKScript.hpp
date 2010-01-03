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
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
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
  void set_y_offset(int offset) { y_offset_ = offset; }

 private:
  struct Record {
    Point top_offset;

    bool use_scrolling;
    Point start_point;
    Point end_point;
    int x_scroll_time_ms;
    int y_scroll_time_ms;

    bool use_clip_area;
    Rect clip_area;

    int opacity;
    std::string image;
    boost::shared_ptr<Surface> surface;
  };

  System& system_;

  // Each graphics component in the HIK script.
  std::vector<Record> records_;

  // Time when this HIK script was loaded (in ms since startup). Used for
  // animation.
  int creation_time_;

  // Bytecode controllable offset.
  int y_offset_;
};

#endif  // SRC_SYSTEMS_BASE_HIKSCRIPT_HPP_
