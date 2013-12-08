// -*- Mode: C++; tab-width:2; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi:tw=80:et:ts=2:sts=2
//
// -----------------------------------------------------------------------
//
// This file is part of RLVM, a RealLive virtual machine clone.
//
// -----------------------------------------------------------------------
//
// Copyright (C) 2006, 2007 Elliot Glaysher
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

#ifndef SRC_SYSTEMS_SDL_SDLRENDERTOTEXTURESURFACE_HPP_
#define SRC_SYSTEMS_SDL_SDLRENDERTOTEXTURESURFACE_HPP_

#include <boost/scoped_ptr.hpp>

#include "base/notification_observer.h"
#include "base/notification_registrar.h"
#include "Systems/Base/Surface.hpp"

class SDLGraphicsSystem;
class Texture;

// Fake SDLSurface that holds on to an OpenGL screenshot. Used for composing
// the screenstate with another.
class SDLRenderToTextureSurface : public Surface,
                                  public NotificationObserver {
 public:
  SDLRenderToTextureSurface(SDLGraphicsSystem* system, const Size& size);
  ~SDLRenderToTextureSurface();

  virtual void dump();

  // Blits to another surface
  virtual void blitToSurface(Surface& surface,
                             const Rect& src, const Rect& dst,
                             int alpha = 255, bool use_src_alpha = true) const;

  virtual void renderToScreen(const Rect& src, const Rect& dst,
                              int alpha = 255) const;

  virtual void renderToScreen(const Rect& src, const Rect& dst,
                              const int opacity[4]) const;

  virtual void renderToScreenAsColorMask(
    const Rect& src, const Rect& dst, const RGBAColour& rgba, int filter) const;

  virtual void renderToScreenAsObject(const GraphicsObject& rp,
                                      const Rect& src,
                                      const Rect& dst,
                                      int alpha) const;

  virtual void fill(const RGBAColour& colour);
  virtual void fill(const RGBAColour& colour, const Rect& rect);
  virtual void invert(const Rect& rect);
  virtual void mono(const Rect& area);
  virtual void toneCurve(const ToneCurveRGBMap effect, const Rect& rect);
  virtual void applyColour(const RGBColour& colour, const Rect& area);

  virtual void getDCPixel(const Point& pos, int& r, int& g, int& b) const;

  virtual Size size() const;

  virtual Surface* clone() const;

  // NotificationObserver:
  virtual void Observe(NotificationType type,
                       const NotificationSource& source,
                       const NotificationDetails& details);

 private:
  // The SDLTexture which wraps one or more OpenGL textures
  boost::scoped_ptr<Texture> texture_;

  NotificationRegistrar registrar_;
};


#endif  // SRC_SYSTEMS_SDL_SDLRENDERTOTEXTURESURFACE_HPP_
