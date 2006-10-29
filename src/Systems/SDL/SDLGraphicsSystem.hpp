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
private:
  /// The SDL_Surface that contains the software version of the bitmap.
  SDL_Surface* m_surface;

  /// The SDLTexture which wraps one or more OpenGL textures
  boost::scoped_ptr<Texture> m_texture;

  /// Whether m_texture represents the contents of m_surface. Blits
  /// from surfaces to surfaces invalidate the target surfaces's
  /// texture.
  bool m_textureIsValid;

  /// A pointer to the graphicsSystem. This item being non-NULL means
  /// that this Surface is the special DC0.
  SDLGraphicsSystem* m_graphicsSystem;

public:
  SDLSurface();

  /// Surface that takes ownership of an externally created surface.
  SDLSurface(SDL_Surface* surf);

  /// Surface created with a specified width and height
  SDLSurface(int width, int height);
  ~SDLSurface();

  /// allocate a surface
  void allocate(int width, int height);

  void allocate(int width, int height, SDLGraphicsSystem* in);
  
  /// Deallocate
  void deallocate();

  operator SDL_Surface*()
  {
    return m_surface;
  }

  /// Blits to another surface
  virtual void blitToSurface(Surface& surface, 
                     int srcX, int srcY, int srcWidth, int srcHeight,
                     int destX, int destY, int destWidth, int destHeight,
                     int alpha = 255);

  virtual void renderToScreen(
                     int srcX, int srcY, int srcWidth, int srcHeight,
                     int destX, int destY, int destWidth, int destHeight,
                     int alpha = 255);

  /// Called after each change to m_surface. Marks the texture as
  /// invalid and notifies SDLGraphicsSystem when appropriate.
  void markWrittenTo();

  // -----------------------------------------------------------------------

  int width() const;
  int height() const;

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
   * Should be called by any of the drawing functions when 
   * 
   * For more information, please see section 5.10.4 of the RLDev
   * manual, which deals with the behaviour of screen updates, and the
   * various modes.
   */
  void dc0writtenTo();


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
  virtual void refresh();

  virtual void endFrame();

  virtual void executeGraphicsSystem();

  virtual int screenWidth() const;
  virtual int screenHeight() const;

  virtual void allocateDC(int dc, int width, int height);
  virtual void freeDC(int dc);

  virtual void wipe(int dc, int r, int g, int b);

  virtual Surface* loadSurfaceFromFile(const std::string& filename);

  virtual Surface& getDC(int dc);

  // Wrapper around 
  /** 
   * Sets the current color;
   * 
   * @param r 
   * @param g 
   * @param b 
   */
//  virtual void rawSetColor(float r, float g, float b);

  /** 
   * Displays a 
   * 
   * @param texture 
   * @param srcX 
   * @param srcY 
   * @param srcWidth 
   * @param srcHeight 
   * @param destX 
   * @param destY 
   * @param destWidth 
   * @param destHeight 
   */
//   virtual void rawDisplayQuad(Surface& texture, 
//                               int srcX, int srcY, int srcWidth, int srcHeight,
//                               int destX, int destY, int destWidth, int destHeight);


//  virtual void recFade(int x, int y, int width, int height,
//                       int r, int g, int b);
};


#endif
