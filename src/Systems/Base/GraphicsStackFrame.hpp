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
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
//
// -----------------------------------------------------------------------


#ifndef SRC_SYSTEMS_BASE_GRAPHICSSTACKFRAME_HPP_
#define SRC_SYSTEMS_BASE_GRAPHICSSTACKFRAME_HPP_

#include "Systems/Base/Rect.hpp"

#include <boost/serialization/access.hpp>
#include <iosfwd>
#include <string>

// -----------------------------------------------------------------------

/**
 * Represents one frame in the graphics stack.
 *
 * @todo Move the serialization code into this class
 */
class GraphicsStackFrame {
 public:
  GraphicsStackFrame();
  explicit GraphicsStackFrame(const std::string& name);
  ~GraphicsStackFrame();

  const std::string& name() const { return command_name_; }

  bool hasFilename() const { return has_filename_; }
  const std::string& filename() const { return file_name_; }
  GraphicsStackFrame& setFilename(const std::string& filename);

  // source stuff
  bool hasSourceDC() const { return has_source_dc_; }
  int sourceDC() const { return source_dc_; }
  GraphicsStackFrame& setSourceDC(int in);

  bool hasSourceCoordinates() const { return has_source_coordinates_; }
  Rect sourceRect() const { return source_rect_; }
  GraphicsStackFrame& setSourceCoordinates(const Rect& in);

  bool hasTargetDC() const { return has_target_dc_; }
  int targetDC() const { return target_dc_; }
  GraphicsStackFrame& setTargetDC(int in);

  bool hasTargetCoordinates() const { return has_target_coordinates_; }
  Point targetPoint() const { return target_point_; }
  GraphicsStackFrame& setTargetCoordinates(const Point& target_point);

  bool hasRGB() const { return has_rgb_; }
  int r() const { return r_; }
  int g() const { return g_; }
  int b() const { return b_; }
  GraphicsStackFrame& setRGB(int r, int g, int b);

  bool hasOpacity() const { return has_opacity_; }
  int opacity() const { return opacity_; }
  GraphicsStackFrame& setOpacity(int in);

  bool hasMask() const { return has_mask_; }
  bool mask() const { return mask_; }
  GraphicsStackFrame& setMask(bool in);

 private:
  std::string command_name_;

  bool has_filename_;
  std::string file_name_;

  bool has_source_dc_;
  int source_dc_;

  bool has_source_coordinates_;
  Rect source_rect_;

  bool has_target_dc_;
  int target_dc_;

  bool has_target_coordinates_;
  Point target_point_;

  bool has_rgb_;
  int r_, g_, b_;

  bool has_opacity_;
  int opacity_;

  bool has_mask_;
  int mask_;

  friend class boost::serialization::access;

  /// boost::serialization support
  template<class Archive>
  void serialize(Archive& ar, unsigned int version) {
    ar & command_name_ & has_filename_ & file_name_ & has_source_dc_
      & source_dc_ & has_source_coordinates_ & source_rect_
      & has_target_dc_ & target_dc_
      & has_target_coordinates_ & target_point_
      & has_rgb_ & r_ & g_ & b_ & has_opacity_
      & opacity_ & has_mask_ & mask_;
  }
};  // end of class GraphicsStackFrame

// -----------------------------------------------------------------------

std::ostream& operator<<(std::ostream&, const GraphicsStackFrame&);

#endif  // SRC_SYSTEMS_BASE_GRAPHICSSTACKFRAME_HPP_
