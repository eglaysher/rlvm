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

#include "Systems/Base/HIKScript.hpp"

#include <boost/filesystem.hpp>
#include <boost/scoped_array.hpp>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "MachineBase/RLMachine.hpp"
#include "Systems/Base/EventSystem.hpp"
#include "Systems/Base/GraphicsSystem.hpp"
#include "Systems/Base/Surface.hpp"
#include "Systems/Base/System.hpp"
#include "Utilities/Exception.hpp"
#include "Utilities/File.hpp"
#include "Utilities/Graphics.hpp"
#include "libReallive/defs.h"

using namespace std;
namespace fs = boost::filesystem;
using boost::scoped_array;

// TODO(erg): Move everything to a common reader.
using libReallive::read_i32;

namespace {

int consume_i32(const char*& curpointer) {
  int x = read_i32(curpointer);
  curpointer += 4;
  return x;
}

std::string consume_string(const char*& curpointer) {
  int size = consume_i32(curpointer);
  std::string x(curpointer, size - 1);
  curpointer += size;
  return x;
}

}  // namespace


HIKScript::HIKScript(System& system, const fs::path& file)
    : system_(system),
      creation_time_(0),
      explicit_frame_(0),
      x_offset_(0),
      y_offset_(0) {
  loadHikFile(file);
}

HIKScript::~HIKScript() {
}

void HIKScript::loadHikFile(const fs::path& file) {
  // This is dumb. This all needs to rewritten as either FILE or stream.
  int file_size = 0;
  scoped_array<char> hik_data;
  if (loadFileData(file, hik_data, file_size)) {
    ostringstream oss;
    oss << "Could not read the contents of \"" << file << "\"";
    throw rlvm::Exception(oss.str());
  }

  const char* curpointer = hik_data.get();
  const char* endpointer = hik_data.get() + file_size;
  int a = consume_i32(curpointer);
  int b = consume_i32(curpointer);
  if (a != 10000 || b != 10000) {
    ostringstream oss;
    oss << "HIK Parse error: Invalid magic";
    throw std::runtime_error(oss.str());
  }

  while (curpointer < endpointer) {
    int property_id = consume_i32(curpointer);
    switch (property_id) {
      case 10100:
      case 10101:
      case 10102: {
        consume_i32(curpointer);
        break;
      }
      case 10103: {
        int width = consume_i32(curpointer);
        int height = consume_i32(curpointer);
        size_of_hik_ = Size(width, height);
        break;
      }
      case 20000: {
        number_of_layers_ = consume_i32(curpointer);
        break;
      }
      case 20001: {
        consume_i32(curpointer);
        layers_.push_back(Layer());
        break;
      }
      case 20100: {
        // String name of this layer? We can't make use of this.
        consume_string(curpointer);
        break;
      }
      case 20101: {
        int x = consume_i32(curpointer);
        int y = consume_i32(curpointer);
        currentLayer().top_offset = Point(x, y);
        break;
      }
      case 21000: {
        consume_i32(curpointer);
        break;
      }
      case 21001: {
        consume_i32(curpointer);
        consume_i32(curpointer);
        consume_i32(curpointer);
        consume_i32(curpointer);
        break;
      }
      case 21002: {
        consume_i32(curpointer);
        consume_i32(curpointer);
        consume_i32(curpointer);
        consume_i32(curpointer);
        consume_i32(curpointer);
        break;
      }
      case 21003: {
        consume_i32(curpointer);
        break;
      }
      case 21100: {
        consume_i32(curpointer);
        break;
      }
      case 21101: {
        consume_i32(curpointer);
        consume_i32(curpointer);
        consume_i32(curpointer);
        consume_i32(curpointer);
        break;
      }
      case 21200: {
        currentLayer().use_scrolling = consume_i32(curpointer);
        break;
      }
      case 21201: {
        int x = consume_i32(curpointer);
        int y = consume_i32(curpointer);
        currentLayer().start_point = Point(x, y);
        x = consume_i32(curpointer);
        y = consume_i32(curpointer);
        currentLayer().end_point = Point(x, y);
        break;
      }
      case 21202: {
        currentLayer().x_scroll_time_ms = consume_i32(curpointer);
        currentLayer().y_scroll_time_ms = consume_i32(curpointer);
        break;
      }
      case 21203: {
        consume_i32(curpointer);
        break;
      }
      case 21301: {
        currentLayer().use_clip_area = consume_i32(curpointer);
        break;
      }
      case 21300: {
        // GRP or REC?
        int x = consume_i32(curpointer);
        int y = consume_i32(curpointer);
        int x2 = consume_i32(curpointer);
        int y2 = consume_i32(curpointer);
        currentLayer().clip_area = Rect::GRP(x, y, x2, y2);
        break;
      }
      case 30000: {
        currentLayer().number_of_animations = consume_i32(curpointer);
        break;
      }
      case 30001: {
        consume_i32(curpointer);
        currentLayer().animations.push_back(Animation());
        break;
      }
      case 30100: {
        currentAnimation().use_multiframe_animation = consume_i32(curpointer);
        break;
      }
      case 30101: {
        currentAnimation().i_30101 = consume_i32(curpointer);
        break;
      }
      case 30102: {
        currentAnimation().i_30102 = consume_i32(curpointer);
        break;
      }
      case 40000: {
        currentAnimation().number_of_frames = consume_i32(curpointer);
        break;
      }
      case 40101: {
        for (int i = 0; i < 31; ++i) {
          consume_i32(curpointer);
        }

        currentAnimation().frames.push_back(Frame());
        break;
      }
      case 40102: {
        currentFrame().opacity = consume_i32(curpointer);
        break;
      }
      case 40103: {
        consume_i32(curpointer);
        consume_i32(curpointer);
        break;
      }
      case 40100: {
        Frame& frame = currentFrame();
        frame.image = consume_string(curpointer);
        frame.surface = system_.graphics().loadNonCGSurfaceFromFile(
            frame.image);
        if (!frame.surface) {
          ostringstream oss;
          oss << "Could not load image " << frame.image << " for HIK";
          throw rlvm::Exception(oss.str());
        }
        frame.grp_pattern = consume_i32(curpointer);
        frame.frame_length_ms = consume_i32(curpointer);
        break;
      }
      default: {
        ostringstream oss;
        oss << "HIK Parse exception. Unknown id: " << property_id;
        throw rlvm::Exception(oss.str());
        break;
      }
    }
  }

  // For every Animation, sum up the frame_length_ms.
  for (std::vector<Layer>::iterator it = layers_.begin();
       it != layers_.end(); ++it) {
    for (std::vector<Animation>::iterator jt = it->animations.begin();
         jt != it->animations.end(); ++jt) {
      jt->total_time = 0;
      for (std::vector<Frame>::const_iterator kt = jt->frames.begin();
           kt != jt->frames.end(); ++kt) {
        jt->total_time += kt->frame_length_ms;
      }
    }
  }

  // Records are in reverse order of what they should be.
  std::reverse(layers_.begin(), layers_.end());

  creation_time_ = system_.event().getTicks();
}

