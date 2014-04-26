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
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
//
// -----------------------------------------------------------------------

#ifndef SRC_SYSTEMS_BASE_ANM_GRAPHICS_OBJECT_DATA_H_
#define SRC_SYSTEMS_BASE_ANM_GRAPHICS_OBJECT_DATA_H_

#include <boost/serialization/access.hpp>
#include <boost/serialization/split_member.hpp>

#include <memory>
#include <string>
#include <vector>

#include "machine/rlmachine.h"
#include "systems/base/graphics_object_data.h"

class Surface;
class System;

// Executable, in-memory representation of an ANM file. This internal structure
// is heavily based off of xkanon's ANM file implementation, but has been
// changed to be all C++ like.
class AnmGraphicsObjectData : public GraphicsObjectData {
 public:
  explicit AnmGraphicsObjectData(System& system);
  AnmGraphicsObjectData(System& system, const std::string& file);
  virtual ~AnmGraphicsObjectData();

  void LoadAnmFile();

  virtual int PixelWidth(const GraphicsObject& rendering_properties) override;
  virtual int PixelHeight(const GraphicsObject& rendering_properties) override;

  virtual GraphicsObjectData* Clone() const override;
  virtual void Execute(RLMachine& machine) override;

  virtual bool IsAnimation() const override;
  virtual void PlaySet(int set) override;

 protected:
  virtual std::shared_ptr<const Surface> CurrentSurface(
      const GraphicsObject& go) override;
  virtual Rect SrcRect(const GraphicsObject& go) override;
  virtual Rect DstRect(const GraphicsObject& go,
                       const GraphicsObject* parent) override;

  virtual void ObjectInfo(std::ostream& tree) override;

 private:
  // Advance the position in the animation.
  void AdvanceFrame();

  struct Frame {
    int src_x1, src_y1;
    int src_x2, src_y2;
    int dest_x, dest_y;
    int time;
  };

  bool TestFileMagic(std::unique_ptr<char[]>& anm_data);
  void ReadIntegerList(const char* start,
                       int offset,
                       int iterations,
                       std::vector<std::vector<int>>& dest);
  void LoadAnmFileFromData(const std::unique_ptr<char[]>& anm_data);
  void FixAxis(Frame& frame, int width, int height);

  // The system we are a part of.
  System& system_;

  // Raw, short name for the ANM file.
  std::string filename_;

  // Animation Data (This structure was stolen from xkanon.)
  std::vector<Frame> frames;
  std::vector<std::vector<int>> framelist_;
  std::vector<std::vector<int>> animation_set_;

  // The image the above coordinates map into.
  std::shared_ptr<const Surface> image_;

  bool currently_playing_;

  int current_set_;

  std::vector<int>::const_iterator cur_frame_set_;
  std::vector<int>::const_iterator cur_frame_set_end_;

  std::vector<int>::const_iterator cur_frame_;
  std::vector<int>::const_iterator cur_frame_end_;

  int current_frame_;

  unsigned int time_at_last_frame_change_;

  friend class boost::serialization::access;
  template <class Archive>
  void save(Archive& ar, const unsigned int file_version) const;

  template <class Archive>
  void load(Archive& ar, const unsigned int file_version);

  BOOST_SERIALIZATION_SPLIT_MEMBER()
};

// We need help creating AnmGraphicsObjectData s since they don't have a
// default constructor:
namespace boost {
namespace serialization {
template <class Archive>
inline void load_construct_data(Archive& ar,
                                AnmGraphicsObjectData* t,
                                const unsigned int file_version) {
  ::new (t) AnmGraphicsObjectData(Serialization::g_current_machine->system());
}
}  // namespace serialization
}  // namespace boost

#endif  // SRC_SYSTEMS_BASE_ANM_GRAPHICS_OBJECT_DATA_H_
