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

#ifndef SRC_SYSTEMS_BASE_DIGITS_GRAPHICS_OBJECT_H_
#define SRC_SYSTEMS_BASE_DIGITS_GRAPHICS_OBJECT_H_

#include <boost/serialization/split_member.hpp>

#include <memory>
#include <string>

#include "machine/rlmachine.h"
#include "machine/serialization.h"
#include "systems/base/graphics_object_data.h"

class GraphicsObject;
class Surface;
class System;

class DigitsGraphicsObject : public GraphicsObjectData {
 public:
  explicit DigitsGraphicsObject(System& system);
  DigitsGraphicsObject(System& system, const std::string& font);
  virtual ~DigitsGraphicsObject();

  virtual int PixelWidth(const GraphicsObject& rendering_properties) override;
  virtual int PixelHeight(const GraphicsObject& rendering_properties) override;

  virtual GraphicsObjectData* Clone() const override;
  virtual void Execute(RLMachine& machine) override;

 protected:
  virtual std::shared_ptr<const Surface> CurrentSurface(
      const GraphicsObject& go) override;
  virtual void ObjectInfo(std::ostream& tree) override;

 private:
  void UpdateSurface(const GraphicsObject& rp);
  bool NeedsUpdate(const GraphicsObject& rendering_properties);

  System& system_;

  // The actual numerical value to display.
  int value_;

  // The source font.
  std::string font_name_;
  std::shared_ptr<const Surface> font_;

  // The current composited surface.
  std::shared_ptr<Surface> surface_;

  // boost::serialization support
  friend class boost::serialization::access;

  template <class Archive>
  void save(Archive& ar, const unsigned int file_version) const;

  template <class Archive>
  void load(Archive& ar, const unsigned int file_version);

  BOOST_SERIALIZATION_SPLIT_MEMBER()
};

// We need help creating DigitsGraphicsObject s since they don't have a default
// constructor:
namespace boost {
namespace serialization {
template <class Archive>
inline void load_construct_data(Archive& ar,
                                DigitsGraphicsObject* t,
                                const unsigned int file_version) {
  ::new (t) DigitsGraphicsObject(Serialization::g_current_machine->system());
}
}
}

#endif  // SRC_SYSTEMS_BASE_DIGITS_GRAPHICS_OBJECT_H_
