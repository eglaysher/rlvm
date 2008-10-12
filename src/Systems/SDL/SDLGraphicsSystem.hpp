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

#ifndef __SDLGraphicsSystem_hpp_
#define __SDLGraphicsSystem_hpp_

#include <boost/shared_ptr.hpp>
#include "Systems/Base/GraphicsSystem.hpp"

#include "lru_cache.hpp"

#include <SDL/SDL_opengl.h>

struct SDL_Surface;
class SDLSurface;
class Texture;
class SDLGraphicsSystem;
class System;
class GraphicsObject;
class Gameexe;

// -----------------------------------------------------------------------

/// These are function pointers to OpenGL extensions that we use in
/// fragment shaders, which we do to get the closest blending to how
/// RealLive does it.
//extern gl_create_shader_object_arbproc_ptr gl_create_shader_object;

// -----------------------------------------------------------------------

/**
 * Implements all screen output and screen management functionality.
 *
 * @todo This public interface really needs to be rethought out.
 */
class SDLGraphicsSystem : public GraphicsSystem
{
private:
  SDL_Surface* screen_;

  boost::shared_ptr<SDLSurface> display_contexts_[16];

  bool redraw_last_frame_;

  /// Whether to display (SEEN####)(Line ###) in the title bar
  bool display_data_in_titlebar_;

  /// The last time the titlebar was updated (in getTicks())
  unsigned int time_of_last_titlebar_update_;

  /// The last seen number;
  int last_seen_number_;

  /// The last line number;
  int last_line_number_;

  /// utf8 encoded title string
  std::string caption_title_;

  /// utf8 encoded subtitle
  std::string subtitle_;

  /// Window icon
  SDL_Surface* icon_;

  /**
   * Texture used to temporarily store what's behind the cursor while
   * we render this frame.
   */
  GLuint behind_cursor_texture_;

  /**
   * LRU cache filled with the last fifteen accessed images.
   *
   * This cache's contents are assumed to be immutable.
   *
   * @todo Think about enforcing the immutability by adding 'const' to
   *       half the program.
   */
  LRUCache<std::string, boost::shared_ptr<Surface> > image_cache_;

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

  void setWindowTitle();


public:
  SDLGraphicsSystem(System& system, Gameexe& gameexe);

  /**
   * When the cursor is changed, also make sure that it exists so that we can
   * switch on/off the operating system cursor when the cursor index is invalid.
   */
  virtual void setCursor(RLMachine& machine, int cursor);

  virtual void beginFrame();

  virtual void markScreenAsDirty(GraphicsUpdateType type);

  virtual void endFrame(RLMachine& machine);

  boost::shared_ptr<Surface> renderToSurfaceWithBg(
    RLMachine& machine, boost::shared_ptr<Surface> bg);
  boost::shared_ptr<Surface> endFrameToSurface();

  virtual void executeGraphicsSystem(RLMachine& machine);

  virtual void allocateDC(int dc, Size screen_size);
  virtual void freeDC(int dc);

  virtual boost::shared_ptr<Surface> loadSurfaceFromFile(
    RLMachine& machine, const std::string& short_filename);

  virtual boost::shared_ptr<Surface> getDC(int dc);
  virtual boost::shared_ptr<Surface> buildSurface(const Size& size);
  // -----------------------------------------------------------------------

  virtual void setWindowSubtitle(const std::string& cp932str,
                                 int text_encoding);

  virtual void clearAllDCs();

  /**
   * Reset the system. Should clear all state for when a user loads a
   * game.
   */
  virtual void reset();
};


#endif
