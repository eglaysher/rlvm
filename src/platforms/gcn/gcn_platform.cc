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

#include "platforms/gcn/gcn_platform.h"

#include <boost/filesystem.hpp>
#include <functional>
#include <iomanip>
#include <queue>
#include <string>
#include <vector>

#include "machine/long_operation.h"
#include "machine/rlmachine.h"
#include "machine/serialization.h"
#include "modules/module_sys_save.h"
#include "platforms/gcn/gcn_graphics.h"
#include "platforms/gcn/gcn_info_window.h"
#include "platforms/gcn/gcn_true_type_font.h"
#include "platforms/gcn/gcn_utils.h"
#include "systems/base/graphics_system.h"
#include "systems/base/rect.h"
#include "systems/base/system.h"
#include "systems/base/text_system.h"
#include "systems/sdl/sdl_event_system.h"
#include "utilities/exception.h"
#include "utilities/find_font_file.h"
#include "libreallive/gameexe.h"

using std::bind;
using std::placeholders::_1;
namespace fs = boost::filesystem;

const char* EVENT_CANCEL = "EVENT_CANCEL";

// A mapping from the SYSCOM_ integer constants to a string suitable for an
// event name.
const char* SYSCOM_EVENTS[] = {
    "SYSCOM_SAVE",                         "SYSCOM_LOAD",
    "SYSCOM_MESSAGE_SPEED",                "SYSCOM_WINDOW_ATTRIBUTES",
    "SYSCOM_VOLUME_SETTINGS",              "SYSCOM_DISPLAY_MODE",
    "SYSCOM_MISCELLANEOUS_SETTINGS",       "NONE",
    "SYSCOM_VOICE_SETTINGS",               "SYSCOM_FONT_SELECTION",
    "SYSCOM_BGM_FADE",                     "SYSCOM_BGM_SETTINGS",
    "SYSCOM_WINDOW_DECORATION_STYLE",      "SYSCOM_AUTO_MODE_SETTINGS",
    "SYSCOM_RETURN_TO_PREVIOUS_SELECTION", "SYSCOM_USE_KOE",
    "SYSCOM_DISPLAY_VERSION",              "SYSCOM_SHOW_WEATHER",
    "SYSCOM_SHOW_OBJECT_1",                "SYSCOM_SHOW_OBJECT_2",
    "SYSCOM_CLASSIFY_TEXT",  // ??????? Unknown function.
    "SYSCOM_GENERIC_1",                    "SYSCOM_GENERIC_2",
    "NONE",                                "SYSCOM_OPEN_MANUAL_PATH",
    "SYSCOM_SET_SKIP_MODE",                "SYSCOM_AUTO_MODE",
    "NONE",                                "SYSCOM_MENU_RETURN",
    "SYSCOM_EXIT_GAME",                    "SYSCOM_HIDE_MENU",
    "SYSCOM_SHOW_BACKGROUND",              NULL};

const int MENU_END = -1;
const int MENU_SEPARATOR = -2;
const int MENU = -3;

struct MenuSpec {
  // Syscom id >= 0, or a MENU* thing.
  int16_t syscom_id;

  // User interface string key, or NULL for syscom default.
  const char* label;

  // Event to send back if the button is pressed, or NULL for
  // SYSCOM_EVENTS[syscom_id]
  const char* event_name;
};

const char* MENU_PREFERENCES_EVENT = "MENU_PREFERENCES_EVENT";
const MenuSpec MENU_PREFERENCES_MENU[] = {
    {SYSCOM_SCREEN_MODE, NULL, NULL},
    {MENU_SEPARATOR, NULL, NULL},
    {SYSCOM_VOLUME_SETTINGS, NULL, NULL},
    {MENU_SEPARATOR, NULL, NULL},
    {SYSCOM_WINDOW_ATTRIBUTES, NULL, NULL},
    {SYSCOM_WINDOW_DECORATION_STYLE, NULL, NULL},
    {MENU_SEPARATOR, NULL, NULL},
    {SYSCOM_FONT_SELECTION, NULL, NULL},
    {SYSCOM_MESSAGE_SPEED, NULL, NULL},
    {SYSCOM_AUTO_MODE_SETTINGS, NULL, NULL},
    {MENU_SEPARATOR, NULL, NULL},
    {SYSCOM_SHOW_OBJECT_1, NULL, NULL},
    {MENU_SEPARATOR, NULL, NULL},
    {SYSCOM_MISCELLANEOUS_SETTINGS, NULL, NULL},
    {MENU_END, NULL, NULL}};