void HIKScript::execute(RLMachine& machine) {
  machine.system().graphics().markScreenAsDirty(GUT_DRAW_HIK);
}

void HIKScript::render(std::ostream* tree) {
  int time_since_creation = system_.event().getTicks() - creation_time_;

  if (tree) {
    *tree << "  HIK Script:" << endl;
  }

  for (std::vector<Layer>::iterator it = layers_.begin();
       it != layers_.end(); ++it) {
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

    for (std::vector<Animation>::iterator jt = it->animations.begin();
         jt != it->animations.end(); ++jt) {
      size_t frame_to_use = 0;
      if (jt->use_multiframe_animation) {
        int ticks_since_sequence_start = time_since_creation % jt->total_time;

        // Separate out the remainder here. Need to check since
        // |ticks_since_sequence_start| may already be 0.
        while (ticks_since_sequence_start > 0) {
          ticks_since_sequence_start -=
              jt->frames.at(frame_to_use).frame_length_ms;

          if (ticks_since_sequence_start > 0) {
            frame_to_use++;
            if (frame_to_use > jt->frames.size()) {
              frame_to_use = 0;
              cerr << "Maybe an impossible situation?";
            }
          }
        }
      }

      const Frame& frame = jt->frames.at(frame_to_use);

      int pattern_to_use = 0;
      if (frame.grp_pattern != -1)
        pattern_to_use = frame.grp_pattern;

      Rect src_rect = frame.surface->getPattern(pattern_to_use).rect;
      src_rect = Rect(src_rect.origin() + Size(x_offset_, y_offset_),
                      src_rect.size());
      Rect dest_rect(dest_point, src_rect.size());
      if (it->use_clip_area)
        ClipDestination(it->clip_area, src_rect, dest_rect);

      frame.surface->renderToScreen(src_rect, dest_rect, frame.opacity);

      if (tree) {
        *tree << "    [L:" << (std::distance(layers_.begin(), it) + 1) << "/"
              << layers_.size() << ", A:"
              << (std::distance(it->animations.begin(), jt) + 1) << "/"
              << it->animations.size()
              << ", F:" << (frame_to_use+1) << "/" << jt->frames.size()
              << ", P:" << pattern_to_use
              << ", O:" << frame.opacity << ", Image: " << frame.image
              << "]" << endl;
      }
    }
  }
}

void HIKScript::NextAnimationFrame() {
  explicit_frame_++;
}

HIKScript::Layer& HIKScript::currentLayer() {
  if (layers_.size() == 0) {
    throw rlvm::Exception("Invalid layer reference");
  }

  return layers_.back();
}

HIKScript::Animation& HIKScript::currentAnimation() {
  Layer& layer = currentLayer();
  if (layer.animations.size() == 0) {
    throw rlvm::Exception("Invalid unkowns reference");
  }

  return layer.animations.back();
}

HIKScript::Frame& HIKScript::currentFrame() {
  Animation& animation = currentAnimation();
  if (animation.frames.size() == 0) {
    throw rlvm::Exception("Invalid frame reference");
  }

  return animation.frames.back();
}
