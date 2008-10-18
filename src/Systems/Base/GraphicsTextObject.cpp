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
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
//
// -----------------------------------------------------------------------

#include "Precompiled.hpp"

// -----------------------------------------------------------------------

#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/serialization/scoped_ptr.hpp>
#include <boost/serialization/export.hpp>

#include "GraphicsTextObject.hpp"

#include "MachineBase/RLMachine.hpp"
#include "Systems/Base/System.hpp"
#include "Systems/Base/TextSystem.hpp"
#include "Systems/Base/Surface.hpp"
#include "Systems/Base/GraphicsObject.hpp"
#include "Utilities.h"

#include <iostream>
using std::endl;

// -----------------------------------------------------------------------

GraphicsTextObject::GraphicsTextObject(RLMachine& machine)
  : machine_(machine)
{
}

// -----------------------------------------------------------------------

GraphicsTextObject::~GraphicsTextObject()
{}

// -----------------------------------------------------------------------

void GraphicsTextObject::updateSurface(const GraphicsObject& rp)
{
  cached_utf8_str_ = rp.textText();
  surface_ = machine_.system().text().renderText(
    machine_, cached_utf8_str_, rp.textSize(), rp.textXSpace(),
    rp.textYSpace(), rp.textColour());
}

// -----------------------------------------------------------------------

bool GraphicsTextObject::needsUpdate(const GraphicsObject& rp)
{
  return !surface_ || rp.textText() != cached_utf8_str_;
}

// -----------------------------------------------------------------------

void GraphicsTextObject::render(RLMachine& machine,
                                const GraphicsObject& rp,
                                std::ostream* tree)
{
  if(needsUpdate(rp))
    updateSurface(rp);

  surface_->renderToScreenAsObject(rp);

  if (tree) {
    *tree << "  Text Object: \"" << rp.textText() << "\"" << endl;
  }
}

// -----------------------------------------------------------------------

int GraphicsTextObject::pixelWidth(const GraphicsObject& rp)
{
  if(needsUpdate(rp))
    updateSurface(rp);

  return int((rp.width() / 100.0f) * surface_->size().width());
}

// -----------------------------------------------------------------------

int GraphicsTextObject::pixelHeight(const GraphicsObject& rp)
{
  if(needsUpdate(rp))
    updateSurface(rp);

  return int((rp.height() / 100.0f) * surface_->size().height());
}

// -----------------------------------------------------------------------

GraphicsObjectData* GraphicsTextObject::clone() const
{
  return new GraphicsTextObject(*this);
}

// -----------------------------------------------------------------------

template<class Archive>
void GraphicsTextObject::load(Archive& ar, unsigned int version)
{
  ar & boost::serialization::base_object<GraphicsObjectData>(*this);

  cached_utf8_str_ = "";
  surface_.reset();
}

// -----------------------------------------------------------------------

template<class Archive>
void GraphicsTextObject::save(Archive& ar, unsigned int version) const
{
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
