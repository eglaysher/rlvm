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
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
//  
// -----------------------------------------------------------------------

#ifndef __Colour_hpp__
#define __Colour_hpp__

#include <boost/serialization/access.hpp>
#include <iosfwd>

/**
 *
 */
class RGBColour
{
public: 
  RGBColour() : r_(0), g_(0), b_(0) {}
  RGBColour(int r, int g, int b) : r_(r), g_(g), b_(b) {}

  int r() const { return r_; }
  int g() const { return g_; }
  int b() const { return b_; }
  float r_float() const { return float(r_) / 256; }
  float g_float() const { return float(g_) / 256; }
  float b_float() const { return float(b_) / 256; }

  void setRed(const int in) { r_ = in; }
  void setGreen(const int in) { g_ = in; }
  void setBlue(const int in) { b_ = in; }

  // Colour constants
  static RGBColour Black() { return RGBColour(0, 0, 0); }
  static RGBColour White() { return RGBColour(255, 255, 255); }

private:
  int r_, g_, b_;

  /// boost::serialization support
  friend class boost::serialization::access;
  template<class Archive>
  void serialize(Archive& ar, unsigned int version) {
    ar & r_ & g_ & b_;
  }
};  // end of class RGB

// -----------------------------------------------------------------------

/**
 * Adds an alpha component to the standard RGB class.
 *
 * @note We compose instead of inherit from RGB because passing in an
 *       instance of RGBA to where one expects an RGB is WRONG!
 */
class RGBAColour
{
public: 
  RGBAColour() : rgb_(0, 0, 0), alpha_(0) { } 
  RGBAColour(int r, int g, int b) : rgb_(r, g, b), alpha_(255) { } 
  RGBAColour(int r, int g, int b, int a) : rgb_(r, g, b), alpha_(a) { } 
  RGBAColour(const RGBColour& colour, int a) : rgb_(colour), alpha_(a) { }

  int r() const { return rgb_.r(); }
  int g() const { return rgb_.g(); }
  int b() const { return rgb_.b(); }
  float r_float() const { return rgb_.r_float(); }
  float g_float() const { return rgb_.g_float(); }
  float b_float() const { return rgb_.b_float(); }

  int a() const { return alpha_; }
  float a_float() const { return float(alpha_) / 256; }

  void setRed(const int in) { rgb_.setRed(in); }
  void setGreen(const int in) { rgb_.setGreen(in); }
  void setBlue(const int in) { rgb_.setBlue(in); }
  void setAlpha(const int in) { alpha_ = in; }

  // Colour constants
  static RGBAColour Clear() { return RGBAColour(0, 0, 0, 0); }
  static RGBAColour Black() { return RGBAColour(0, 0, 0, 255); }
  static RGBAColour White() { return RGBAColour(255, 255, 255, 255); }

private:
  RGBColour rgb_;

  int alpha_;

  /// boost::serialization support
  friend class boost::serialization::access;
  template<class Archive>
  void serialize(Archive& ar, unsigned int version) {
    ar & rgb_ & alpha_;
  }
};  // end of class RGBA


std::ostream& operator<<(std::ostream& os, const RGBColour& rgb);
std::ostream& operator<<(std::ostream& os, const RGBAColour& rgba);

#endif
