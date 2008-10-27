// -*- Mode: C++; tab-width:2; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi:tw=80:et:ts=2:sts=2
//
// -----------------------------------------------------------------------
//
// This file is part of RLVM, a RealLive virtual machine clone.
//
// -----------------------------------------------------------------------
//
// Copyright (C) 2008 Elliot Glaysher
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
// -----------------------------------------------------------------------

#ifndef __GCNPlatform_hpp__
#define __GCNPlatform_hpp__

#include "Systems/Base/Platform.hpp"
#include "Platforms/gcn/GCNMenu.hpp"
#include "Platforms/gcn/GCNWindow.hpp"
#include "Platforms/gcn/GCNSaveLoadWindow.hpp"

#include <boost/enable_shared_from_this.hpp>
#include <boost/scoped_ptr.hpp>

#include <guichan.hpp>
#include <guichan/opengl.hpp>
#include <guichan/sdl.hpp>
#include <guichan/opengl/openglsdlimageloader.hpp>

#include <vector>

class Gameexe;
class GCNPlatformBlocker;
class Rect;

// -----------------------------------------------------------------------

extern const std::string EVENT_CANCEL;

// -----------------------------------------------------------------------

/**
 * Guichan GUI.
 */
class GCNPlatform : public Platform,
                    public boost::enable_shared_from_this<GCNPlatform>
{
public:
  GCNPlatform(Gameexe& gameexe, const Rect& screen_size);
  ~GCNPlatform();

  // Events from UI objects
  void run(RLMachine& machine);
  void render(RLMachine& machine);
  void showNativeSyscomMenu(RLMachine& machine);
  void windowCanceled(GCNWindow* window);
  void receiveGCNMenuEvent(GCNMenu* menu, const std::string& event);
  void saveEvent(int slot);
  void loadEvent(int slot);

private:
  /// Initializes all of the above.
  void initializeGuichan(const Rect& screen_size);

  /// Builds the current syscom menu, based on the currently visisble syscom
  /// items, erasing the old one if necessary.
  void buildSyscomMenuFor(const int menu_items[], RLMachine& machine);

  /// Clears the window stack (unblocking the RLMachine).
  void clearWindowStack();

  /// Deallocates the window on the top of the window stack.
  void popWindowFromStack();

  /// Displays a window.
  void pushWindowOntoStack(GCNWindow* window);

  /**
   * @name Event Handling functions
   *
   * @{
   */
  void MenuSave(RLMachine& machine);
  void DoSave(RLMachine& machine, int slot);
  void MenuLoad(RLMachine& machine);
  void DoLoad(RLMachine& machine, int slot);
  void QuitEvent(RLMachine& machine);
  void MenuReturnEvent(RLMachine& machine);
  /// @}

  /// This is our LongOperation on the stack.
  GCNPlatformBlocker* blocker_;

  /// We have to delay poping from the stack since most actions that would pop
  /// the stack have call stack frames into those windows...
  bool pop_stack_next_run_;

  /**
   * @name GUIchan syscom implementation
   *
   * In addition to the SDL systems, SDLSystem also owns the guichan based
   * GUI. The following gcn objects are the
   *
   * @{
   */
  boost::scoped_ptr<gcn::OpenGLSDLImageLoader> sdl_image_loader_;
  boost::scoped_ptr<gcn::OpenGLGraphics> opengl_graphics_;
  boost::scoped_ptr<gcn::SDLInput> sdl_input_;

  boost::scoped_ptr<gcn::Container> toplevel_container_;
  boost::scoped_ptr<gcn::Gui> guichan_gui_;

  boost::scoped_ptr<gcn::Font> global_font_;

  /**
   * Stack of menus rendered on top of each other.
   */
  std::vector<GCNWindow*> window_stack_;
  /// @}

  friend class GCNPlatformBlocker;
};  // end of class GCNPlatform


#endif
