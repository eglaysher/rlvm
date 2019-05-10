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

#ifndef SRC_SYSTEMS_BASE_COLOUR_FILTER_OBJECT_DATA_H_
#define SRC_SYSTEMS_BASE_COLOUR_FILTER_OBJECT_DATA_H_

#include <boost/serialization/access.hpp>

#include <memory>

#include "machine/rlmachine.h"
#include "machine/serialization.h"
#include "systems/base/graphics_object_data.h"
#include "systems/base/rect.h"

class ColourFilter;
class GraphicsObject;
class GraphicsSystem;

class ColourFilterObjectData : public GraphicsObjectData {
 public:
  ColourFilterObjectData(GraphicsSystem& system, const Rect& screen_rect);
  virtual ~ColourFilterObjectData();

  // load_construct_data helper. Wish I could make this private.
  explicit ColourFilterObjectData(System& system);

  void set_rect(const Rect& screen_rect) { screen_rect_ = screen_rect; }

  // Returns the colour filter, lazily creating it if necessary.
  ColourFilter* GetColourFilter();

  // Overridden from GraphicsObjectData:
  virtual void Render(const GraphicsObject& go,
                      const GraphicsObject* parent,
                      std::ostream* tree) override;
  virtual int PixelWidth(const GraphicsObject& rendering_properties) override;
  virtual int PixelHeight(const GraphicsObject& rendering_properties) override;
  virtual GraphicsObjectData* Clone() const override;
  virtual void Execute(RLMachine& machine) override;
  virtual bool IsAnimation() const override;
  virtual void PlaySet(int set) override;

 protected:
  virtual std::shared_ptr<const Surface> CurrentSurface(
      const GraphicsObject& rp) override;
  virtual void ObjectInfo(std::ostream& tree) override;

 private:
  GraphicsSystem& graphics_system_;

  Rect screen_rect_;

  std::unique_ptr<ColourFilter> colour_filer_;

  friend class boost::serialization::access;
  template <class Archive>
  void serialize(Archive& ar, const unsigned int file_version);
};

// We need help creating ColourFilterObjectData s since they don't have a
// default constructor:
namespace boost {
namespace serialization {
template <class Archive>
inline void load_construct_data(Archive& ar,
                                ColourFilterObjectData* t,
                                const unsigned int file_version) {
  ::new (t) ColourFilterObjectData(Serialization::g_current_machine->system());
}
}  // namespace serialization
}  // namespace boost

#endif  // SRC_SYSTEMS_BASE_COLOUR_FILTER_OBJECT_DATA_H_
