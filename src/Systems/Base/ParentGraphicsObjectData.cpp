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
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
// -----------------------------------------------------------------------

#include "Systems/Base/ParentGraphicsObjectData.hpp"

#include "Systems/Base/GraphicsObject.hpp"
#include "Utilities/Exception.hpp"

// -----------------------------------------------------------------------
// ParentGraphicsObjectData
// -----------------------------------------------------------------------
ParentGraphicsObjectData::ParentGraphicsObjectData(int size)
    : objects_(size) {
}

// -----------------------------------------------------------------------

ParentGraphicsObjectData::~ParentGraphicsObjectData() {
}

// -----------------------------------------------------------------------

GraphicsObject& ParentGraphicsObjectData::getObject(int obj_number) {
  return objects_[obj_number];
}

// -----------------------------------------------------------------------

void ParentGraphicsObjectData::setObject(int obj_number,
                                         GraphicsObject& object) {
  objects_[obj_number] = object;
}

// -----------------------------------------------------------------------

void ParentGraphicsObjectData::render(const GraphicsObject& go,
                                      std::ostream* tree) {
  AllocatedLazyArrayIterator<GraphicsObject> it = objects_.allocated_begin();
  AllocatedLazyArrayIterator<GraphicsObject> end = objects_.allocated_end();
  for (; it != end; ++it) {
    it->render(it.pos(), tree);
  }
}

// -----------------------------------------------------------------------

int ParentGraphicsObjectData::pixelWidth(
    const GraphicsObject& rendering_properties) {
  throw rlvm::Exception("There is no sane value for this!");
}

// -----------------------------------------------------------------------

int ParentGraphicsObjectData::pixelHeight(
    const GraphicsObject& rendering_properties) {
  throw rlvm::Exception("There is no sane value for this!");
}

// -----------------------------------------------------------------------

GraphicsObjectData* ParentGraphicsObjectData::clone() const {
  int size = objects_.size();
  ParentGraphicsObjectData* cloned = new ParentGraphicsObjectData(size);

  for (int i = 0; i < size; ++i) {
    if (objects_.exists(i)) {
      cloned->objects_[i] = GraphicsObject(objects_[i]);
    }
  }

  return cloned;
}

// -----------------------------------------------------------------------

void ParentGraphicsObjectData::execute() {
  // We pass on execute to all our children
  AllocatedLazyArrayIterator<GraphicsObject> it = objects_.allocated_begin();
  AllocatedLazyArrayIterator<GraphicsObject> end = objects_.allocated_end();
  for (; it != end; ++it) {
    it->execute();
  }
}

// -----------------------------------------------------------------------

bool ParentGraphicsObjectData::isAnimation() const {
  return false;
}

// -----------------------------------------------------------------------

void ParentGraphicsObjectData::playSet(int set) {
  // Deliberately empty.
}

// -----------------------------------------------------------------------

void ParentGraphicsObjectData::objectInfo(std::ostream& tree) {
  tree << "ParentGraphicsObjectData::objectInfo is a TODO";
}
