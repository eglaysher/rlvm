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

private:
  int r_, g_, b_;
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
  RGBAColour(int r, int g, int b, int a) : rgb_(r, g, b), alpha_(a) { } 

  int r() const { return rgb_.r(); }
  int g() const { return rgb_.g(); }
  int b() const { return rgb_.b(); }
  float r_float() const { return rgb_.r_float(); }
  float g_float() const { return rgb_.g_float(); }
  float b_float() const { return rgb_.b_float(); }

  int a() const { return alpha_; }
  float a_float() const { return float(alpha_) / 256; }

private:
  RGBColour rgb_;

  int alpha_;
};  // end of class RGBA


#endif
