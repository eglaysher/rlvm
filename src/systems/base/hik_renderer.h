// -*- Mode: C++; tab-width:2; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi:tw=80:et:ts=2:sts=2
//
// -----------------------------------------------------------------------
//
// This file is part of RLVM, a RealLive virtual machine clone.
//
// -----------------------------------------------------------------------
//
// Copyright (C) 2011 Elliot Glaysher
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

#ifndef SRC_SYSTEMS_BASE_HIK_RENDERER_H_
#define SRC_SYSTEMS_BASE_HIK_RENDERER_H_

#include <memory>
#include <vector>

class HIKScript;
class RLMachine;
class System;

// Displays a HIKScript at a certain time to the screen.
class HIKRenderer {
 public:
  HIKRenderer(System& system, const std::shared_ptr<const HIKScript>& script);
  ~HIKRenderer();

  // Run once per tick.
  void Execute(RLMachine& machine);

  void Render(std::ostream* os);

  // Advances to the next layer.
  void NextAnimationFrame();

  // RL bytecode controlled offsets from the top left corner of the source
  // image.
  void set_x_offset(int offset) { x_offset_ = offset; }
  void set_y_offset(int offset) { y_offset_ = offset; }

 private:
  System& system_;

  // The script data.
  std::shared_ptr<const HIKScript> script_;

  // Time when this HIK renderer was loaded (in ms since startup). Used for
  // animation.
  int creation_time_;

  // Bytecode controllable offset.
  int x_offset_;
  int y_offset_;

  struct LayerData {
    explicit LayerData(int time);
    int animation_num_;
    int animation_start_time_;
  };

  // Which animation frame to use per layer. Defaults to zero.
  std::vector<LayerData> layer_to_animation_num_;
};

#endif  // SRC_SYSTEMS_BASE_HIK_RENDERER_H_
