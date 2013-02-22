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

#ifndef SRC_SYSTEMS_BASE_COLOURFILTEROBJECTDATA_HPP_
#define SRC_SYSTEMS_BASE_COLOURFILTEROBJECTDATA_HPP_

#include <boost/serialization/access.hpp>
#include <boost/shared_ptr.hpp>

#include "Systems/Base/GraphicsObjectData.hpp"
#include "Systems/Base/Rect.hpp"
#include "MachineBase/RLMachine.hpp"
#include "MachineBase/Serialization.hpp"

class ColourFilter;
class GraphicsObject;
class GraphicsSystem;

class ColourFilterObjectData : public GraphicsObjectData {
 public:
  ColourFilterObjectData(GraphicsSystem& system, const Rect& screen_rect);
  virtual ~ColourFilterObjectData();

  // load_construct_data helper. Wish I could make this private.
  ColourFilterObjectData(System& system);

  void setRect(const Rect& screen_rect) { screen_rect_ = screen_rect; }

  // Overriden from GraphicsObjectData:
  virtual void render(const GraphicsObject& go,
                      const GraphicsObject* parent,
                      std::ostream* tree);
  virtual int pixelWidth(const GraphicsObject& rendering_properties);
  virtual int pixelHeight(const GraphicsObject& rendering_properties);
  virtual GraphicsObjectData* clone() const;
  virtual void execute(RLMachine& machine);
  virtual bool isAnimation() const;
  virtual void playSet(int set);

 protected:
  virtual boost::shared_ptr<const Surface> currentSurface(
      const GraphicsObject& rp);
  virtual void objectInfo(std::ostream& tree);

 private:
  GraphicsSystem& graphics_system_;

  Rect screen_rect_;

  boost::scoped_ptr<ColourFilter> colour_filer_;

  friend class boost::serialization::access;
  template<class Archive>
  void serialize(Archive & ar, const unsigned int file_version);
};

// We need help creating ColourFilterObjectData s since they don't have a
// default constructor:
namespace boost { namespace serialization {
template<class Archive>
inline void load_construct_data(
  Archive & ar, ColourFilterObjectData* t, const unsigned int file_version) {
  ::new(t)ColourFilterObjectData(Serialization::g_current_machine->system());
}
}}  // namespace boost::serialization

#endif  // SRC_SYSTEMS_BASE_COLOURFILTEROBJECTDATA_HPP_
