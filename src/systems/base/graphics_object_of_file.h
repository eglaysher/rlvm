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

#ifndef SRC_SYSTEMS_BASE_GRAPHICS_OBJECT_OF_FILE_H_
#define SRC_SYSTEMS_BASE_GRAPHICS_OBJECT_OF_FILE_H_

#include <boost/serialization/access.hpp>
#include <boost/serialization/split_member.hpp>

#include <memory>
#include <string>

#include "machine/rlmachine.h"
#include "machine/serialization.h"
#include "systems/base/graphics_object_data.h"

class System;
class Surface;
class RLMachine;

// -----------------------------------------------------------------------

// GraphicsObjectData class that encapsulates a G00 or ANM file.
//
// GraphicsObjectOfFile is used for loading individual bitmaps into an
// object. It has support for normal display, and also
class GraphicsObjectOfFile : public GraphicsObjectData {
 public:
  explicit GraphicsObjectOfFile(System& system);
  GraphicsObjectOfFile(System& system, const std::string& filename);
  virtual ~GraphicsObjectOfFile();

  const std::string& filename() const { return filename_; }

  virtual int PixelWidth(const GraphicsObject& rp) override;
  virtual int PixelHeight(const GraphicsObject& rp) override;

  virtual GraphicsObjectData* Clone() const override;

  virtual void Execute(RLMachine& machine) override;

  virtual bool IsAnimation() const override;
  virtual void PlaySet(int set) override;

 protected:
  virtual void LoopAnimation() override;
  virtual std::shared_ptr<const Surface> CurrentSurface(
      const GraphicsObject& go) override;
  virtual Rect SrcRect(const GraphicsObject& go) override;
  virtual void ObjectInfo(std::ostream& tree) override;

 private:
  // Private constructor for cloning
  GraphicsObjectOfFile(const GraphicsObjectOfFile& obj);

  // Used in serialization system.
  void LoadFile();

  // Our parent system.
  System& system_;

  // The name of the graphics file that was loaded.
  std::string filename_;

  // The encapsulated surface to render
  std::shared_ptr<const Surface> surface_;

  // Number of milliseconds to spend on a single frame in the
  // animation
  unsigned int frame_time_;

  // Current frame displayed (when animating)
  int current_frame_;

  // While currentlyPlaying() is true, this variable is used to store
  // the time when the frame was switched last
  unsigned int time_at_last_frame_change_;

  // boost::serialization support
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
                                GraphicsObjectOfFile* t,
                                const unsigned int file_version) {
  ::new (t) GraphicsObjectOfFile(Serialization::g_current_machine->system());
}
}
}

#endif  // SRC_SYSTEMS_BASE_GRAPHICS_OBJECT_OF_FILE_H_
