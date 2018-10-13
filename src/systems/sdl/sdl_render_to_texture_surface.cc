// -*- Mode: C++; tab-width:2; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi:tw=80:et:ts=2:sts=2
//
// -----------------------------------------------------------------------
//
// This file is part of RLVM, a RealLive virtual machine clone.
//
// -----------------------------------------------------------------------
//
// Copyright (C) 2006 Elliot Glaysher
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

#include "systems/sdl/sdl_render_to_texture_surface.h"

#include <SDL2/SDL.h>
#include <iostream>
#include <sstream>

#include "base/notification_source.h"
#include "base/notification_type.h"
#include "systems/base/system_error.h"
#include "systems/sdl/sdl_graphics_system.h"
#include "systems/sdl/sdl_utils.h"
#include "systems/sdl/texture.h"
#include "utilities/exception.h"

// -----------------------------------------------------------------------
// SDLRenderToTextureSurface
// -----------------------------------------------------------------------

SDLRenderToTextureSurface::SDLRenderToTextureSurface(SDLGraphicsSystem* system,
                                                     const Size& size)
    : texture_(new Texture(render_to_texture(), size.width(), size.height())) {
  registrar_.Add(this,
                 NotificationType::FULLSCREEN_STATE_CHANGED,
                 Source<GraphicsSystem>(system));
}

SDLRenderToTextureSurface::~SDLRenderToTextureSurface() {}

void SDLRenderToTextureSurface::Dump() {
  std::cerr << "Attempting to dump a remembered texture." << std::endl;
}

void SDLRenderToTextureSurface::BlitToSurface(Surface& dest_surface,
                                              const Rect& src,
                                              const Rect& dst,
                                              int alpha,
                                              bool use_src_alpha) const {
  throw SystemError(
      "Unsupported operation blit_to_surface on "
      "SDLRenderToTextureSurface!");
}

void SDLRenderToTextureSurface::RenderToScreen(const Rect& src,
                                               const Rect& dst,
                                               int opacity) const {
  if (texture_)
    texture_->RenderToScreen(src, dst, opacity);
}

void SDLRenderToTextureSurface::RenderToScreen(const Rect& src,
                                               const Rect& dst,
                                               const int opacity[4]) const {
  if (texture_)
    texture_->RenderToScreen(src, dst, opacity);
}

void SDLRenderToTextureSurface::RenderToScreenAsColorMask(
    const Rect& src,
    const Rect& dst,
    const RGBAColour& rgba,
    int filter) const {
  throw rlvm::Exception(
      "SDLRenderToTextureSurface::render_to_screen_as_colour_mask "
      "unimplemented");
}

void SDLRenderToTextureSurface::RenderToScreenAsObject(const GraphicsObject& rp,
                                                       const Rect& src,
                                                       const Rect& dst,
                                                       int alpha) const {
  throw rlvm::Exception(
      "SDLRenderToTextureSurface::render_to_screen_as_object unimplemented");
}

void SDLRenderToTextureSurface::Fill(const RGBAColour& colour) {
  throw SystemError("Unsupported operation fill on SDLRenderToTextureSurface!");
}

void SDLRenderToTextureSurface::Fill(const RGBAColour& colour,
                                     const Rect& rect) {
  throw SystemError("Unsupported operation fill on SDLRenderToTextureSurface!");
}

void SDLRenderToTextureSurface::Invert(const Rect& rect) {
  throw SystemError(
      "Unsupported operation invert on SDLRenderToTextureSurface!");
}

void SDLRenderToTextureSurface::Mono(const Rect& rect) {
  throw SystemError("Unsupported operation mono on SDLRenderToTextureSurface!");
}

void SDLRenderToTextureSurface::ToneCurve(const ToneCurveRGBMap effect,
                                          const Rect& rect) {
  throw SystemError(
      "Unsupported operation toneCurve on SDLRenderToTextureSurface!");
}

void SDLRenderToTextureSurface::ApplyColour(const RGBColour& colour,
                                            const Rect& area) {
  throw SystemError(
      "Unsupported operation applyColour on SDLRenderToTextureSurface!");
}

void SDLRenderToTextureSurface::GetDCPixel(const Point& pos,
                                           int& r,
                                           int& g,
                                           int& b) const {
  throw SystemError(
      "Unsupported operation GetDCPixel on SDLRenderToTextureSurface!");
}

Size SDLRenderToTextureSurface::GetSize() const {
  if (texture_)
    return Size(texture_->width(), texture_->height());
  else
    return Size();
}

Surface* SDLRenderToTextureSurface::Clone() const {
  throw SystemError(
      "Unsupported operation clone on "
      "SDLRenderToTextureSurface!");
}

void SDLRenderToTextureSurface::Observe(NotificationType type,
                                        const NotificationSource& source,
                                        const NotificationDetails& details) {
  // We regretfully can't restore the state here. Oh well. Since
  // SDLRenderToTextureSurface are only used during Effects, we will soon be in
  // the right state.
  texture_.reset();
}
