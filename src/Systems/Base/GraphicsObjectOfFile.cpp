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

#include "Systems/Base/GraphicsSystem.hpp"
#include "Systems/Base/GraphicsObject.hpp"
#include "Systems/Base/System.hpp"
#include "Systems/Base/Surface.hpp"
#include "Systems/Base/GraphicsObjectOfFile.hpp"
#include "Systems/Base/EventSystem.hpp"
#include "MachineBase/RLMachine.hpp"
#include "MachineBase/Serialization.hpp"
#include <boost/shared_ptr.hpp>
#include <string>
#include <iostream>
#include "Utilities.h"

using namespace std;
namespace fs = boost::filesystem;

// -----------------------------------------------------------------------

GraphicsObjectOfFile::GraphicsObjectOfFile()
  : filename_(""),
    frame_time_(0),
    current_frame_(0),
    time_at_last_frame_change_(0)
{
}

// -----------------------------------------------------------------------

GraphicsObjectOfFile::GraphicsObjectOfFile(
  const GraphicsObjectOfFile& obj)
  : GraphicsObjectData(obj),
    filename_(obj.filename_),
    surface_(obj.surface_),
    frame_time_(obj.frame_time_),
    current_frame_(obj.current_frame_),
    time_at_last_frame_change_(obj.time_at_last_frame_change_)
{}

// -----------------------------------------------------------------------

GraphicsObjectOfFile::GraphicsObjectOfFile(
  RLMachine& machine, const std::string& filename)
  : filename_(filename),
    frame_time_(0),
    current_frame_(0),
    time_at_last_frame_change_(0)
{
  loadFile(machine);
}

// -----------------------------------------------------------------------

void GraphicsObjectOfFile::loadFile(RLMachine& machine)
{
  fs::path full_path = findFile(machine, filename_);
  surface_ = machine.system().graphics().loadSurfaceFromFile(full_path);
}

// -----------------------------------------------------------------------

void GraphicsObjectOfFile::render(RLMachine& machine, const GraphicsObject& rp)
{
  if(currentlyPlaying())
  {
    const Surface::GrpRect& rect = surface_->getPattern(current_frame_);

    // POINT
    GraphicsObjectOverride override_data;
    override_data.setOverrideSource(rect.rect.x(), rect.rect.y(), rect.rect.x2(),
                                   rect.rect.y2());

    surface_->renderToScreenAsObject(rp, override_data);
  }
  else
    surface_->renderToScreenAsObject(rp);
}

// -----------------------------------------------------------------------

int GraphicsObjectOfFile::pixelWidth(RLMachine& machine, const GraphicsObject& rp)
{
  const Surface::GrpRect& rect = surface_->getPattern(rp.pattNo());
  int width = rect.rect.width();
  return int((rp.width() / 100.0f) * width);
}

// -----------------------------------------------------------------------

int GraphicsObjectOfFile::pixelHeight(RLMachine& machine, const GraphicsObject& rp)
{
  const Surface::GrpRect& rect = surface_->getPattern(rp.pattNo());
  int height = rect.rect.height();
  return int((rp.height() / 100.0f) * height);
}

// -----------------------------------------------------------------------

GraphicsObjectData* GraphicsObjectOfFile::clone() const
{
  return new GraphicsObjectOfFile(*this);
}

// -----------------------------------------------------------------------

void GraphicsObjectOfFile::execute(RLMachine& machine)
{
  if(currentlyPlaying())
  {
    unsigned int current_time = machine.system().event().getTicks();
    unsigned int time_since_last_frame_change =
      current_time - time_at_last_frame_change_;

    while(time_since_last_frame_change > frame_time_)
    {
      current_frame_++;
      if(current_frame_ == surface_->numPatterns())
      {
        current_frame_--;
        endAnimation();
      }

      time_at_last_frame_change_ += frame_time_;
      time_since_last_frame_change = current_time - time_at_last_frame_change_;
      machine.system().graphics().markScreenAsDirty(GUT_DISPLAY_OBJ);
    }
  }
}

// -----------------------------------------------------------------------

bool GraphicsObjectOfFile::isAnimation() const
{
  return surface_->numPatterns();
}

// -----------------------------------------------------------------------

void GraphicsObjectOfFile::loopAnimation()
{
  current_frame_ = 0;
}

// -----------------------------------------------------------------------

void GraphicsObjectOfFile::playSet(RLMachine& machine, int frame_time)
{
  setCurrentlyPlaying(true);
  frame_time_ = frame_time;
  current_frame_ = 0;

  if (frame_time_ == 0) {
    cerr << "WARNING: GraphicsObjectOfFile::playSet(0) is invalid;"
         << " this is probably going to cause a graphical glitch..."
         << endl;
    frame_time_ = 10;
  }

  time_at_last_frame_change_ = machine.system().event().getTicks();
  machine.system().graphics().markScreenAsDirty(GUT_DISPLAY_OBJ);
}

// -----------------------------------------------------------------------

template<class Archive>
void GraphicsObjectOfFile::load(Archive& ar, unsigned int version)
{
  ar & boost::serialization::base_object<GraphicsObjectData>(*this)
    & filename_ & frame_time_ & current_frame_ & time_at_last_frame_change_;

  loadFile(*Serialization::g_current_machine);
}

// -----------------------------------------------------------------------

template<class Archive>
void GraphicsObjectOfFile::save(Archive& ar, unsigned int version) const
{
  ar & boost::serialization::base_object<GraphicsObjectData>(*this)
    & filename_ & frame_time_ & current_frame_ & time_at_last_frame_change_;
}

// -----------------------------------------------------------------------

BOOST_CLASS_EXPORT(GraphicsObjectOfFile);

// -----------------------------------------------------------------------

// Explicit instantiations for text archives (since we hide the
// implementation)

template void GraphicsObjectOfFile::save<boost::archive::text_oarchive>(
  boost::archive::text_oarchive & ar, unsigned int version) const;

template void GraphicsObjectOfFile::load<boost::archive::text_iarchive>(
  boost::archive::text_iarchive & ar, unsigned int version);
