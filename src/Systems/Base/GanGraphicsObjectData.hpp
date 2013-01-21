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

#ifndef SRC_SYSTEMS_BASE_GANGRAPHICSOBJECTDATA_HPP_
#define SRC_SYSTEMS_BASE_GANGRAPHICSOBJECTDATA_HPP_

#include "Systems/Base/GraphicsObjectData.hpp"

#include "MachineBase/RLMachine.hpp"
#include "MachineBase/Serialization.hpp"

#include <boost/scoped_array.hpp>
#include <boost/serialization/access.hpp>
#include <boost/serialization/split_member.hpp>
#include <boost/shared_ptr.hpp>
#include <iosfwd>
#include <string>
#include <vector>

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
  GanGraphicsObjectData(System& system, const std::string& ganfile,
                        const std::string& imgfile);
  ~GanGraphicsObjectData();

  void load();

  virtual int pixelWidth(const GraphicsObject& rendering_properties);
  virtual int pixelHeight(const GraphicsObject& rendering_properties);

  virtual GraphicsObjectData* clone() const;
  virtual void execute(RLMachine& machine);

  virtual bool isAnimation() const { return true; }
  virtual void playSet(int set);

 protected:
  /// Resets to the first frame.
  virtual void loopAnimation();

  virtual boost::shared_ptr<const Surface> currentSurface(
      const GraphicsObject& go);
  virtual Rect srcRect(const GraphicsObject& go);
  virtual Point dstOrigin(const GraphicsObject& go);
  virtual int getRenderingAlpha(const GraphicsObject& go,
                                const GraphicsObject* parent);
  virtual void objectInfo(std::ostream& tree);

 private:
  struct Frame {
    int pattern;
    int x;
    int y;
    int time;
    int alpha;
    int other;  // No idea what this is.
  };

  typedef std::vector< std::vector<Frame> > AnimationSets;

  void testFileMagic(const std::string& file_name,
                     boost::scoped_array<char>& gan_data, int file_size);
  void readData(
    const std::string& file_name,
    boost::scoped_array<char>& gan_data, int file_size);
  Frame readSetFrame(const std::string& filename, const char*& data);

  /// Throws an error on bad GAN files.
  void throwBadFormat(const std::string& filename, const std::string& error);

  System& system_;

  AnimationSets animation_sets;

  std::string gan_filename_;
  std::string img_filename_;

  int current_set_;
  int current_frame_;
  int time_at_last_frame_change_;

  /// The image the above coordinates map into.
  boost::shared_ptr<const Surface> image_;

  friend class boost::serialization::access;

  // boost::serialization forward declaration
  template<class Archive>
  void save(Archive & ar, const unsigned int file_version) const;

  // boost::serialization forward declaration
  template<class Archive>
  void load(Archive& ar, const unsigned int file_version);

  BOOST_SERIALIZATION_SPLIT_MEMBER()
};

// We need help creating GanGraphicsObjectData s since they don't have a
// default constructor:
namespace boost { namespace serialization {
template<class Archive>
inline void load_construct_data(
  Archive & ar, GanGraphicsObjectData* t, const unsigned int file_version) {
  ::new(t)GanGraphicsObjectData(Serialization::g_current_machine->system());
}
}}  // namespace boost::serialization


#endif  // SRC_SYSTEMS_BASE_GANGRAPHICSOBJECTDATA_HPP_