const char* MENU_RETURN_MENU_EVENT = "MENU_RETURN_MENU_EVENT";
const MenuSpec MENU_RETURN_MENU[] = {
    {SYSCOM_HIDE_MENU, "028.000", EVENT_CANCEL},
    {SYSCOM_MENU_RETURN, "028.001", NULL}, {MENU_END, NULL, NULL}};

const char* EXIT_GAME_MENU_EVENT = "EXIT_GAME_MENU_EVENT";
const MenuSpec EXIT_GAME_MENU[] = {{SYSCOM_HIDE_MENU, "029.000", EVENT_CANCEL},
                                   {SYSCOM_EXIT_GAME, "029.001", NULL},
                                   {MENU_END, NULL, NULL}};

// TODO(erg): Things like SYSCOM_MENU_RETURN need to be turned into menu
// pointers in their own right.
const MenuSpec SYCOM_MAIN_MENU[] = {
    {SYSCOM_SET_SKIP_MODE, NULL, NULL},
    {SYSCOM_AUTO_MODE, NULL, NULL},
    {SYSCOM_SHOW_BACKGROUND, NULL, NULL},
    {MENU_SEPARATOR, NULL, NULL},
    {SYSCOM_SAVE, NULL, NULL},
    {SYSCOM_LOAD, NULL, NULL},
    {SYSCOM_RETURN_TO_PREVIOUS_SELECTION, NULL, NULL},
    {MENU_SEPARATOR, NULL, NULL},
    //  {MENU, "", MENU_PREFERENCES_EVENT},
    //  {MENU_SEPARATOR, 0, 0},
    {MENU, "028", MENU_RETURN_MENU_EVENT},
    {MENU, "029", EXIT_GAME_MENU_EVENT},
    {MENU_END, NULL, NULL}};

// -----------------------------------------------------------------------
// GCNPlatformBlocker
// -----------------------------------------------------------------------
class GCNPlatformBlocker : public LongOperation,
                           public Renderable,
                           public RawSDLInputHandler {
 public:
  GCNPlatformBlocker(SDLEventSystem& system,
                     GraphicsSystem& graphics,
                     const std::shared_ptr<GCNPlatform>& platform)
      : event_system_(system), graphics_system_(graphics), platform_(platform) {
    event_system_.set_raw_sdl_input_handler(this);
    graphics_system_.AddRenderable(this);
    platform_->blocker_ = this;
  }

  ~GCNPlatformBlocker() {
    graphics_system_.RemoveRenderable(this);
    event_system_.set_raw_sdl_input_handler(NULL);
    platform_->blocker_ = NULL;
  }

  void addTask(const std::function<void(void)>& task) {
    delayed_tasks_.push(task);
  }

  void addMachineTask(const std::function<void(RLMachine&)>& task) {
    delayed_rlmachine_tasks_.push(task);
  }

  // Overridden from LongOperation:
  virtual bool operator()(RLMachine& machine) override {
    while (delayed_tasks_.size()) {
      delayed_tasks_.front()();
      delayed_tasks_.pop();
    }

    while (delayed_rlmachine_tasks_.size()) {
      delayed_rlmachine_tasks_.front()(machine);
      delayed_rlmachine_tasks_.pop();
    }

    machine.system().graphics().ForceRefresh();

    return platform_->window_stack_.size() == 0;
  }

  // Overridden from Renderable:
  virtual void Render(std::ostream* tree) override { platform_->render(); }

  // Overridden from RawSDLInputHandler:
  virtual void pushInput(SDL_Event event) override {
    platform_->sdl_input_->pushInput(event);
  }

 private:
  SDLEventSystem& event_system_;
  GraphicsSystem& graphics_system_;
  std::shared_ptr<GCNPlatform> platform_;

  std::queue<std::function<void(void)>> delayed_tasks_;
  std::queue<std::function<void(RLMachine&)>> delayed_rlmachine_tasks_;
};

// -----------------------------------------------------------------------
// GCNPlatform
// -----------------------------------------------------------------------

GCNPlatform::GCNPlatform(System& system, const Rect& screen_size)
    : Platform(system.gameexe()), blocker_(NULL), screen_size_(screen_size) {
  initializeGuichan(system, screen_size);
}

// -----------------------------------------------------------------------

GCNPlatform::~GCNPlatform() {
  toplevel_container_->removeMouseListener(this);
  guichan_gui_->removeGlobalKeyListener(this);

  clearWindowStack();
}

// -----------------------------------------------------------------------

void GCNPlatform::Run(RLMachine& machine) { guichan_gui_->logic(); }

// -----------------------------------------------------------------------

