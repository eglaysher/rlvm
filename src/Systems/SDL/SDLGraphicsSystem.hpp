// This file is part of RLVM, a RealLive virutal machine clone.
//
// -----------------------------------------------------------------------
//
// Copyright (C) 2006 El Riot
//  
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
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

#ifndef __SDLGraphicsSystem_hpp_
#define __SDLGraphicsSystem_hpp_

#include <boost/noncopyable.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/ptr_container/ptr_vector.hpp>

#include <SDL/SDL.h>

#include <vector>

#include "Systems/Base/GraphicsSystem.hpp"

class Texture;
class SDLGraphicsSystem;
class GraphicsObject;

/** 
 * Wrapper around an OpenGL texture; meant to be passed out of the
 * graphics system.
 *
 * Some SDLSurfaces will own their underlying SDL_Surface, for
 * example, anything returned from loadSurfaceFromFile(), while others
 * don't own their surfaces (SDLSurfaces returned by getDC()
 */
class SDLSurface : public Surface, public boost::noncopyable
{
public:
  struct GrpRect {
    int x1, y1, x2, y2;
  };

private:
  /// The SDL_Surface that contains the software version of the bitmap.
  SDL_Surface* m_surface;

  /// The region table
  std::vector<GrpRect> m_regionTable;

  /// The SDLTexture which wraps one or more OpenGL textures
  boost::scoped_ptr<Texture> m_texture;

  /// Whether m_texture represents the contents of m_surface. Blits
  /// from surfaces to surfaces invalidate the target surfaces's
  /// texture.
  bool m_textureIsValid;

  /// A pointer to the graphicsSystem. This item being non-NULL means
  /// that this Surface is the special DC0.
  SDLGraphicsSystem* m_graphicsSystem;

  /** 
   * Makes sure that m_texture is a valid object and that it's
   * updated. This method should be called before doing anything with
   * m_texture.
   */
  void uploadTextureIfNeeded();

public:
  SDLSurface();

  /// Surface that takes ownership of an externally created surface.
  SDLSurface(SDL_Surface* surf, 
             const std::vector<SDLSurface::GrpRect>& region_table);

  /// Surface created with a specified width and height
  SDLSurface(int width, int height);
  ~SDLSurface();

  void dump();

  /// allocate a surface
  void allocate(int width, int height);

  void allocate(int width, int height, SDLGraphicsSystem* in);
  
  /// Deallocate
  void deallocate();

  operator SDL_Surface*()
  {
    return m_surface;
  }

  /// Accessor to the texture for doing advanced rendering by composing
  /// SDLSurface into another part of the SDL_* graphics system
  Texture& texture();

  /// Blits to another surface
  virtual void blitToSurface(Surface& surface, 
                     int srcX, int srcY, int srcWidth, int srcHeight,
                     int destX, int destY, int destWidth, int destHeight,
                             int alpha = 255, bool useSrcAlpha = true);

  virtual void renderToScreen(
                     int srcX, int srcY, int srcWidth, int srcHeight,
                     int destX, int destY, int destWidth, int destHeight,
                     int alpha = 255);

  virtual void renderToScreen(
                     int srcX, int srcY, int srcWidth, int srcHeight,
                     int destX, int destY, int destWidth, int destHeight,
                     const int opacity[4]);

  virtual void rawRenderQuad(const int srcCoords[8], 
                             const int destCoords[8],
                             const int opacity[4]);

  // Used internally; not exposed to the general graphics system
  void renderToScreenAsObject(const GraphicsObject& rp);

  /// Called after each change to m_surface. Marks the texture as
  /// invalid and notifies SDLGraphicsSystem when appropriate.
  void markWrittenTo();

  /// Returns pattern information.
  const GrpRect& getPattern(int pattNo) const;

  // -----------------------------------------------------------------------

  int width() const;
  int height() const;

  virtual void fill(int r, int g, int b, int alpha);
  virtual void fill(int r, int g, int b, int alpha, int x, int y, 
                    int width, int height);

  SDL_Surface* surface() { return m_surface; }

  virtual Surface* clone() const;
};

// -----------------------------------------------------------------------

/**
 * Implements all screen output and screen management functionality.
 *
 * @todo This public interface really needs to be rethought out.
 */
class SDLGraphicsSystem : public GraphicsSystem
{
private:
  SDL_Surface* m_screen;
  
  boost::ptr_vector<SDLSurface> displayContexts;

  int m_width, m_height;
  
  /// Flag set to indicate that the screen needs a redraw (usually
  /// because of a blit to DC0)
  bool m_screenDirty;

  /// Flag set to redraw the screen NOW
  bool m_screenNeedsRefresh;

  /// Foreground objects
  GraphicsObject foregroundObjects[256];

  /// Background objects
  GraphicsObject backgroundObjects[256];

  // ---------------------------------------------------------------------

  /** 
   * @name Internal Error Checking Methods
   * 
   * These methods are used internally to seperate out commonly used
   * error checking and parameter validation code from the rest of the
   * class.
   *
   * @{
   */

  /** 
   * Makes sure that a passed in dc number is valid.
   * 
   * @exception Error Throws when dc is greater then the maximum.
   * @exception Error Throws when dc is unallocated.
   */
  void verifySurfaceExists(int dc, const std::string& caller);

  /** 
   * Makes sure that a surface we just allocated was, in fact,
   * allocated.
   */
  void verifyDCAllocation(int dc, const std::string& caller);

  /// @}
  // ---------------------------------------------------------------------

public:
  SDLGraphicsSystem(); 

  /** 
   * Should be called by any of the drawing functions the screen is
   * invalidated.
   * 
   * For more information, please see section 5.10.4 of the RLDev
   * manual, which deals with the behaviour of screen updates, and the
   * various modes.
   */
  virtual void markScreenAsDirty();

  virtual void beginFrame();

  /** 
   * Performs a full redraw of the screen; blitting dc0 to the screen,
   * then blitting all the visible objects, and then blitting all the
   * visible text windows.
   *
   * The Reallive function refresh() simply calls this method, but
   * this method is also used internally to perform the same task.
   *
   * @todo When we support Objects and TextWindows, make this blit
   * them. For now, only blit DC0.
   */
  virtual void refresh(RLMachine& machine);

  virtual void endFrame();

  virtual void executeGraphicsSystem(RLMachine& machine);

  virtual int screenWidth() const;
  virtual int screenHeight() const;

  virtual void allocateDC(int dc, int width, int height);
  virtual void freeDC(int dc);

  virtual Surface* loadSurfaceFromFile(const std::string& filename);

  virtual Surface& getDC(int dc);

  // -----------------------------------------------------------------------

  // Object related functions

  virtual GraphicsObjectData* buildObjOfFile(const std::string& filename);

  virtual GraphicsObject& getFgObject(int objNumber);
  virtual void setFgObject(int objNumber, GraphicsObject& object);
  
  virtual GraphicsObject& getBgObject(int objNumber);
  virtual void setBgObject(int objNumber, GraphicsObject& object);
};


#endif
