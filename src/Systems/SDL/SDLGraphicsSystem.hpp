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
#include <boost/ptr_container/ptr_vector.hpp>

#include <SDL/SDL.h>

#include <vector>

#include "Systems/Base/GraphicsSystem.hpp"


/** 
 * Simple container for SDL_Surface; meant to be passed out of the
 * graphics system.
 *
 * Some SDLSurfaces will own their underlying SDL_Surface, for
 * example, anything returned from loadSurfaceFromFile(), while others
 * don't own their surfaces (SDLSurfaces returned by getDC()
 */
class SDLSurface : public Surface, public boost::noncopyable
{
private:
  SDL_Surface* m_surface;

public:
  SDLSurface()
    : m_surface(NULL) {}

  /// Surface that takes ownership of an externally created surface.
  SDLSurface(SDL_Surface* surf)
    : m_surface(surf) {}

  /// Surface created with a specified width and height
  SDLSurface(int width, int height, SDL_PixelFormat* pixelFormat);

  ~SDLSurface();

  /// allocate a surface
  void allocate(int width, int height, SDL_PixelFormat* pixelFormat);
  
  /// deallocate
  void deallocate();

  operator SDL_Surface*()
  {
    return m_surface;
  }

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

  /** 
   * Called when an SDL function returns a non-zero value.
   * 
   * @param sdlName 
   * @param functionName 
   */
  void reportSDLError(const std::string& sdlName,
                      const std::string& functionName);


  /// @}
  // ---------------------------------------------------------------------


  /** 
   * Should be called by any of the drawing functions when 
   * 
   * For more information, please see section 5.10.4 of the RLDev
   * manual, which deals with the behaviour of screen updates, and the
   * various modes.
   */
  void dc0writtenTo();

public:
  SDLGraphicsSystem(); 

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

  virtual void executeGraphicsSystem();

  virtual int screenWidth() const;
  virtual int screenHeight() const;

  virtual void allocateDC(int dc, int width, int height);
  virtual void freeDC(int dc);

  virtual void wipe(int dc, int r, int g, int b);

  virtual Surface* loadSurfaceFromFile(const std::string& filename);

  virtual Surface& getDC(int dc);
  virtual void blitSurfaceToDC(Surface& sourceObj, int targetDC, 
                               int srcX, int srcY, int srcWidth, int srcHeight,
                               int destX, int destY, int destWidth, int destHeight,
                               int alpha = 255);

//  virtual void recFade(int x, int y, int width, int height,
//                       int r, int g, int b);
};


#endif
