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

#include "Precompiled.hpp"

// -----------------------------------------------------------------------

#include "Platforms/gcn/GCNPlatform.hpp"

#include "MachineBase/LongOperation.hpp"
#include "MachineBase/RLMachine.hpp"
#include "MachineBase/Serialization.hpp"
#include "Modules/Module_Sys.hpp"
#include "Platforms/gcn/SDLTrueTypeFont.hpp"
#include "Platforms/gcn/gcnUtils.hpp"
#include "Systems/Base/Rect.hpp"
#include "Systems/Base/System.hpp"
#include "Systems/SDL/SDLEventSystem.hpp"
#include "libReallive/gameexe.h"

#include <iostream>
#include <iomanip>
#include <boost/bind.hpp>

using namespace std;
using boost::bind;

const std::string EVENT_CANCEL = "EVENT_CANCEL";

// A mapping from the SYSCOM_ integer constants to a string suitable for an
// event name.
const char* SYSCOM_EVENTS[] = {
  "SYSCOM_SAVE",
  "SYSCOM_LOAD",
  "SYSCOM_MESSAGE_SPEED",
  "SYSCOM_WINDOW_ATTRIBUTES",
  "SYSCOM_VOLUME_SETTINGS",
  "SYSCOM_DISPLAY_MODE",
  "SYSCOM_MISCELLANEOUS_SETTINGS",
  "NONE",
  "SYSCOM_VOICE_SETTINGS",
  "SYSCOM_FONT_SELECTION",
  "SYSCOM_BGM_FADE",
  "SYSCOM_BGM_SETTINGS",
  "SYSCOM_WINDOW_DECORATION_STYLE",
  "SYSCOM_AUTO_MODE_SETTINGS",
  "SYSCOM_RETURN_TO_PREVIOUS_SELECTION",
  "SYSCOM_USE_KOE",
  "SYSCOM_DISPLAY_VERSION",
  "SYSCOM_SHOW_WEATHER",
  "SYSCOM_SHOW_OBJECT_1",
  "SYSCOM_SHOW_OBJECT_2",
  "SYSCOM_CLASSIFY_TEXT", // ??????? Unknown function.
  "SYSCOM_GENERIC_1",
  "SYSCOM_GENERIC_2",
  "NONE",
  "SYSCOM_OPEN_MANUAL_PATH",
  "SYSCOM_SET_SKIP_MODE",
  "SYSCOM_AUTO_MODE",
  "NONE",
  "SYSCOM_MENU_RETURN",
  "SYSCOM_EXIT_GAME",
  "SYSCOM_HIDE_MENU",
  "SYSCOM_SHOW_BACKGROUND",
  NULL
};

const int MENU_END = -1;
const int MENU_SEPARATOR = -2;
const int MENU_PREFERENCES = -3;

// TODO: Things like SYSCOM_MENU_RETURN need to be turned into menu pointers in
// their own right.
const int SYCOM_MAIN_MENU[] = {
  SYSCOM_AUTO_MODE,
  SYSCOM_SHOW_BACKGROUND,
  MENU_SEPARATOR,
  SYSCOM_SAVE,
  SYSCOM_LOAD,
  SYSCOM_RETURN_TO_PREVIOUS_SELECTION,
  MENU_SEPARATOR,
//  MENU_PREFERENCES,
//  MENU_SEPARATOR,
  SYSCOM_MENU_RETURN,
  SYSCOM_EXIT_GAME,
  MENU_END
};

