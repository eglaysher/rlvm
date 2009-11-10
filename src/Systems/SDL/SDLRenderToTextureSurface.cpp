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
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
//
// -----------------------------------------------------------------------

#include "Precompiled.hpp"

// -----------------------------------------------------------------------

#include "Systems/SDL/SDLRenderToTextureSurface.hpp"
#include "Systems/SDL/SDLUtils.hpp"
#include "Systems/SDL/Texture.hpp"
#include "Systems/SDL/SDLGraphicsSystem.hpp"

#include "Systems/Base/SystemError.hpp"

#include <SDL/SDL.h>

#include <iostream>
#include <sstream>

#include "Utilities/Exception.hpp"

using namespace std;

// -----------------------------------------------------------------------
// SDLRenderToTextureSurface
// -----------------------------------------------------------------------

SDLRenderToTextureSurface::SDLRenderToTextureSurface(SDLGraphicsSystem* system,
                                                     const Size& size)
    : texture_(new Texture(render_to_texture(), size.width(), size.height())),
      graphics_system_(system) {
  registerWithGraphicsSystem();
}

// -----------------------------------------------------------------------

SDLRenderToTextureSurface::~SDLRenderToTextureSurface() {
  unregisterFromGraphicsSystem();
}

// -----------------------------------------------------------------------

void SDLRenderToTextureSurface::invalidate() {
  // We regretfully can't restore the state here. Oh well. Since
  // SDLRenderToTextureSurface are only used during Effects, we will soon be in
  // the right state.
  texture_.reset();
}

void SDLRenderToTextureSurface::unregisterFromGraphicsSystem() {
  if (graphics_system_) {
    graphics_system_->unregisterSurface(this);
    graphics_system_ = NULL;
  }
}

void SDLRenderToTextureSurface::registerWithGraphicsSystem() {
  if (graphics_system_)
    graphics_system_->registerSurface(this);
}

// -----------------------------------------------------------------------

void SDLRenderToTextureSurface::dump() {
  cerr << "Attempting to dump a remembered texture." << endl;
}

// -----------------------------------------------------------------------

/**
 * @todo This function doesn't ignore alpha blending when use_src_alpha
 *       is false; thus, grp_open and grp_mask_open are really grp_mask_open.
 */
void SDLRenderToTextureSurface::blitToSurface(Surface& dest_surface,
                                              const Rect& src, const Rect& dst,
                                              int alpha, bool use_src_alpha) {
  throw SystemError("Unsupported operation blit_to_surface on "
                    "SDLRenderToTextureSurface!");
}

// -----------------------------------------------------------------------

void SDLRenderToTextureSurface::renderToScreen(
    const Rect& src, const Rect& dst, int opacity) {
  if (texture_)
    texture_->renderToScreen(src, dst, opacity);
}

// -----------------------------------------------------------------------

void SDLRenderToTextureSurface::renderToScreen(
    const Rect& src, const Rect& dst, const int opacity[4]) {
  if (texture_)
    texture_->renderToScreen(src, dst, opacity);
}

// -----------------------------------------------------------------------

void SDLRenderToTextureSurface::renderToScreenAsColorMask(
  const Rect& src, const Rect& dst, const RGBAColour& rgba, int filter) {
  throw rlvm::Exception(
    "SDLRenderToTextureSurface::render_to_screen_as_colour_mask unimplemented");
}

// -----------------------------------------------------------------------

void SDLRenderToTextureSurface::renderToScreenAsObject(
  const GraphicsObject& rp, const Rect& src, const Rect& dst, int alpha) {
  throw rlvm::Exception(
    "SDLRenderToTextureSurface::render_to_screen_as_object unimplemented");
}

// -----------------------------------------------------------------------

void SDLRenderToTextureSurface::fill(const RGBAColour& colour) {
  throw SystemError("Unsupported operation fill on SDLRenderToTextureSurface!");
}

// -----------------------------------------------------------------------

void SDLRenderToTextureSurface::fill(const RGBAColour& colour,
                                     const Rect& rect) {
  throw SystemError("Unsupported operation fill on SDLRenderToTextureSurface!");
}

// -----------------------------------------------------------------------

void SDLRenderToTextureSurface::invert(const Rect& rect) {
  throw SystemError("Unsupported operation fill on SDLRenderToTextureSurface!");
}

// -----------------------------------------------------------------------

void SDLRenderToTextureSurface::mono(const Rect& rect) {
  throw SystemError("Unsupported operation fill on SDLRenderToTextureSurface!");
}

// -----------------------------------------------------------------------

void SDLRenderToTextureSurface::applyColour(
    const RGBColour& colour, const Rect& area) {
  throw SystemError("Unsupported operation fill on SDLRenderToTextureSurface!");
}

// -----------------------------------------------------------------------

void SDLRenderToTextureSurface::getDCPixel(const Point& pos,
                                           int& r, int& g, int& b) {
  throw SystemError("Unsupported operation fill on SDLRenderToTextureSurface!");
}

// -----------------------------------------------------------------------

Size SDLRenderToTextureSurface::size() const {
  if (texture_)
    return Size(texture_->width(), texture_->height());
  else
    return Size();
}

// -----------------------------------------------------------------------

Surface* SDLRenderToTextureSurface::clone() const {
  throw SystemError("Unsupported operation clone on "
                    "SDLRenderToTextureSurface!");
}
