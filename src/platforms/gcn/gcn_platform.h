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

#ifndef SRC_PLATFORMS_GCN_GCN_PLATFORM_H_
#define SRC_PLATFORMS_GCN_GCN_PLATFORM_H_

#include <boost/enable_shared_from_this.hpp>
#include <boost/scoped_ptr.hpp>

#include <guichan.hpp>
#include <guichan/opengl.hpp>
#include <guichan/sdl.hpp>
#include <guichan/opengl/openglsdlimageloader.hpp>

#include <string>
#include <vector>

#include "systems/base/platform.h"
#include "systems/base/renderable.h"
#include "platforms/gcn/gcn_menu.h"
#include "platforms/gcn/gcn_window.h"
#include "platforms/gcn/gcn_save_load_window.h"

class Gameexe;
class GCNPlatformBlocker;
class Rect;
class System;
struct MenuSpec;

// -----------------------------------------------------------------------

extern const char* EVENT_CANCEL;

// -----------------------------------------------------------------------

/**
 * Guichan GUI.
 */
class GCNPlatform : public Platform,
                    public gcn::MouseListener,
                    public gcn::KeyListener,
                    public boost::enable_shared_from_this<GCNPlatform> {
 public:
  GCNPlatform(System& system, const Rect& screen_size);
  ~GCNPlatform();

  void render();
  void windowCanceled(GCNWindow* window);
  void receiveGCNMenuEvent(GCNMenu* menu, const std::string& event);
  void saveEvent(int slot);
  void loadEvent(int slot);

  // Overridden from Platform:
  virtual void run(RLMachine& machine);
  virtual void showNativeSyscomMenu(RLMachine& machine);
  virtual void invokeSyscomStandardUI(RLMachine& machine, int syscom);
  virtual void showSystemInfo(RLMachine& machine, const RlvmInfo& info);

  // Overridden from gcn::MouseListener:
  virtual void mouseClicked(gcn::MouseEvent& mouseEvent);

  // Overridden from gcn::KeyListener:
  virtual void keyReleased(gcn::KeyEvent& keyEvent);

 private:
  // Blocks the world until we're done.
  void pushBlocker(RLMachine& machine);

  // Initializes all of the above.
  void initializeGuichan(System& system, const Rect& screen_size);

  // Builds the current syscom menu, based on the currently visisble syscom
  // items, erasing the old one if necessary.
  void buildSyscomMenuFor(const std::string& label,
                          const MenuSpec menu_items[],
                          RLMachine& machine);

  // Clears the window stack (unblocking the RLMachine).
  void clearWindowStack();

  // Deallocates the window on the top of the window stack.
  void popWindowFromStack();

  // Displays a window.
  void pushWindowOntoStack(GCNWindow* window);

  // Event Handling functions
  void MenuSave(RLMachine& machine);
  void DoSave(RLMachine& machine, int slot);
  void MenuLoad(RLMachine& machine);
  void DoLoad(RLMachine& machine, int slot);
  void InvokeSyscom(RLMachine& machine, int syscom);

  // This is our LongOperation on the stack.
  GCNPlatformBlocker* blocker_;

  // Used to center dialogs in the window.
  Rect screen_size_;

  // GUIchan syscom implementation
  //
  // In addition to the SDL systems, SDLSystem also owns the guichan based
  // GUI. The following gcn objects are the
  boost::scoped_ptr<gcn::OpenGLSDLImageLoader> sdl_image_loader_;
  boost::scoped_ptr<gcn::OpenGLGraphics> opengl_graphics_;
  boost::scoped_ptr<gcn::SDLInput> sdl_input_;

  boost::scoped_ptr<gcn::Container> toplevel_container_;
  boost::scoped_ptr<gcn::Gui> guichan_gui_;

  boost::scoped_ptr<gcn::Font> global_font_;

  // Stack of menus rendered on top of each other.
  std::vector<GCNWindow*> window_stack_;

  friend class GCNPlatformBlocker;
};  // end of class GCNPlatform

#endif  // SRC_PLATFORMS_GCN_GCN_PLATFORM_H_