// -----------------------------------------------------------------------
// GCNPlatformBlocker
// -----------------------------------------------------------------------
class GCNPlatformBlocker : public LongOperation,
                           public RawSDLInputHandler
{
public:
  GCNPlatformBlocker(SDLEventSystem& system,
                     const boost::shared_ptr<GCNPlatform>& platform)
    : event_system_(system), platform_(platform) {
    event_system_.setRawSDLInputHandler(this);
    platform_->blocker_ = this;
  }

  ~GCNPlatformBlocker() {
    event_system_.setRawSDLInputHandler(NULL);
    platform_->blocker_ = NULL;
  }

  void addTask(const boost::function<void(void)>& task) {
    delayed_tasks_.push(task);
  }

  void addMachineTask(const boost::function<bool(RLMachine&)>& task) {
    delayed_rlmachine_tasks_.push(task);
  }

  // Overridden from LongOperation:
  virtual bool operator()(RLMachine& machine) {
    while (delayed_tasks_.size()) {
      delayed_tasks_.front()();
      delayed_tasks_.pop();
    }

    bool stack_is_gone = false;
    while (delayed_rlmachine_tasks_.size()) {
      stack_is_gone |= delayed_rlmachine_tasks_.front()(machine);
      delayed_rlmachine_tasks_.pop();
    }

    if (stack_is_gone) {
      // We've performed an action that has reset or otherwise modified
      // ourselves off the stack. Don't remove the top item from the stack
      // since it's no longer us.
      return false;
    }

    return platform_->window_stack_.size() == 0;
  }

  // Overridden from RawSDLInputHandler:
  virtual void pushInput(SDL_Event event) {
    platform_->sdl_input_->pushInput(event);
  }

private:
  SDLEventSystem& event_system_;
  boost::shared_ptr<GCNPlatform> platform_;

  std::queue<boost::function<void(void)> > delayed_tasks_;
  std::queue<boost::function<bool(RLMachine&)> > delayed_rlmachine_tasks_;
};

// -----------------------------------------------------------------------
// GCNPlatform
// -----------------------------------------------------------------------

GCNPlatform::GCNPlatform(Gameexe& gameexe, const Rect& screen_size)
  : Platform(gameexe)
{
  initializeGuichan(screen_size);
}

// -----------------------------------------------------------------------

GCNPlatform::~GCNPlatform()
{
  clearWindowStack();
}

// -----------------------------------------------------------------------

void GCNPlatform::run(RLMachine& machine)
{
  guichan_gui_->logic();
}

// -----------------------------------------------------------------------

void GCNPlatform::render(RLMachine& machine)
{
  guichan_gui_->draw();
}

// -----------------------------------------------------------------------

void GCNPlatform::showNativeSyscomMenu(RLMachine& machine)
{
  // Block the world!
  SDLEventSystem& system = dynamic_cast<SDLEventSystem&>(
    machine.system().event());
  machine.pushLongOperation(new GCNPlatformBlocker(system, shared_from_this()));

  buildSyscomMenuFor(SYCOM_MAIN_MENU, machine);
}

// -----------------------------------------------------------------------

void GCNPlatform::windowCanceled(GCNWindow* window)
{
  blocker_->addTask(bind(&GCNPlatform::popWindowFromStack, this));
}

// -----------------------------------------------------------------------

void GCNPlatform::receiveGCNMenuEvent(GCNMenu* menu, const std::string& event)
{
  // First, clear the window_stack_
  blocker_->addTask(bind(&GCNPlatform::clearWindowStack, this));

  if (event == SYSCOM_EVENTS[SYSCOM_SAVE])
    blocker_->addMachineTask(bind(&GCNPlatform::MenuSave, this, _1));
  else if (event == SYSCOM_EVENTS[SYSCOM_LOAD])
    blocker_->addMachineTask(bind(&GCNPlatform::MenuLoad, this, _1));
  else if (event == SYSCOM_EVENTS[SYSCOM_EXIT_GAME])
    blocker_->addMachineTask(bind(&GCNPlatform::QuitEvent, this, _1));
  else if (event == SYSCOM_EVENTS[SYSCOM_MENU_RETURN])
    blocker_->addMachineTask(bind(&GCNPlatform::MenuReturnEvent, this, _1));
}

// -----------------------------------------------------------------------

void GCNPlatform::saveEvent(int slot)
{
  // First, clear the window_stack_
  blocker_->addTask(bind(&GCNPlatform::clearWindowStack, this));

  blocker_->addMachineTask(bind(&GCNPlatform::DoSave, this, _1, slot));
}

// -----------------------------------------------------------------------

void GCNPlatform::loadEvent(int slot)
{
  // First, clear the window_stack_
  blocker_->addTask(bind(&GCNPlatform::clearWindowStack, this));

  cerr << "Should load from " << slot << endl;
}

// -----------------------------------------------------------------------
// Private
// -----------------------------------------------------------------------