void GCNPlatform::render() {
  try {
    guichan_gui_->draw();
  }
  catch (gcn::Exception& e) {
    std::ostringstream oss;
    oss << "Guichan Exception at " << e.getFunction() << ": " << e.getMessage();
    throw rlvm::Exception(oss.str());
  }

  // HACK: Something is either really wrong with Guichan and/or the intel
  // drivers in Intrepid Ibex. Probably both. Something guichan is
  // doing/drivers haven't implemented is causing a whole bunch of invalid
  // enumerant errors.
  while (glGetError() != GL_NO_ERROR) {}
}

// -----------------------------------------------------------------------

void GCNPlatform::ShowNativeSyscomMenu(RLMachine& machine) {
  pushBlocker(machine);
  buildSyscomMenuFor("", SYCOM_MAIN_MENU, machine);
}

// -----------------------------------------------------------------------

void GCNPlatform::InvokeSyscomStandardUI(RLMachine& machine, int syscom) {
  pushBlocker(machine);
  if (syscom == SYSCOM_SAVE)
    blocker_->addMachineTask(bind(&GCNPlatform::MenuSave, this, _1));
  else if (syscom == SYSCOM_LOAD)
    blocker_->addMachineTask(bind(&GCNPlatform::MenuLoad, this, _1));
}

// -----------------------------------------------------------------------

void GCNPlatform::ShowSystemInfo(RLMachine& machine, const RlvmInfo& info) {
  pushBlocker(machine);
  pushWindowOntoStack(new GCNInfoWindow(machine, info, this));
}

// -----------------------------------------------------------------------

void GCNPlatform::windowCanceled(GCNWindow* window) {
  blocker_->addTask(bind(&GCNPlatform::popWindowFromStack, this));
}

// -----------------------------------------------------------------------

void GCNPlatform::receiveGCNMenuEvent(GCNMenu* menu, const std::string& event) {
  // First, clear the window_stack_
  blocker_->addTask(bind(&GCNPlatform::clearWindowStack, this));

  // Handle triggered syscom events
  for (int i = 0; i <= SYSCOM_SHOW_BACKGROUND; ++i) {
    if (event == SYSCOM_EVENTS[i]) {
      blocker_->addMachineTask(bind(&GCNPlatform::InvokeSyscom, this, _1, i));
      return;
    }
  }

  // Handle our own internal events
  if (event == MENU_PREFERENCES_EVENT) {
    blocker_->addMachineTask(bind(
        &GCNPlatform::buildSyscomMenuFor, this, "", MENU_PREFERENCES_MENU, _1));
  } else if (event == MENU_RETURN_MENU_EVENT) {
    blocker_->addMachineTask(bind(&GCNPlatform::buildSyscomMenuFor,
                                  this,
                                  GetSyscomString("MENU_RETURN_MESS_STR"),
                                  MENU_RETURN_MENU,
                                  _1));
  } else if (event == EXIT_GAME_MENU_EVENT) {
    blocker_->addMachineTask(bind(&GCNPlatform::buildSyscomMenuFor,
                                  this,
                                  GetSyscomString("GAME_END_MESS_STR"),
                                  EXIT_GAME_MENU,
                                  _1));
  }
}

// -----------------------------------------------------------------------

void GCNPlatform::saveEvent(int slot) {
  blocker_->addTask(bind(&GCNPlatform::clearWindowStack, this));
  blocker_->addMachineTask(bind(&GCNPlatform::DoSave, this, _1, slot));
}

// -----------------------------------------------------------------------

void GCNPlatform::loadEvent(int slot) {
  blocker_->addTask(bind(&GCNPlatform::clearWindowStack, this));
  blocker_->addMachineTask(bind(&GCNPlatform::DoLoad, this, _1, slot));
}

// -----------------------------------------------------------------------

void GCNPlatform::mouseClicked(gcn::MouseEvent& mouseEvent) {
  if (mouseEvent.getSource() == toplevel_container_.get() &&
      mouseEvent.getButton() == gcn::MouseEvent::RIGHT) {
    blocker_->addTask(bind(&GCNPlatform::clearWindowStack, this));
  }
}

// -----------------------------------------------------------------------

void GCNPlatform::keyReleased(gcn::KeyEvent& keyEvent) {
  if (keyEvent.getKey() == gcn::Key::ESCAPE)
    blocker_->addTask(bind(&GCNPlatform::clearWindowStack, this));
}

// -----------------------------------------------------------------------
// Private
// -----------------------------------------------------------------------

void GCNPlatform::pushBlocker(RLMachine& machine) {
  if (blocker_ == NULL) {
    // Block the world!
    SDLEventSystem& event =
        dynamic_cast<SDLEventSystem&>(machine.system().event());
    GraphicsSystem& graphics = machine.system().graphics();
    machine.PushLongOperation(
        new GCNPlatformBlocker(event, graphics, shared_from_this()));
  }
}

// -----------------------------------------------------------------------

