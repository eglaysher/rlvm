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

#ifndef SRC_SYSTEMS_BASE_GAN_GRAPHICS_OBJECT_DATA_H_
#define SRC_SYSTEMS_BASE_GAN_GRAPHICS_OBJECT_DATA_H_

#include <boost/serialization/access.hpp>
#include <boost/serialization/split_member.hpp>

#include <iosfwd>
#include <memory>
#include <string>
#include <vector>

#include "machine/rlmachine.h"
#include "machine/serialization.h"
#include "systems/base/graphics_object_data.h"

class Surface;
class System;
class RLMachine;
class GraphicsObject;

// -----------------------------------------------------------------------

// In-memory representation of a GAN file. Responsible for reading in,
// storing, and rendering GAN data as a GraphicsObjectData.
class GanGraphicsObjectData : public GraphicsObjectData {
 public:
  explicit GanGraphicsObjectData(System& system);
  GanGraphicsObjectData(System& system,
                        const std::string& ganfile,
                        const std::string& imgfile);
  virtual ~GanGraphicsObjectData();

  void LoadGANData();

  virtual int PixelWidth(const GraphicsObject& rendering_properties) override;
  virtual int PixelHeight(const GraphicsObject& rendering_properties) override;

  virtual GraphicsObjectData* Clone() const override;
  virtual void Execute(RLMachine& machine) override;

  virtual bool IsAnimation() const override { return true; }
  virtual void PlaySet(int set) override;

 protected:
  // Resets to the first frame.
  virtual void LoopAnimation() override;

  virtual std::shared_ptr<const Surface> CurrentSurface(
      const GraphicsObject& go) override;
  virtual Rect SrcRect(const GraphicsObject& go) override;
  virtual Point DstOrigin(const GraphicsObject& go) override;
  virtual int GetRenderingAlpha(const GraphicsObject& go,
                                const GraphicsObject* parent) override;
  virtual void ObjectInfo(std::ostream& tree) override;

 private:
  struct Frame {
    int pattern;
    int x;
    int y;
    int time;
    int alpha;
    int other;  // No idea what this is.
  };

  typedef std::vector<std::vector<Frame>> AnimationSets;

  void TestFileMagic(const std::string& file_name,
                     std::unique_ptr<char[]>& gan_data,
                     int file_size);
  void ReadData(const std::string& file_name,
                std::unique_ptr<char[]>& gan_data,
                int file_size);
  Frame ReadSetFrame(const std::string& filename, const char*& data);

  // Throws an error on bad GAN files.
  void ThrowBadFormat(const std::string& filename, const std::string& error);

  System& system_;

  AnimationSets animation_sets;

  std::string gan_filename_;
  std::string img_filename_;

  int current_set_;
  int current_frame_;
  int time_at_last_frame_change_;

  // The image the above coordinates map into.
  std::shared_ptr<const Surface> image_;

  friend class boost::serialization::access;

  // boost::serialization forward declaration
  template <class Archive>
  void save(Archive& ar, const unsigned int file_version) const;

  // boost::serialization forward declaration
  template <class Archive>
  void load(Archive& ar, const unsigned int file_version);

  BOOST_SERIALIZATION_SPLIT_MEMBER()
};

// We need help creating GanGraphicsObjectData s since they don't have a
// default constructor:
namespace boost {
namespace serialization {
template <class Archive>
inline void load_construct_data(Archive& ar,
                                GanGraphicsObjectData* t,
                                const unsigned int file_version) {
  ::new (t) GanGraphicsObjectData(Serialization::g_current_machine->system());
}
}  // namespace serialization
}  // namespace boost

#endif  // SRC_SYSTEMS_BASE_GAN_GRAPHICS_OBJECT_DATA_H_
