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

#ifndef SRC_SYSTEMS_BASE_PARENT_GRAPHICS_OBJECT_DATA_H_
#define SRC_SYSTEMS_BASE_PARENT_GRAPHICS_OBJECT_DATA_H_

#include <boost/serialization/access.hpp>

#include <iosfwd>

#include "systems/base/graphics_object_data.h"
#include "utilities/lazy_array.h"

class GraphicsObject;

// A GraphicsObjectData implementation which owns a full set of graphics
// objects which inherit some(?) of its parent properties.
class ParentGraphicsObjectData : public GraphicsObjectData {
 public:
  explicit ParentGraphicsObjectData(int size);
  virtual ~ParentGraphicsObjectData();

  GraphicsObject& GetObject(int obj_number);
  void SetObject(int obj_number, GraphicsObject& object);

  LazyArray<GraphicsObject>& objects();

  virtual void Render(const GraphicsObject& go,
                      const GraphicsObject* parent,
                      std::ostream* tree) override;
  virtual int PixelWidth(const GraphicsObject& rendering_properties) override;
  virtual int PixelHeight(const GraphicsObject& rendering_properties) override;
  virtual GraphicsObjectData* Clone() const override;
  virtual void Execute(RLMachine& machine) override;
  virtual bool IsAnimation() const override;
  virtual void PlaySet(int set) override;

  virtual bool IsParentLayer() const override { return true; }

 protected:
  virtual std::shared_ptr<const Surface> CurrentSurface(
      const GraphicsObject& rp) override;
  virtual void ObjectInfo(std::ostream& tree) override;

 private:
  ParentGraphicsObjectData();

  LazyArray<GraphicsObject> objects_;

  friend class boost::serialization::access;
  template <class Archive>
  void serialize(Archive& ar, const unsigned int file_version);
};  // class ParentGraphicsObjectData

#endif  // SRC_SYSTEMS_BASE_PARENT_GRAPHICS_OBJECT_DATA_H_
