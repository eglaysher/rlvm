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

#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/serialization/scoped_ptr.hpp>
#include <boost/serialization/export.hpp>

#include "Systems/Base/GraphicsTextObject.hpp"

#include <ostream>
#include <vector>

#include "Systems/Base/GraphicsObject.hpp"
#include "Systems/Base/Surface.hpp"
#include "Systems/Base/System.hpp"
#include "Systems/Base/TextSystem.hpp"
#include "libReallive/gameexe.h"

// -----------------------------------------------------------------------

GraphicsTextObject::GraphicsTextObject(System& system)
    : system_(system) {
}

// -----------------------------------------------------------------------

GraphicsTextObject::~GraphicsTextObject() {
}

// -----------------------------------------------------------------------

void GraphicsTextObject::updateSurface(const GraphicsObject& rp) {
  cached_utf8_str_ = rp.textText();

  // Get the correct colour
  Gameexe& gexe = system_.gameexe();
  std::vector<int> vec = gexe("COLOR_TABLE", rp.textColour());
  RGBColour colour(vec.at(0), vec.at(1), vec.at(2));

  RGBColour* shadow = NULL;
  RGBColour shadow_impl;
  if (rp.textShadowColour() != -1) {
    vec = gexe("COLOR_TABLE", rp.textShadowColour());
    shadow_impl = RGBColour(vec.at(0), vec.at(1), vec.at(2));
    shadow = &shadow_impl;
  }

  surface_ = system_.text().renderText(
      cached_utf8_str_, rp.textSize(), rp.textXSpace(),
      rp.textYSpace(), colour, shadow);
}

// -----------------------------------------------------------------------

bool GraphicsTextObject::needsUpdate(const GraphicsObject& rp) {
  return !surface_ || rp.textText() != cached_utf8_str_;
}

// -----------------------------------------------------------------------

boost::shared_ptr<const Surface> GraphicsTextObject::currentSurface(
  const GraphicsObject& go) {
  if (needsUpdate(go))
    updateSurface(go);

  return surface_;
}

// -----------------------------------------------------------------------

void GraphicsTextObject::objectInfo(std::ostream& tree) {
  tree << "  Text: \"" << cached_utf8_str_ << "\"" << std::endl;
}

// -----------------------------------------------------------------------

int GraphicsTextObject::pixelWidth(const GraphicsObject& rp) {
  if (needsUpdate(rp))
    updateSurface(rp);

  return int(rp.getWidthScaleFactor() * surface_->size().width());
}

// -----------------------------------------------------------------------

int GraphicsTextObject::pixelHeight(const GraphicsObject& rp) {
  if (needsUpdate(rp))
    updateSurface(rp);

  return int(rp.getHeightScaleFactor() * surface_->size().height());
}

// -----------------------------------------------------------------------

GraphicsObjectData* GraphicsTextObject::clone() const {
  return new GraphicsTextObject(*this);
}

// -----------------------------------------------------------------------

void GraphicsTextObject::execute(RLMachine& machine) {
}

// -----------------------------------------------------------------------

template<class Archive>
void GraphicsTextObject::load(Archive& ar, unsigned int version) {
  ar & boost::serialization::base_object<GraphicsObjectData>(*this);

  cached_utf8_str_ = "";
  surface_.reset();
}

// -----------------------------------------------------------------------

template<class Archive>
void GraphicsTextObject::save(Archive& ar, unsigned int version) const {
  ar & boost::serialization::base_object<GraphicsObjectData>(*this);
}

// -----------------------------------------------------------------------

BOOST_CLASS_EXPORT(GraphicsTextObject);

// -----------------------------------------------------------------------

// Explicit instantiations for text archives (since we hide the
// implementation)

template void GraphicsTextObject::save<boost::archive::text_oarchive>(
  boost::archive::text_oarchive & ar, unsigned int version) const;

template void GraphicsTextObject::load<boost::archive::text_iarchive>(
  boost::archive::text_iarchive & ar, unsigned int version);
