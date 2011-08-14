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

#ifndef SRC_SYSTEMS_BASE_PARENTGRAPHICSOBJECTDATA_HPP_
#define SRC_SYSTEMS_BASE_PARENTGRAPHICSOBJECTDATA_HPP_

#include <boost/serialization/access.hpp>

#include "Systems/Base/GraphicsObjectData.hpp"
#include "Utilities/LazyArray.hpp"

#include <iosfwd>

class GraphicsObject;

// A GraphicsObjectData implementation which owns a full set of graphics
// objects which inherit some(?) of its parent properties.
class ParentGraphicsObjectData : public GraphicsObjectData {
 public:
  ParentGraphicsObjectData(int size);
  virtual ~ParentGraphicsObjectData();

  GraphicsObject& getObject(int obj_number);
  void setObject(int obj_number, GraphicsObject& object);

  virtual void render(const GraphicsObject& go, std::ostream* tree);
  virtual int pixelWidth(const GraphicsObject& rendering_properties);
  virtual int pixelHeight(const GraphicsObject& rendering_properties);
  virtual GraphicsObjectData* clone() const;
  virtual void execute();
  virtual bool isAnimation() const;
  virtual void playSet(int set);

  virtual bool isParentLayer() const { return true; }

 protected:
  virtual boost::shared_ptr<const Surface> currentSurface(
      const GraphicsObject& rp);
  virtual void objectInfo(std::ostream& tree);

 private:
  ParentGraphicsObjectData();

  // 256 child objects.
  LazyArray<GraphicsObject> objects_;

  friend class boost::serialization::access;
  template<class Archive>
  void serialize(Archive & ar, const unsigned int file_version);
};  // class ParentGraphicsObjectData

#endif  // SRC_SYSTEMS_BASE_PARENTGRAPHICSOBJECTDATA_HPP_
