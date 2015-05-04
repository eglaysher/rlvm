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

#include "systems/base/hik_script.h"

#include <boost/filesystem.hpp>
#include <sstream>
#include <string>
#include <vector>

#include "libreallive/defs.h"
#include "machine/rlmachine.h"
#include "systems/base/graphics_system.h"
#include "systems/base/surface.h"
#include "systems/base/system.h"
#include "utilities/exception.h"
#include "utilities/file.h"
#include "utilities/graphics.h"

namespace fs = boost::filesystem;

// TODO(erg): Move everything to a common reader.
using libreallive::read_i32;

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

HIKScript::HIKScript(System& system, const fs::path& file) {
  LoadHikFile(system, file);
}

HIKScript::~HIKScript() {}

void HIKScript::LoadHikFile(System& system, const fs::path& file) {
  // This is dumb. This all needs to rewritten as either FILE or stream.
  int file_size = 0;
  std::unique_ptr<char[]> hik_data;
  if (LoadFileData(file, hik_data, file_size)) {
    std::ostringstream oss;
    oss << "Could not read the contents of \"" << file << "\"";
    throw rlvm::Exception(oss.str());
  }

  const char* curpointer = hik_data.get();
  const char* endpointer = hik_data.get() + file_size;
  int a = consume_i32(curpointer);
  int b = consume_i32(curpointer);
  if (a != 10000 || b != 10000) {
    std::ostringstream oss;
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
        layers_.emplace_back();
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
        CurrentLayer().top_offset = Point(x, y);
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
        CurrentLayer().use_scrolling = consume_i32(curpointer);
        break;
      }
      case 21201: {
        int x = consume_i32(curpointer);
        int y = consume_i32(curpointer);
        CurrentLayer().start_point = Point(x, y);
        x = consume_i32(curpointer);
        y = consume_i32(curpointer);
        CurrentLayer().end_point = Point(x, y);
        break;
      }
      case 21202: {
        CurrentLayer().x_scroll_time_ms = consume_i32(curpointer);
        CurrentLayer().y_scroll_time_ms = consume_i32(curpointer);
        break;
      }
      case 21203: {
        consume_i32(curpointer);
        break;
      }
      case 21301: {
        CurrentLayer().use_clip_area = consume_i32(curpointer);
        break;
      }
      case 21300: {
        // GRP or REC?
        int x = consume_i32(curpointer);
        int y = consume_i32(curpointer);
        int x2 = consume_i32(curpointer);
        int y2 = consume_i32(curpointer);
        CurrentLayer().clip_area = Rect::GRP(x, y, x2, y2);
        break;
      }
      case 30000: {
        CurrentLayer().number_of_animations = consume_i32(curpointer);
        break;
      }
      case 30001: {
        consume_i32(curpointer);
        CurrentLayer().animations.emplace_back();
        break;
      }
      case 30100: {
        CurrentAnimation().use_multiframe_animation = consume_i32(curpointer);
        break;
      }
      case 30101: {
        CurrentAnimation().i_30101 = consume_i32(curpointer);
        break;
      }
      case 30102: {
        CurrentAnimation().i_30102 = consume_i32(curpointer);
        break;
      }
      case 40000: {
        CurrentAnimation().number_of_frames = consume_i32(curpointer);
        break;
      }
      case 40101: {
        for (int i = 0; i < 31; ++i) {
          consume_i32(curpointer);
        }

        CurrentAnimation().frames.emplace_back();
        break;
      }
      case 40102: {
        CurrentFrame().opacity = consume_i32(curpointer);
        break;
      }
      case 40103: {
        consume_i32(curpointer);
        consume_i32(curpointer);
        break;
      }
      case 40100: {
        Frame& frame = CurrentFrame();
        frame.image = consume_string(curpointer);
        frame.surface = system.graphics().GetSurfaceNamed(frame.image);
        if (!frame.surface) {
          std::ostringstream oss;
          oss << "Could not load image " << frame.image << " for HIK";
          throw rlvm::Exception(oss.str());
        }
        frame.grp_pattern = consume_i32(curpointer);
        frame.frame_length_ms = consume_i32(curpointer);
        break;
      }
      default: {
        std::ostringstream oss;
        oss << "HIK Parse exception. Unknown id: " << property_id;
        throw rlvm::Exception(oss.str());
        break;
      }
    }
  }

  // For every Animation, sum up the frame_length_ms.
  for (Layer& layer : layers_) {
    for (Animation& animation : layer.animations) {
      animation.total_time = 0;
      for (Frame& frame : animation.frames) {
        animation.total_time += frame.frame_length_ms;
      }
    }
  }

  // Records are in reverse order of what they should be.
  std::reverse(layers_.begin(), layers_.end());
}

void HIKScript::EnsureUploaded() {
  // Force every frame to be uploaded.
  for (Layer& layer : layers_) {
    for (Animation& animation : layer.animations) {
      for (Frame& frame : animation.frames) {
        frame.surface->EnsureUploaded();
      }
    }
  }
}

HIKScript::Layer& HIKScript::CurrentLayer() {
  if (layers_.size() == 0) {
    throw rlvm::Exception("Invalid layer reference");
  }

  return layers_.back();
}

HIKScript::Animation& HIKScript::CurrentAnimation() {
  Layer& layer = CurrentLayer();
  if (layer.animations.size() == 0) {
    throw rlvm::Exception("Invalid unknowns reference");
  }

  return layer.animations.back();
}

HIKScript::Frame& HIKScript::CurrentFrame() {
  Animation& animation = CurrentAnimation();
  if (animation.frames.size() == 0) {
    throw rlvm::Exception("Invalid frame reference");
  }

  return animation.frames.back();
}
