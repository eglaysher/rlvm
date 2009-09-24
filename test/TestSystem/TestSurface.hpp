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

#ifndef TEST_TESTSYSTEM_TESTSURFACE_HPP_
#define TEST_TESTSYSTEM_TESTSURFACE_HPP_

#include "Systems/Base/Surface.hpp"

#include <string>
#include <vector>

// -----------------------------------------------------------------------

class TestSurface : public Surface {
 public:
  explicit TestSurface(const std::string& surface_name);
  TestSurface(const std::string& surface_name, const Size& size);

  void allocate(const Size& size);
  void deallocate();

  virtual Size size() const;

  virtual void blitToSurface(Surface& surface,
                             const Rect& src, const Rect& dst,
                             int alpha = 255, bool use_src_alpha = true);

  virtual void renderToScreen(
    const Rect& src, const Rect& dst,
    int alpha = 255);

  virtual void renderToScreenAsColorMask(
    const Rect& src, const Rect& dst, const RGBAColour& rgba, int filter);

  virtual void renderToScreen(
    const Rect& src, const Rect& dst, const int opacity[4]);

  virtual void renderToScreenAsObject(const GraphicsObject& rp,
                                      const Rect& src,
                                      const Rect& dst,
                                      int alpha);

  virtual int numPatterns() const;
  virtual const GrpRect& getPattern(int patt_no) const;

  virtual void rawRenderQuad(const int src_coords[8],
                             const int dest_coords[8],
                             const int opacity[4]);

  virtual void fill(const RGBAColour& colour);
  virtual void fill(const RGBAColour& colour, const Rect& rect);

  virtual void getDCPixel(const Point& pos, int& r, int& g, int& b);

  virtual boost::shared_ptr<Surface> clipAsColorMask(
    const Rect& rect, int r, int g, int b);

  virtual Surface* clone() const;

 private:
  /// Unique name of this surface.
  std::string surface_name_;

  /// Whether we are allocated.
  bool allocated_;

  /// Supposed size of this surface.
  Size size_;

  /// The region table
  std::vector<GrpRect> region_table_;
};


#endif  // TEST_TESTSYSTEM_TESTSURFACE_HPP_
