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

#ifndef TEST_TESTSYSTEM_MOCKSURFACE_HPP_
#define TEST_TESTSYSTEM_MOCKSURFACE_HPP_

#include "gmock/gmock.h"

#include "Systems/Base/Surface.hpp"
#include "Systems/Base/GraphicsObject.hpp"

#include <string>
#include <vector>

// -----------------------------------------------------------------------

// A suface which has 90% of it mocked out for testing.
class MockSurface : public Surface {
 public:
  // Creates NiceMock<>ed MockSurfaces.
  static MockSurface* Create(const std::string& surface_name);
  static MockSurface* Create(const std::string& surface_name, const Size& size);

  // Size related stuff.
  void allocate(const Size& size);
  void deallocate();
  virtual Size size() const;

  MOCK_CONST_METHOD5(
      blitToSurface,
      void(Surface&, const Rect&, const Rect&, int alpha, bool use_src_alpha));
  MOCK_CONST_METHOD3(renderToScreen, void(const Rect&, const Rect&, int alpha));
  MOCK_CONST_METHOD4(renderToScreenAsColorMask,
                     void(const Rect&, const Rect&, const RGBAColour&, int));
  MOCK_CONST_METHOD3(renderToScreen,
                     void(const Rect&, const Rect&, const int[4]));
  MOCK_CONST_METHOD4(
      renderToScreenAsObject,
      void(const GraphicsObject&, const Rect&, const Rect&, int));
  MOCK_CONST_METHOD0(numPatterns, int());
  MOCK_CONST_METHOD1(getPattern, const GrpRect&(int patt_no));
  MOCK_METHOD1(fill, void(const RGBAColour&));
  MOCK_METHOD2(fill, void(const RGBAColour&, const Rect&));
  MOCK_METHOD1(invert, void(const Rect&));
  MOCK_METHOD2(toneCurve, void(const ToneCurveRGBMap effect, const Rect& area));
  MOCK_METHOD1(mono, void(const Rect&));
  MOCK_METHOD2(applyColour, void(const RGBColour&, const Rect&));

  MOCK_CONST_METHOD4(getDCPixel, void(const Point&, int&, int&, int&));

  // Concrete implementations of the cloning methods.
  virtual boost::shared_ptr<Surface> clipAsColorMask(const Rect& rect,
                                                     int r,
                                                     int g,
                                                     int b);
  virtual Surface* clone() const;

 protected:
  // The real constructors are private. (See static creation methods above).
  explicit MockSurface(const std::string& surface_name);
  MockSurface(const std::string& surface_name, const Size& size);

 private:
  // Unique name of this surface.
  std::string surface_name_;

  // Whether we are allocated.
  bool allocated_;

  // Supposed size of this surface.
  Size size_;

  // The region table
  std::vector<GrpRect> region_table_;
};

#endif  // TEST_TESTSYSTEM_MOCKSURFACE_HPP_
