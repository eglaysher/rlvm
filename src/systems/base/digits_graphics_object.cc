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

#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/serialization/export.hpp>

#include "systems/base/digits_graphics_object.h"

#include <algorithm>
#include <iostream>

#include "systems/base/colour.h"
#include "systems/base/system.h"
#include "systems/base/graphics_object.h"
#include "systems/base/graphics_system.h"
#include "systems/base/surface.h"

DigitsGraphicsObject::DigitsGraphicsObject(System& system)
    : system_(system), value_(0) {}

DigitsGraphicsObject::DigitsGraphicsObject(System& system,
                                           const std::string& font)
    : system_(system),
      value_(0),
      font_name_(font),
      font_(system.graphics().GetSurfaceNamed(font)) {}

DigitsGraphicsObject::~DigitsGraphicsObject() {}

int DigitsGraphicsObject::PixelWidth(const GraphicsObject& rp) {
  if (NeedsUpdate(rp))
    UpdateSurface(rp);

  return int(rp.GetWidthScaleFactor() * surface_->GetSize().width());
}

int DigitsGraphicsObject::PixelHeight(const GraphicsObject& rp) {
  if (NeedsUpdate(rp))
    UpdateSurface(rp);

  return int(rp.GetHeightScaleFactor() * surface_->GetSize().height());
}

GraphicsObjectData* DigitsGraphicsObject::Clone() const {
  return new DigitsGraphicsObject(*this);
}

void DigitsGraphicsObject::Execute(RLMachine& machine) {}

std::shared_ptr<const Surface> DigitsGraphicsObject::CurrentSurface(
    const GraphicsObject& go) {
  if (NeedsUpdate(go))
    UpdateSurface(go);

  return surface_;
}

void DigitsGraphicsObject::ObjectInfo(std::ostream& tree) {
  tree << "  Digits: \"" << value_ << "\"" << std::endl;
}

void DigitsGraphicsObject::UpdateSurface(const GraphicsObject& rp) {
  value_ = rp.GetDigitValue();

  // Calculate the size our canvas will have to be.
  int digit_pixel_width = rp.GetDigitSpace()
                              ? rp.GetDigitSpace()
                              : font_->GetPattern(0).rect.size().width();
  int num_chars = 0;
  for (int a = value_; a > 0; a = a / 10, num_chars++) {}
  num_chars = std::max(num_chars, rp.GetDigitDigits());

  int num_extra = 0;
  if (value_ < 0 || rp.GetDigitSign())
    num_extra++;

  int total_pixel_width = (num_chars + num_extra) * digit_pixel_width;

  surface_ = system_.graphics().BuildSurface(
      Size(total_pixel_width, font_->GetPattern(0).rect.size().height()));
  surface_->Fill(RGBAColour::Clear());

  // We draw glyphs onto the canvas from right to left so we can use the
  // obvious div/mod method to get the current digit to display.
  int printed_number = std::abs(value_);
  int x_offset = total_pixel_width - digit_pixel_width;
  int i = printed_number;
  int printed = 0;
  do {
    int digit = i % 10;
    const Surface::GrpRect& grp = font_->GetPattern(digit);

    font_->BlitToSurface(
        *surface_, grp.rect, Rect(x_offset, 0, grp.rect.size()), 255, false);

    i = i / 10;
    printed++;
    x_offset -= digit_pixel_width;
  } while (i > 0);

  const Surface::GrpRect& zero_grp = font_->GetPattern(0);
  bool print_zeros = rp.GetDigitZero();
  while (printed < num_chars) {
    if (print_zeros) {
      font_->BlitToSurface(*surface_,
                           zero_grp.rect,
                           Rect(x_offset, 0, zero_grp.rect.size()),
                           255,
                           false);
    }
    printed++;
    x_offset -= digit_pixel_width;
  }

  if (value_ < 0 || rp.GetDigitSign()) {
    std::cerr << "We don't support negative numbers in objOfDigits() yet."
              << std::endl;
  }
}

bool DigitsGraphicsObject::NeedsUpdate(const GraphicsObject& rp) {
  return !surface_ || rp.GetDigitValue() != value_;
}

// -----------------------------------------------------------------------

template <class Archive>
void DigitsGraphicsObject::load(Archive& ar, unsigned int version) {
  ar& boost::serialization::base_object<GraphicsObjectData>(*this) & font_name_;

  value_ = 0;
  surface_.reset();
  font_ = system_.graphics().GetSurfaceNamed(font_name_);
}

template <class Archive>
void DigitsGraphicsObject::save(Archive& ar, unsigned int version) const {
  ar& boost::serialization::base_object<GraphicsObjectData>(*this) & font_name_;
}

// -----------------------------------------------------------------------

BOOST_CLASS_EXPORT(DigitsGraphicsObject);

// -----------------------------------------------------------------------

// Explicit instantiations for text archives (since we hide the
// implementation)

template void DigitsGraphicsObject::save<boost::archive::text_oarchive>(
    boost::archive::text_oarchive& ar,
    unsigned int version) const;

template void DigitsGraphicsObject::load<boost::archive::text_iarchive>(
    boost::archive::text_iarchive& ar,
    unsigned int version);
