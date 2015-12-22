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

#include "systems/base/hik_renderer.h"

#include <iostream>

#include "machine/rlmachine.h"
#include "systems/base/event_system.h"
#include "systems/base/graphics_system.h"
#include "systems/base/hik_script.h"
#include "systems/base/surface.h"
#include "systems/base/system.h"
#include "utilities/graphics.h"

HIKRenderer::LayerData::LayerData(int time)
    : animation_num_(0), animation_start_time_(time) {}

HIKRenderer::HIKRenderer(System& system,
                         const std::shared_ptr<const HIKScript>& script)
    : system_(system),
      script_(script),
      creation_time_(system_.event().GetTicks()),
      x_offset_(0),
      y_offset_(0) {
  layer_to_animation_num_.insert(layer_to_animation_num_.begin(),
                                 script->layers().size(),
                                 LayerData(creation_time_));
}

HIKRenderer::~HIKRenderer() {}

void HIKRenderer::Execute(RLMachine& machine) {
  machine.system().graphics().MarkScreenAsDirty(GUT_DRAW_HIK);
}

void HIKRenderer::Render(std::ostream* tree) {
  int current_ticks = system_.event().GetTicks();
  int time_since_creation = current_ticks - creation_time_;

  if (tree) {
    *tree << "  HIK Script:" << std::endl;
  }

  int layer_num = 0;
  for (std::vector<HIKScript::Layer>::const_iterator
           it = script_->layers().begin();
       it != script_->layers().end();
       ++it, ++layer_num) {
    // Calculate the source rectangle

    Point dest_point = it->top_offset;
    if (it->use_scrolling) {
      dest_point += it->start_point;

      Size difference = it->end_point - it->start_point;
      int x_difference = 0;
      int y_difference = 0;
      if (it->x_scroll_time_ms) {
        double x_percent = (time_since_creation % it->x_scroll_time_ms) /
                           static_cast<float>(it->x_scroll_time_ms);
        x_difference = difference.width() * x_percent;
      }
      if (it->y_scroll_time_ms) {
        double y_percent = (time_since_creation % it->y_scroll_time_ms) /
                           static_cast<float>(it->y_scroll_time_ms);
        y_difference = difference.height() * y_percent;
      }

      dest_point += Point(x_difference, y_difference);
    }

    LayerData& layer_data = layer_to_animation_num_.at(layer_num);
    const HIKScript::Animation* animation =
        &it->animations.at(layer_data.animation_num_);
    size_t frame_to_use = 0;
    if (animation->use_multiframe_animation) {
      int ticks_since_animation_began =
          current_ticks - layer_data.animation_start_time_;

      // Advance to the correct animation.
      bool advanced = false;
      while (ticks_since_animation_began > animation->total_time) {
        switch (animation->i_30101) {
          case 0:
            // Don't change the animation number.
            break;
          case 3:
            // Move to the next animation.
            layer_data.animation_num_++;
            if (layer_data.animation_num_ == it->animations.size())
              layer_data.animation_num_ = 0;
            break;
          default:
            break;
        }

        animation = &it->animations.at(layer_data.animation_num_);
        ticks_since_animation_began -= animation->total_time;
        advanced = true;
      }

      if (advanced)
        layer_data.animation_start_time_ = current_ticks;

      while (ticks_since_animation_began > 0) {
        ticks_since_animation_began -=
            animation->frames.at(frame_to_use).frame_length_ms;

        if (ticks_since_animation_began > 0) {
          frame_to_use++;
          if (frame_to_use > animation->frames.size()) {
            frame_to_use = 0;
          }
        }
      }
    }

    const HIKScript::Frame& frame = animation->frames.at(frame_to_use);

    int pattern_to_use = 0;
    if (frame.grp_pattern != -1)
      pattern_to_use = frame.grp_pattern;

    Rect src_rect = frame.surface->GetPattern(pattern_to_use).rect;
    src_rect =
        Rect(src_rect.origin() + Size(x_offset_, y_offset_), src_rect.size());
    Rect dest_rect(dest_point, src_rect.size());
    if (it->use_clip_area)
      ClipDestination(it->clip_area, src_rect, dest_rect);

    frame.surface->RenderToScreen(src_rect, dest_rect, frame.opacity);

    if (tree) {
      *tree << "    [L:" << (std::distance(script_->layers().begin(), it) + 1)
            << "/" << script_->layers().size()
            << ", A:" << (layer_data.animation_num_ + 1) << "/"
            << it->animations.size() << ", F:" << (frame_to_use + 1) << "/"
            << animation->frames.size() << ", P:" << pattern_to_use
            << ", ??: " << animation->use_multiframe_animation << "/"
            << animation->i_30101 << "/" << animation->i_30102
            << ", O:" << frame.opacity << ", Image: " << frame.image << "]"
            << std::endl;
    }
  }
}

void HIKRenderer::NextAnimationFrame() {
  int time = system_.event().GetTicks();

  int idx = 0;
  for (std::vector<LayerData>::iterator it = layer_to_animation_num_.begin();
       it != layer_to_animation_num_.end();
       ++it, ++idx) {
    it->animation_num_++;
    if (it->animation_num_ == script_->layers().at(idx).animations.size())
      it->animation_num_ = 0;

    it->animation_start_time_ = time;
  }
}
