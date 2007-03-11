// This file is part of RLVM, a RealLive virtual machine clone.
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

#include <boost/shared_ptr.hpp>
#include "Systems/Base/GraphicsSystem.hpp"

struct SDL_Surface;
class SDLSurface;
class Texture;
class SDLGraphicsSystem;
class GraphicsObject;
class Gameexe;

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
  
  boost::shared_ptr<SDLSurface> m_displayContexts[16];

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

  /// Whether to display (SEEN####)(Line ###) in the title bar
  bool m_displayDataInTitlebar;

  /// The last time the titlebar was updated (in getTicks())
  unsigned int m_timeOfLastTitlebarUpdate;

  /// The last seen number;
  unsigned int m_lastSeenNumber;

  /// The last line number;
  unsigned int m_lastLineNumber;

  /// cp932 encoded title string
  string m_baseTitle;

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

  void setTitle();

public:
  SDLGraphicsSystem(Gameexe& gameexe);

  /** 
   * Should be called by any of the drawing functions the screen is
   * invalidated.
   * 
   * For more information, please see section 5.10.4 of the RLDev
   * manual, which deals with the behaviour of screen updates, and the
   * various modes.
   */
  virtual void markScreenAsDirty();

  /** 
   * Forces a refresh of the screen the next time the graphics system
   * executes.
   */
  virtual void markScreenForRefresh();

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

  boost::shared_ptr<Surface> renderToSurfaceWithBg(
    RLMachine& machine, boost::shared_ptr<Surface> bg);
  boost::shared_ptr<Surface> endFrameToSurface();

  virtual void executeGraphicsSystem(RLMachine& machine);

  virtual int screenWidth() const;
  virtual int screenHeight() const;

  virtual void allocateDC(int dc, int width, int height);
  virtual void freeDC(int dc);

  virtual Surface* loadSurfaceFromFile(const std::string& filename);

  virtual boost::shared_ptr<Surface> getDC(int dc);

  // -----------------------------------------------------------------------

  // Object related functions
  virtual void promoteObjects();

  virtual GraphicsObjectData* buildObjOfFile(RLMachine& machine, const std::string& filename);

  virtual GraphicsObject& getObject(int layer, int objNumber);
  virtual void setObject(int layer, int objNumber, GraphicsObject& object);
};


#endif
