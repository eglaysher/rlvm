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
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
//
// -----------------------------------------------------------------------
#ifndef __SDLSurface_hpp__
#define __SDLSurface_hpp__

#include <boost/scoped_ptr.hpp>
#include <boost/ptr_container/ptr_vector.hpp>

#include "Systems/Base/Surface.hpp"
#include "Systems/SDL/SurfaceInvalidatable.hpp"

struct SDL_Surface;
class Texture;
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
class SDLSurface : public SurfaceInvalidatable,
                   public Surface
{
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
    void reupload(SDL_Surface* surface);

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
  std::vector<TextureRecord> textures_;

  /// Whether texture_ represents the contents of surface_. Blits
  /// from surfaces to surfaces invalidate the target surfaces's
  /// texture.
  bool texture_is_valid_;

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
  void uploadTextureIfNeeded();

  bool is_mask_;

  static std::vector<int> segmentPicture(int size_remainging);

public:
  SDLSurface(SDLGraphicsSystem* system);

  /// Surface that takes ownership of an externally created surface
  /// and assumes it is only a single region.
  SDLSurface(SDLGraphicsSystem* system, SDL_Surface* sruf);

  /// Surface that takes ownership of an externally created surface.
  SDLSurface(SDLGraphicsSystem* system, SDL_Surface* surf,
             const std::vector<SDLSurface::GrpRect>& region_table);

  /// Surface created with a specified width and height
  SDLSurface(SDLGraphicsSystem* system, const Size& size);
  ~SDLSurface();

  /// Overridden from SurfaceInvalidatable:

  /// Clears |texture|. Called before a switch between windowed and
  /// fullscreen mode, so that we aren't holding stale references.
  virtual void invalidate();

  /// Unregisters this object from the GraphicsSystem.
  virtual void unregisterFromGraphicsSystem();

  virtual void setIsMask(const bool is) { is_mask_ = is; }

  void buildRegionTable(const Size& size);
  void registerWithGraphicsSystem();

  void dump();

  /// allocate a surface
  void allocate(const Size& size);

  void allocate(const Size& size, bool is_dc0);

  /// Deallocate
  void deallocate();

  operator SDL_Surface*()
  {
    return surface_;
  }

  SDL_Surface* rawSurface() { return surface_; }

  /// Blits to another surface
  virtual void blitToSurface(Surface& surface,
                             const Rect& src, const Rect& dst,
                             int alpha = 255, bool use_src_alpha = true);

  void blitFROMSurface(SDL_Surface* src_surface,
                       const Rect& src, const Rect& dst,
                       int alpha = 255, bool use_src_alpha = true);

  virtual void renderToScreen(
    const Rect& src, const Rect& dst, int alpha = 255);

  virtual void renderToScreenAsColorMask(
    const Rect& src, const Rect& dst, const RGBAColour& rgba, int filter);

  virtual void renderToScreen(
    const Rect& src, const Rect& dst, const int opacity[4]);

  virtual void rawRenderQuad(const int src_coords[8],
                             const int dest_coords[8],
                             const int opacity[4]);

  // Used internally; not exposed to the general graphics system
  virtual void renderToScreenAsObject(const GraphicsObject& rp,
                                      const Rect& src,
                                      const Rect& dst,
                                      int alpha);

  /// Called after each change to surface_. Marks the texture as
  /// invalid and notifies SDLGraphicsSystem when appropriate.
  void markWrittenTo();

  virtual int numPatterns() const;

  /// Returns pattern information.
  virtual const GrpRect& getPattern(int patt_no) const;

  // -----------------------------------------------------------------------

  virtual Size size() const;

  virtual void fill(const RGBAColour& colour);
  virtual void fill(const RGBAColour& colour, const Rect& area);

  SDL_Surface* surface() { return surface_; }


  virtual void getDCPixel(const Point& pos, int& r, int& g, int& b);
  virtual boost::shared_ptr<Surface> clipAsColorMask(
    const Rect& clip_rect, int r, int g, int b);

  virtual Surface* clone() const;



  void interpretAsColorMask(int r, int g, int b, int alpha);
};


#endif
