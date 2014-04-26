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

#ifndef SRC_SYSTEMS_BASE_SURFACE_H_
#define SRC_SYSTEMS_BASE_SURFACE_H_

#include <memory>

#include "systems/base/rect.h"
#include "systems/base/tone_curve.h"

class RGBColour;
class RGBAColour;
class GraphicsObject;
struct GraphicsObjectOverride;

// Abstract surface used in rlvm. Various systems graphics systems should
// provide a subclass of Surface that implement all the following primitives.
class Surface : public std::enable_shared_from_this<Surface> {
 public:
  struct GrpRect {
    Rect rect;

    // Describes an offset to rect. Why VisualArts threw this in is
    // unknown.
    int originX, originY;
  };

 public:
  Surface();
  virtual ~Surface();

  // Optional method which makes sure the data is on the graphics card for
  // uploading.
  virtual void EnsureUploaded() const {}

  // ------------------------------------------------- [ Drawing functions ]

  // Fills the surface with |colour|.
  virtual void Fill(const RGBAColour& colour) = 0;

  // Fills |area| with |colour|.
  virtual void Fill(const RGBAColour& colour, const Rect& area) = 0;

  // Apply the given tone curve effect to the surface
  virtual void ToneCurve(const ToneCurveRGBMap effect, const Rect& area) = 0;

  // Inverts each color of the surface (like a photo negative).
  virtual void Invert(const Rect& area) = 0;

  // Turns |area| to grayscale.
  virtual void Mono(const Rect& area) = 0;

  // Applies |colour| to |area| of the surface.
  virtual void ApplyColour(const RGBColour& colour, const Rect& area) = 0;

  // ----------------------------------------------------- [ Uncategorized ]
  virtual void SetIsMask(const bool is) {}

  virtual Size GetSize() const = 0;
  Rect GetRect() const;

  virtual void Dump();

  // Blits to another surface
  virtual void BlitToSurface(Surface& dest_surface,
                             const Rect& src,
                             const Rect& dst,
                             int alpha = 255,
                             bool use_src_alpha = true) const = 0;

  virtual void RenderToScreen(const Rect& src,
                              const Rect& dst,
                              int alpha = 255) const = 0;

  virtual void RenderToScreenAsColorMask(const Rect& src,
                                         const Rect& dst,
                                         const RGBAColour& colour,
                                         int filter) const = 0;

  virtual void RenderToScreen(const Rect& src,
                              const Rect& dst,
                              const int opacity[4]) const = 0;

  virtual void RenderToScreenAsObject(const GraphicsObject& rp,
                                      const Rect& src,
                                      const Rect& dst,
                                      int alpha) const = 0;

  virtual int GetNumPatterns() const;
  virtual const GrpRect& GetPattern(int patt_no) const;

  virtual void GetDCPixel(const Point& pos, int& r, int& g, int& b) const = 0;

  virtual std::shared_ptr<Surface> ClipAsColorMask(const Rect& clip_rect,
                                                     int r,
                                                     int g,
                                                     int b) const;

  virtual Surface* Clone() const = 0;
};

#endif  // SRC_SYSTEMS_BASE_SURFACE_H_
