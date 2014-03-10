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

#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/serialization/scoped_ptr.hpp>
#include <boost/serialization/export.hpp>

#include "systems/base/drift_graphics_object.h"

#include <string>
#include <vector>

#include "systems/base/event_system.h"
#include "systems/base/graphics_object.h"
#include "systems/base/graphics_system.h"
#include "systems/base/rect.h"
#include "systems/base/surface.h"
#include "systems/base/system.h"
#include "Utilities/Graphics.hpp"

namespace {

double scaleAmplitude(int amplitude) {
  // So the amplitude of the curve in RealLive is weird. Some value close to
  // 100 means one width of the screen, 1 is a vary large amount that I can't
  // reliably measure, and values greater than 100 are increasingly smaller.  I
  // can't reliably measure this because I suspect that RL deliberately
  // introduces some randomness here. Oh well. There's probably some curve that
  // fits this, but whatever. I think this might be a valid approximation:
  int x = amplitude / 100;
  return 1 / static_cast<double>(x);
}

}  // namespace

DriftGraphicsObject::DriftGraphicsObject(System& system)
    : system_(system), filename_(), surface_(), last_rendered_time_(0) {}

DriftGraphicsObject::DriftGraphicsObject(const DriftGraphicsObject& obj)
    : GraphicsObjectData(obj),
      system_(obj.system_),
      filename_(obj.filename_),
      surface_(obj.surface_),
      last_rendered_time_(0) {}

DriftGraphicsObject::DriftGraphicsObject(System& system,
                                         const std::string& filename)
    : system_(system), filename_(filename), surface_(), last_rendered_time_(0) {
  loadFile();
}

DriftGraphicsObject::~DriftGraphicsObject() {}

void DriftGraphicsObject::render(const GraphicsObject& go,
                                 const GraphicsObject* parent,
                                 std::ostream* tree) {
  boost::shared_ptr<const Surface> surface = currentSurface(go);
  if (surface) {
    int current_time = system_.event().getTicks();
    last_rendered_time_ = current_time;

    size_t count = go.driftParticleCount();
    bool use_animation = go.driftUseAnimation();
    int start_pattern = go.driftStartPattern();
    int end_pattern = go.driftEndPattern();
    int animation_time = go.driftAnimationTime();
    int yspeed = go.driftYSpeed();
    int period = go.driftPeriod();
    int amplitude = go.driftAmplitude();
    int use_drift = go.driftUseDrift();
    int drift_speed = go.driftDriftSpeed();

    Rect bounding_box = go.driftArea();
    if (bounding_box.x() == -1) {
      bounding_box = system_.graphics().screenRect();
    }

    double scaled_amplitude =
        bounding_box.size().width() * scaleAmplitude(amplitude);

    // Grab the drift object
    if (particles_.size() < count) {
      Particle p;
      p.x = rand() % bounding_box.size().width();   // NOLINT
      p.y = rand() % bounding_box.size().height();  // NOLINT
      p.alpha = 255;
      p.start_time = current_time;

      particles_.push_back(p);
    }

    // Now that we have all the particles, update state and render each
    // particle.
    for (std::vector<Particle>::iterator it = particles_.begin();
         it != particles_.end();
         ++it) {
      int pattern = start_pattern;
      if (use_animation && end_pattern > start_pattern) {
        int number_of_patterns = end_pattern - start_pattern + 1;
        int frame_time = animation_time / number_of_patterns;
        int frame_number =
            ((current_time - it->start_time) / frame_time) % number_of_patterns;
        pattern = start_pattern + frame_number;
      }
      Rect src = surface->getPattern(pattern).rect;

      int dest_x = it->x;
      int dest_y = it->y;

      // Add the base yspeed.
      dest_y += bounding_box.size().height() *
                (static_cast<double>((current_time - it->start_time) % yspeed) /
                 static_cast<double>(yspeed));

      // Add the sine wave that defines how the particle moves back and forth.
      if (period != 0 && amplitude != 0) {
        double result =
            sin((static_cast<double>(current_time - it->start_time) / period) *
                (2 * 3.14));
        dest_x += scaled_amplitude * result;
      }

      // Add the left drift if we have this bit set.
      if (use_drift) {
        dest_x -= bounding_box.size().width() *
                  (static_cast<double>((current_time - it->start_time) %
                                       drift_speed) /
                   static_cast<double>(drift_speed));
      }

      if (dest_x < 0)
        dest_x += bounding_box.size().width();
      else
        dest_x %= bounding_box.size().width();

      if (dest_y < 0)
        dest_y += bounding_box.size().height();
      else
        dest_y %= bounding_box.size().height();
      Rect dest(bounding_box.origin() + Size(dest_x, dest_y), src.size());

      if (go.hasClip())
        ClipDestination(go.clipRect(), src, dest);

      surface->renderToScreen(src, dest, it->alpha);
    }
  }
}

int DriftGraphicsObject::pixelWidth(
    const GraphicsObject& rendering_properties) {
  return rendering_properties.driftArea().width();
}

int DriftGraphicsObject::pixelHeight(
    const GraphicsObject& rendering_properties) {
  return rendering_properties.driftArea().height();
}

GraphicsObjectData* DriftGraphicsObject::clone() const {
  return new DriftGraphicsObject(*this);
}

void DriftGraphicsObject::execute(RLMachine& machine) {
  // We could theoretically redraw every time around the game loop, so
  // throttle to once every 100ms.
  int current_time = system_.event().getTicks();
  if (current_time - last_rendered_time_ > 10) {
    system_.graphics().markScreenAsDirty(GUT_DISPLAY_OBJ);
  }
}

boost::shared_ptr<const Surface> DriftGraphicsObject::currentSurface(
    const GraphicsObject& rp) {
  return surface_;
}

void DriftGraphicsObject::objectInfo(std::ostream& tree) {
  tree << "  Drift image: " << filename_ << std::endl;
}

void DriftGraphicsObject::loadFile() {
  surface_ = system_.graphics().getSurfaceNamed(filename_);
  surface_->EnsureUploaded();
}

template <class Archive>
void DriftGraphicsObject::load(Archive& ar, unsigned int version) {
  ar& boost::serialization::base_object<GraphicsObjectData>(*this) & filename_;

  loadFile();
}

// -----------------------------------------------------------------------

template <class Archive>
void DriftGraphicsObject::save(Archive& ar, unsigned int version) const {
  ar& boost::serialization::base_object<GraphicsObjectData>(*this) & filename_;
}

// -----------------------------------------------------------------------

BOOST_CLASS_EXPORT(DriftGraphicsObject);

// -----------------------------------------------------------------------

// Explicit instantiations for text archives (since we hide the
// implementation)

template void DriftGraphicsObject::save<boost::archive::text_oarchive>(
    boost::archive::text_oarchive& ar,
    unsigned int version) const;

template void DriftGraphicsObject::load<boost::archive::text_iarchive>(
    boost::archive::text_iarchive& ar,
    unsigned int version);
