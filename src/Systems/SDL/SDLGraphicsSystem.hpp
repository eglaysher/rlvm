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

#ifndef SRC_SYSTEMS_SDL_SDLGRAPHICSSYSTEM_HPP_
#define SRC_SYSTEMS_SDL_SDLGRAPHICSSYSTEM_HPP_

#include <set>
#include <string>

#include <boost/shared_ptr.hpp>
#include "base/notification_observer.h"
#include "base/notification_registrar.h"
#include "Systems/Base/GraphicsSystem.hpp"

#include <SDL/SDL_opengl.h>

struct SDL_Surface;

class Gameexe;
class GraphicsObject;
class SDLGraphicsSystem;
class SDLSurface;
class System;
class Texture;

// -----------------------------------------------------------------------

/**
 * Implements all screen output and screen management functionality.
 *
 * @todo This public interface really needs to be rethought out.
 */
class SDLGraphicsSystem : public GraphicsSystem,
                          public NotificationObserver {
 public:
  // SDL should be initialized before you create an SDLGraphicsSystem.
  SDLGraphicsSystem(System& system, Gameexe& gameexe);
  ~SDLGraphicsSystem();

//  virtual void setScreenUpdateMode(DCScreenUpdateMode u);

  /**
   * When the cursor is changed, also make sure that it exists so that we can
   * switch on/off the operating system cursor when the cursor index is invalid.
   */
  virtual void setCursor(int cursor);

  virtual void beginFrame();

  virtual void markScreenAsDirty(GraphicsUpdateType type);

  virtual void endFrame();

  void redrawLastFrame();
  void drawCursor();

  virtual boost::shared_ptr<Surface> endFrameToSurface();

  virtual void executeGraphicsSystem(RLMachine& machine);

  virtual void allocateDC(int dc, Size screen_size);
  virtual void setMinimumSizeForDC(int dc, Size size);
  virtual void freeDC(int dc);

  virtual boost::shared_ptr<const Surface> loadSurfaceFromFile(
      const std::string& short_filename);

  virtual boost::shared_ptr<Surface> getHaikei();
  virtual boost::shared_ptr<Surface> getDC(int dc);
  virtual boost::shared_ptr<Surface> buildSurface(const Size& size);

  virtual ColourFilter* BuildColourFiller();

  // -----------------------------------------------------------------------

  virtual void setWindowSubtitle(const std::string& cp932str,
                                 int text_encoding);

  virtual void setScreenMode(const int in);

  // Reset the system. Should clear all state for when a user loads a
  // game.
  virtual void reset();

 private:
  void setupVideo();

  // Makes sure that a passed in dc number is valid.
  //
  // @exception Error Throws when dc is greater then the maximum.
  // @exception Error Throws when dc is unallocated.
  void verifySurfaceExists(int dc, const std::string& caller);

  // Makes sure that a surface we just allocated was, in fact,
  // allocated.
  void verifyDCAllocation(int dc, const std::string& caller);

  void setWindowTitle();

  // NotificationObserver:
  virtual void Observe(NotificationType type,
                       const NotificationSource& source,
                       const NotificationDetails& details);

  // ---------------------------------------------------------------------

  SDL_Surface* screen_;

  boost::shared_ptr<SDLSurface> haikei_;
  boost::shared_ptr<SDLSurface> display_contexts_[16];

  bool redraw_last_frame_;

  // Whether to display (SEEN####)(Line ###) in the title bar
  bool display_data_in_titlebar_;

  // The last time the titlebar was updated (in getTicks())
  unsigned int time_of_last_titlebar_update_;

  // The last seen number;
  int last_seen_number_;

  // The last line number;
  int last_line_number_;

  // utf8 encoded title string
  std::string caption_title_;

  // utf8 encoded subtitle
  std::string subtitle_;

  // The value we've set the window title to. We do this to work around a
  // memory leak in PulseAudio.
  std::string currently_set_title_;

  // Texture used to store the contents of the screen while in DrawManual()
  // mode. The stored image is then used if we need to redraw in the
  // intervening time (expose events, mouse cursor moves, et cetera).
  GLuint screen_contents_texture_;

  // Whether |screen_contents_texture_| is valid to use.
  bool screen_contents_texture_valid_;

  // The size of |screen_contents_texture_|. This can be different
  // from |screen_size_| because textures need to be powers of two on
  // OpenGL v1.x drivers.
  int screen_tex_width_;
  int screen_tex_height_;

  NotificationRegistrar registrar_;
};


#endif  // SRC_SYSTEMS_SDL_SDLGRAPHICSSYSTEM_HPP_