void GCNPlatform::initializeGuichan(System& system, const Rect& screen_size) {
  sdl_image_loader_.reset(new gcn::OpenGLSDLImageLoader());
  gcn::Image::setImageLoader(sdl_image_loader_.get());

  sdl_input_.reset(new gcn::SDLInput());

  opengl_graphics_.reset(
      new GCNGraphics(screen_size.width(), screen_size.height()));

  guichan_gui_.reset(new gcn::Gui);
  guichan_gui_->setTabbingEnabled(false);  // Do I want this on?
  guichan_gui_->setGraphics(opengl_graphics_.get());
  guichan_gui_->setInput(sdl_input_.get());
  guichan_gui_->addGlobalKeyListener(this);

  toplevel_container_.reset(new gcn::Container);
  toplevel_container_->setBaseColor(gcn::Color(0x000000));
  toplevel_container_->setOpaque(false);
  toplevel_container_->setDimension(rectConvert(screen_size));
  toplevel_container_->addMouseListener(this);
  guichan_gui_->setTop(toplevel_container_.get());

  fs::path font_file = FindFontFile(system);
  global_font_.reset(new GCNTrueTypeFont(font_file.string().c_str(), 12));
  gcn::Widget::setGlobalFont(global_font_.get());
}

// -----------------------------------------------------------------------

void GCNPlatform::buildSyscomMenuFor(const std::string& label,
                                     const MenuSpec menu_items[],
                                     RLMachine& machine) {
  System& sys = machine.system();

  std::vector<GCNMenuButton> buttons;
  for (int i = 0; menu_items[i].syscom_id != MENU_END; ++i) {
    GCNMenuButton button_definition;

    if (menu_items[i].syscom_id == MENU_SEPARATOR) {
      button_definition.separator = true;
      buttons.push_back(button_definition);
    } else if (menu_items[i].syscom_id == MENU) {
      button_definition.label = GetSyscomString(menu_items[i].label);
      button_definition.action = menu_items[i].event_name;
      button_definition.enabled = true;
      buttons.push_back(button_definition);
    } else {
      int id = menu_items[i].syscom_id;
      int enabled = sys.IsSyscomEnabled(id);
      if (enabled != SYSCOM_INVISIBLE) {
        std::ostringstream labelss;
        labelss << std::setw(3) << std::setfill('0') << id;

        if (menu_items[i].label == NULL)
          button_definition.label = GetSyscomString(labelss.str());
        else
          button_definition.label = GetSyscomString(menu_items[i].label);

        if (menu_items[i].event_name == NULL)
          button_definition.action = SYSCOM_EVENTS[id];
        else
          button_definition.action = menu_items[i].event_name;

        button_definition.enabled = enabled != SYSCOM_GREYED_OUT;
        buttons.push_back(button_definition);
      }
    }
  }

  pushWindowOntoStack(new GCNMenu(label, buttons, this));
}

// -----------------------------------------------------------------------

void GCNPlatform::clearWindowStack() {
  while (window_stack_.size())
    popWindowFromStack();
}

// -----------------------------------------------------------------------

void GCNPlatform::popWindowFromStack() {
  GCNWindow* to_pop = window_stack_.back();
  window_stack_.pop_back();

  toplevel_container_->remove(to_pop);
  delete to_pop;
}

// -----------------------------------------------------------------------

void GCNPlatform::pushWindowOntoStack(GCNWindow* window) {
  window->centerInWindow(screen_size_.size());
  window_stack_.push_back(window);
  toplevel_container_->add(window);
}

// -----------------------------------------------------------------------
// Event Handler Functions
// -----------------------------------------------------------------------

void GCNPlatform::MenuSave(RLMachine& machine) {
  pushWindowOntoStack(
      new GCNSaveLoadWindow(machine, GCNSaveLoadWindow::DO_SAVE, this));
}

// -----------------------------------------------------------------------

void GCNPlatform::DoSave(RLMachine& machine, int slot) {
  Serialization::saveGlobalMemory(machine);
  Serialization::saveGameForSlot(machine, slot);
}

// -----------------------------------------------------------------------

void GCNPlatform::MenuLoad(RLMachine& machine) {
  pushWindowOntoStack(
      new GCNSaveLoadWindow(machine, GCNSaveLoadWindow::DO_LOAD, this));
}

// -----------------------------------------------------------------------

void GCNPlatform::DoLoad(RLMachine& machine, int slot) {
  machine.ClearLongOperationsOffBackOfStack();
  Sys_load()(machine, slot);
}

// -----------------------------------------------------------------------

void GCNPlatform::InvokeSyscom(RLMachine& machine, int syscom) {
  machine.system().InvokeSyscom(machine, syscom);
}
