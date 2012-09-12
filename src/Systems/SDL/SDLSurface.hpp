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

#ifndef SRC_SYSTEMS_SDL_SDLSURFACE_HPP_
#define SRC_SYSTEMS_SDL_SDLSURFACE_HPP_

#include <vector>

#include <boost/scoped_ptr.hpp>
#include <boost/ptr_container/ptr_vector.hpp>

#include "base/notification_observer.h"
#include "base/notification_registrar.h"
#include "Systems/Base/Surface.hpp"
#include "Systems/Base/ToneCurve.hpp"

struct SDL_Surface;
class Texture;
class GraphicsSystem;
class SDLGraphicsSystem;
class GraphicsObject;

/**
 * Helper function. Used throughout the SDL system.
 */
SDL_Surface* buildNewSurface(const Size& size);


/**
 * Wrapper around an OpenGL texture; meant to be passed out of the
 * graphics system.
 *
 * Some SDLSurfaces will own their underlying SDL_Surface, for
 * example, anything returned from loadSurfaceFromFile(), while others
 * don't own their surfaces (SDLSurfaces returned by getDC()
 */
class SDLSurface : public Surface,
                   public NotificationObserver {
 private:
  /**
   * Keeps track of a texture and the information about which region
   * of the current surface this Texture is. We keep track of this
   * information so we can reupload a certain part of the Texture
   * without allocating a new opengl texture. glGenTexture()/
   * glTexImage2D() is SLOW and should never be done in a loop.)
   */
  struct TextureRecord {
    /**
     * Builds the texture and
     */
    TextureRecord(SDL_Surface* surface,
                  int x, int y, int w, int h, unsigned int bytes_per_pixel,
                  int byte_order, int byte_type);

    /**
     * Reuploads this current piece of surface from the supplied
     * surface without allocating a new texture.
     */
    void reupload(SDL_Surface* surface, const Rect& dirty);

    /// Clears |texture|. Called before a switch between windowed and
    /// fullscreen mode, so that we aren't holding stale references.
    void forceUnload();

    /// The actual texture.
    boost::shared_ptr<Texture> texture;

    int x_, y_, w_, h_;
    unsigned int bytes_per_pixel_;
    int byte_order_, byte_type_;
  };

  /// The SDL_Surface that contains the software version of the bitmap.
  SDL_Surface* surface_;

  /// The region table
  std::vector<GrpRect> region_table_;

  /// The SDLTexture which wraps one or more OpenGL textures
  mutable std::vector<TextureRecord> textures_;

  /// Whether texture_ represents the contents of surface_. Blits
  /// from surfaces to surfaces invalidate the target surfaces's
  /// texture.
  mutable bool texture_is_valid_;

  /// When a chunk of the surface is invalidated, we only want to upload the
  /// smallest possible area, but for simplicity, we only keep one dirty area.
  mutable Rect dirty_rectangle_;

  /// Whether this surface is DC0 and needs special treatment.
  bool is_dc0_;

  /// A pointer to the graphics_system. We use this to make sure the
  /// GraphicsSystem has a weak_ptr to all SDLSurface instances so it can
  /// invalidate them all in the case of a screen change.
  SDLGraphicsSystem* graphics_system_;

  /**
   * Makes sure that texture_ is a valid object and that it's
   * updated. This method should be called before doing anything with
   * texture_.
   */
  void uploadTextureIfNeeded() const;

  bool is_mask_;

  NotificationRegistrar registrar_;

  static std::vector<int> segmentPicture(int size_remainging);

 public:
  explicit SDLSurface(SDLGraphicsSystem* system);

  /// Surface that takes ownership of an externally created surface
  /// and assumes it is only a single region.
  SDLSurface(SDLGraphicsSystem* system, SDL_Surface* sruf);

  /// Surface that takes ownership of an externally created surface.
  SDLSurface(SDLGraphicsSystem* system, SDL_Surface* surf,
             const std::vector<SDLSurface::GrpRect>& region_table);

  /// Surface created with a specified width and height
  SDLSurface(SDLGraphicsSystem* system, const Size& size);
  ~SDLSurface();

  virtual void EnsureUploaded() const;

  void registerForNotification(GraphicsSystem* system);

  // Whether we have an underlying allocated surface.
  bool allocated() { return surface_; }

  virtual void setIsMask(const bool is) { is_mask_ = is; }

  void buildRegionTable(const Size& size);

  void dump();

  /// allocate a surface
  void allocate(const Size& size);

  void allocate(const Size& size, bool is_dc0);

  /// Deallocate
  void deallocate();

  operator SDL_Surface*() {
    return surface_;
  }

  SDL_Surface* rawSurface() { return surface_; }

  /// Blits to another surface
  virtual void blitToSurface(Surface& dest_surface,
                             const Rect& src, const Rect& dst,
                             int alpha = 255, bool use_src_alpha = true) const;

  void blitFROMSurface(SDL_Surface* src_surface,
                       const Rect& src, const Rect& dst,
                       int alpha = 255, bool use_src_alpha = true);

  virtual void renderToScreen(
    const Rect& src, const Rect& dst, int alpha = 255) const;

  virtual void renderToScreenAsColorMask(
    const Rect& src, const Rect& dst, const RGBAColour& rgba, int filter) const;

  virtual void renderToScreen(
    const Rect& src, const Rect& dst, const int opacity[4]) const;

  // Used internally; not exposed to the general graphics system
  virtual void renderToScreenAsObject(const GraphicsObject& rp,
                                      const Rect& src,
                                      const Rect& dst,
                                      int alpha) const;

  virtual int numPatterns() const;

  /// Returns pattern information.
  virtual const GrpRect& getPattern(int patt_no) const;

  // -----------------------------------------------------------------------

  virtual Size size() const;

  virtual void fill(const RGBAColour& colour);
  virtual void fill(const RGBAColour& colour, const Rect& area);
  virtual void invert(const Rect& rect);
  virtual void mono(const Rect& area);
  virtual void toneCurve(const ToneCurveRGBMap effect, const Rect& area);
  virtual void applyColour(const RGBColour& colour, const Rect& area);

  SDL_Surface* surface() { return surface_; }


  virtual void getDCPixel(const Point& pos, int& r, int& g, int& b) const;
  virtual boost::shared_ptr<Surface> clipAsColorMask(
    const Rect& clip_rect, int r, int g, int b) const;

  virtual Surface* clone() const;

  void interpretAsColorMask(int r, int g, int b, int alpha);

  // Called after each change to surface_. Marks the texture as
  // invalid and notifies SDLGraphicsSystem when appropriate.
  void markWrittenTo(const Rect& written_rect);

  // NotificationObserver:
  virtual void Observe(NotificationType type,
                       const NotificationSource& source,
                       const NotificationDetails& details);
};


#endif  // SRC_SYSTEMS_SDL_SDLSURFACE_HPP_