void GCNPlatform::initializeGuichan(const Rect& screen_size)
{
  sdl_image_loader_.reset(new gcn::OpenGLSDLImageLoader());
	gcn::Image::setImageLoader(sdl_image_loader_.get());

  sdl_input_.reset(new gcn::SDLInput());

  opengl_graphics_.reset(new gcn::OpenGLGraphics(screen_size.width(),
                                                 screen_size.height()));

  guichan_gui_.reset(new gcn::Gui);
  guichan_gui_->setTabbingEnabled(false); // Do I want this on?
  guichan_gui_->setGraphics(opengl_graphics_.get());
  guichan_gui_->setInput(sdl_input_.get());

  toplevel_container_.reset(new gcn::Container);
  toplevel_container_->setBaseColor(gcn::Color(0x000000));
  toplevel_container_->setOpaque(false);
  toplevel_container_->setDimension(rectConvert(screen_size));
  guichan_gui_->setTop(toplevel_container_.get());

  global_font_.reset(new SDLTrueTypeFont("/home/elliot/msgothic.ttc", 12));
	gcn::Widget::setGlobalFont(global_font_.get());
}

// -----------------------------------------------------------------------

void GCNPlatform::buildSyscomMenuFor(const int menu_items[], RLMachine& machine)
{
  System& sys = machine.system();

  std::vector<GCNMenuButton> buttons;
  for (int i = 0; menu_items[i] != MENU_END; ++i) {
    GCNMenuButton button_definition;

    if (menu_items[i] == MENU_SEPARATOR) {
      button_definition.separator = true;
    } else {
      int enabled = sys.isSyscomEnabled(menu_items[i]);
      if (enabled != SYSCOM_INVISIBLE) {
        ostringstream labelss;
        labelss << setw(3) << setfill('0') << menu_items[i];
        button_definition.label = syscomString(labelss.str());
        button_definition.action = SYSCOM_EVENTS[menu_items[i]];
        button_definition.enabled = enabled != SYSCOM_GREYED_OUT;
      }
    }

    buttons.push_back(button_definition);
  }

  pushWindowOntoStack(new GCNMenu(buttons, this));
}

// -----------------------------------------------------------------------

void GCNPlatform::clearWindowStack()
{
  while (window_stack_.size())
    popWindowFromStack();
}

// -----------------------------------------------------------------------

void GCNPlatform::popWindowFromStack()
{
  GCNWindow* to_pop = window_stack_.back();
  window_stack_.pop_back();

  toplevel_container_->remove(to_pop);
  delete to_pop;
}

// -----------------------------------------------------------------------

void GCNPlatform::pushWindowOntoStack(GCNWindow* window)
{
  window->centerInWindow(Size(640, 480));
  window_stack_.push_back(window);
  toplevel_container_->add(window);
}

// -----------------------------------------------------------------------
// Event Handler Functions
// -----------------------------------------------------------------------

bool GCNPlatform::MenuSave(RLMachine& machine) {
  pushWindowOntoStack(
    new GCNSaveLoadWindow(machine, GCNSaveLoadWindow::DO_SAVE, this));
  return false;
}

// -----------------------------------------------------------------------

bool GCNPlatform::DoSave(RLMachine& machine, int slot) {
  Serialization::saveGlobalMemory(machine);
  Serialization::saveGameForSlot(machine, slot);
  return false;
}

// -----------------------------------------------------------------------

bool GCNPlatform::MenuLoad(RLMachine& machine) {
  pushWindowOntoStack(
    new GCNSaveLoadWindow(machine, GCNSaveLoadWindow::DO_LOAD, this));

  return false;
}

// -----------------------------------------------------------------------

bool GCNPlatform::DoLoad(RLMachine& machine, int slot) {

  return true;
}

// -----------------------------------------------------------------------

/* static */
bool GCNPlatform::QuitEvent(RLMachine& machine) {
  machine.halt();

  return false;
}

// -----------------------------------------------------------------------

bool GCNPlatform::MenuReturnEvent(RLMachine& machine) {
  // This is a hack since we probably have a bunch of crap on the stack.
  machine.clearLongOperationsOffBackOfStack();

  // Simulate a MenuReturn.
  Sys_MenuReturn()(machine);

/*
Problem:
Current stack:
{seen=9032, offset=23}
{seen=50, offset=27 [LONG OP=18NewPageAfterLongop]}
{seen=50, offset=27 [LONG OP=18GCNPlatformBlocker]}
{seen=50, offset=27 [LONG OP=10FadeEffect]}

Returning true in this situation is just confusing and 

 */

  return false;
}
