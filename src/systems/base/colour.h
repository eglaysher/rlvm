// -*- Mode: C++; tab-width:2; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi:tw=80:et:ts=2:sts=2
//
// -----------------------------------------------------------------------
//
// This file is part of RLVM, a RealLive virtual machine clone.
//
// -----------------------------------------------------------------------
//
// Copyright (C) 2008 Elliot Glaysher
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

#ifndef SRC_SYSTEMS_BASE_COLOUR_H_
#define SRC_SYSTEMS_BASE_COLOUR_H_

#include <boost/serialization/access.hpp>
#include <vector>
#include <iosfwd>

// A colour without an alpha channel.
class RGBColour {
 public:
  RGBColour() : r_(0), g_(0), b_(0) {}
  RGBColour(int r, int g, int b) : r_(r), g_(g), b_(b) {}

  // Reads (r,g,b) from (colour[0], colour[1], colour[2]).
  explicit RGBColour(const std::vector<int>& colour);

  int r() const { return r_; }
  int g() const { return g_; }
  int b() const { return b_; }
  float r_float() const { return static_cast<float>(r_) / 255; }
  float g_float() const { return static_cast<float>(g_) / 255; }
  float b_float() const { return static_cast<float>(b_) / 255; }

  void set_red(const int in) { r_ = in; }
  void set_green(const int in) { g_ = in; }
  void set_blue(const int in) { b_ = in; }

  bool operator==(const RGBColour& rhs) const;
  bool operator!=(const RGBColour& rhs) const;

  // Colour constants
  static RGBColour Black() { return RGBColour(0, 0, 0); }
  static RGBColour White() { return RGBColour(255, 255, 255); }

 private:
  int r_, g_, b_;

  // boost::serialization support
  friend class boost::serialization::access;
  template <class Archive>
  void serialize(Archive& ar, unsigned int version) {
    ar& r_& g_& b_;
  }
};  // end of class RGB

// -----------------------------------------------------------------------

// Adds an alpha component to the standard RGB class.
class RGBAColour {
 public:
  RGBAColour() : rgb_(0, 0, 0), alpha_(0) {}
  RGBAColour(int r, int g, int b) : rgb_(r, g, b), alpha_(255) {}
  RGBAColour(int r, int g, int b, int a) : rgb_(r, g, b), alpha_(a) {}
  explicit RGBAColour(const RGBColour& colour) : rgb_(colour), alpha_(255) {}
  RGBAColour(const RGBColour& colour, int a) : rgb_(colour), alpha_(a) {}

  // Reads (r,g,b) from (colour[0], colour[1], colour[2]). Alpha is always 255.
  explicit RGBAColour(const std::vector<int>& colour);

  int r() const { return rgb_.r(); }
  int g() const { return rgb_.g(); }
  int b() const { return rgb_.b(); }
  const RGBColour& rgb() const { return rgb_; }
  float r_float() const { return rgb_.r_float(); }
  float g_float() const { return rgb_.g_float(); }
  float b_float() const { return rgb_.b_float(); }

  int a() const { return alpha_; }
  float a_float() const { return static_cast<float>(alpha_) / 255; }

  void set_red(const int in) { rgb_.set_red(in); }
  void set_green(const int in) { rgb_.set_green(in); }
  void set_blue(const int in) { rgb_.set_blue(in); }
  void set_alpha(const int in) { alpha_ = in; }

  bool operator==(const RGBAColour& rhs) const;
  bool operator!=(const RGBAColour& rhs) const;

  // Colour constants
  static RGBAColour Clear() { return RGBAColour(0, 0, 0, 0); }
  static RGBAColour Black() { return RGBAColour(0, 0, 0, 255); }
  static RGBAColour White() { return RGBAColour(255, 255, 255, 255); }

 private:
  RGBColour rgb_;

  int alpha_;

  // boost::serialization support
  friend class boost::serialization::access;
  template <class Archive>
  void serialize(Archive& ar, unsigned int version) {
    ar& rgb_& alpha_;
  }
};  // end of class RGBA

// -----------------------------------------------------------------------

std::ostream& operator<<(std::ostream& os, const RGBColour& rgb);
std::ostream& operator<<(std::ostream& os, const RGBAColour& rgba);

#endif  // SRC_SYSTEMS_BASE_COLOUR_H_
