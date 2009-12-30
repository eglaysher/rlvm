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

#include "Systems/Base/HIKScript.hpp"

#include <boost/filesystem.hpp>
#include <boost/scoped_array.hpp>
#include <string>
#include <sstream>
#include <iostream>

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
    : system_(system) {
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

  // Skip the header for now.
  int num_entries = read_i32(hik_data.get() + 0x30);
  const char* curpointer = hik_data.get() + 0x34;

  // Read a property id.
  bool eof = false;
  while (curpointer < hik_data.get() + file_size) {
    Record record;

    int property_id = consume_i32(curpointer);
    while (property_id != -1) {
      switch (property_id) {
        case 20001:
          // This may be |use_top_offet_|?
          consume_i32(curpointer);
          break;
        case 20100:
          consume_string(curpointer);
          break;
        case 20101: {
          int x = consume_i32(curpointer);
          int y = consume_i32(curpointer);
          record.top_offset = Point(x, y);
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
          record.use_scrolling = consume_i32(curpointer);
          break;
        }
        case 21201: {
          int x = consume_i32(curpointer);
          int y = consume_i32(curpointer);
          record.start_point = Point(x, y);
          x = consume_i32(curpointer);
          y = consume_i32(curpointer);
          record.end_point = Point(x, y);
          break;
        }
        case 21202: {
          record.x_scroll_time_ms = consume_i32(curpointer);
          record.y_scroll_time_ms = consume_i32(curpointer);
          break;
        }
        case 21203: {
          consume_i32(curpointer);
          break;
        }
        case 21301: {
          record.use_clip_area = consume_i32(curpointer);
          break;
        }
        case 21300: {
          // GRP or REC?
          int x = consume_i32(curpointer);
          int y = consume_i32(curpointer);
          int x2 = consume_i32(curpointer);
          int y2 = consume_i32(curpointer);
          record.clip_area = Rect::GRP(x, y, x2, y2);
          break;
        }
        case 30000:
        case 30001:
        case 30100:
        case 30101:
        case 30102: {
          consume_i32(curpointer);
          break;
        }
        case 40000: {
          consume_i32(curpointer);
          break;
        }
        case 40101: {
          for (int i = 0; i < 31; ++i) {
            consume_i32(curpointer);
          }
          break;
        }
        case 40102: {
          record.opacity = consume_i32(curpointer);
          break;
        }
        case 40103: {
          consume_i32(curpointer);
          consume_i32(curpointer);
          break;
        }
        case 40100: {
          record.image = consume_string(curpointer);
          record.surface = system_.graphics().loadNonCGSurfaceFromFile(
              record.image);
          if (!record.surface) {
            ostringstream oss;
            oss << "Could not load image " << record.image << " for HIK";
            throw rlvm::Exception(oss.str());
          }
          break;
        }
        default: {
          ostringstream oss;
          oss << "HIK Parse exception. Unknown id: " << property_id;
          throw rlvm::Exception(oss.str());
          break;
        }
      }

      property_id = consume_i32(curpointer);
    }

    // We only work with Planetarian's HIK files right now, which have a 100
    // here. Other games have a different values.
    if (consume_i32(curpointer) != 100) {
      throw rlvm::Exception("HIK Parse exception: Different terminator");
    }

    records_.push_back(record);
  }

  // Records are in reverse order of what they should be.
  std::reverse(records_.begin(), records_.end());

  creation_time_ = system_.event().getTicks();
}

void HIKScript::execute(RLMachine& machine) {
  machine.system().graphics().markScreenAsDirty(GUT_DRAW_HIK);
}

void HIKScript::render(std::ostream* tree) {
  int time_since_creation = system_.event().getTicks() - creation_time_;
  for (std::vector<Record>::const_iterator it = records_.begin();
       it != records_.end(); ++it) {
    // Calculate the source rectangle

    // TODO(erg): Should top_offset only be conditionally added?
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

    Rect src_rect = it->surface->rect();
    Rect dest_rect(dest_point, src_rect.size());
    if (it->use_clip_area)
      ClipDestination(it->clip_area, src_rect, dest_rect);

    it->surface->renderToScreen(src_rect, dest_rect, it->opacity);

    if (tree) {
      *tree << "  [" << it->image << "]" << endl;
    }
  }
}
