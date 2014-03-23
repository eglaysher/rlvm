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

#include "systems/base/graphics_system.h"

#include <boost/algorithm/string.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/serialization/deque.hpp>
#include <boost/serialization/vector.hpp>

#include <algorithm>
#include <deque>
#include <iostream>
#include <iterator>
#include <list>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

#include "base/notification_details.h"
#include "base/notification_service.h"
#include "base/notification_source.h"
#include "libreallive/expression.h"
#include "libreallive/gameexe.h"
#include "machine/rlmachine.h"
#include "machine/serialization.h"
#include "machine/stack_frame.h"
#include "modules/module_grp.h"
#include "systems/base/anm_graphics_object_data.h"
#include "systems/base/cgm_table.h"
#include "systems/base/event_system.h"
#include "systems/base/graphics_object.h"
#include "systems/base/graphics_object_data.h"
#include "systems/base/graphics_object_of_file.h"
#include "systems/base/graphics_stack_frame.h"
#include "systems/base/hik_renderer.h"
#include "systems/base/hik_script.h"
#include "systems/base/mouse_cursor.h"
#include "systems/base/object_mutator.h"
#include "systems/base/object_settings.h"
#include "systems/base/surface.h"
#include "systems/base/system.h"
#include "systems/base/system_error.h"
#include "systems/base/text_system.h"
#include "utilities/exception.h"
#include "utilities/lazy_array.h"

using boost::iends_with;
using std::cerr;
using std::cout;
using std::endl;
using std::fill;
using std::get;
using std::for_each;
using std::ostringstream;
using std::vector;

namespace fs = boost::filesystem;

// -----------------------------------------------------------------------
// GraphicsSystem::GraphicsObjectSettings
// -----------------------------------------------------------------------
// Impl object
struct GraphicsSystem::GraphicsObjectSettings {
  // Number of graphical objects in a layer.
  int objects_in_a_layer;

  // Each is a valid index into data, refering to
  std::unique_ptr<unsigned char[]> position;

  std::vector<ObjectSettings> data;

  explicit GraphicsObjectSettings(Gameexe& gameexe);

  const ObjectSettings& getObjectSettingsFor(int obj_num);
};

// -----------------------------------------------------------------------

GraphicsSystem::GraphicsObjectSettings::GraphicsObjectSettings(
    Gameexe& gameexe) {
  if (gameexe.exists("OBJECT_MAX"))
    objects_in_a_layer = gameexe("OBJECT_MAX");
  else
    objects_in_a_layer = 256;

  // First we populate everything with the special value
  position.reset(new unsigned char[objects_in_a_layer]);
  fill(position.get(), position.get() + objects_in_a_layer, 0);

  if (gameexe.exists("OBJECT.999"))
    data.push_back(ObjectSettings(gameexe("OBJECT.999")));
  else
    data.push_back(ObjectSettings());

  // Read the #OBJECT.xxx entries from the Gameexe
  GameexeFilteringIterator it = gameexe.filtering_begin("OBJECT.");
  GameexeFilteringIterator end = gameexe.filtering_end();
  for (; it != end; ++it) {
    string s = it->key().substr(it->key().find_first_of(".") + 1);
    std::list<int> object_nums;
    string::size_type poscolon = s.find_first_of(":");
    if (poscolon != string::npos) {
      int obj_num_first = std::stoi(s.substr(0, poscolon));
      int obj_num_last = std::stoi(s.substr(poscolon + 1));
      while (obj_num_first <= obj_num_last) {
        object_nums.push_back(obj_num_first++);
      }
    } else {
      object_nums.push_back(std::stoi(s));
    }

    for (int obj_num : object_nums) {
      if (obj_num != 999 && obj_num < objects_in_a_layer) {
        position[obj_num] = data.size();
        data.push_back(ObjectSettings(*it));
      }
    }
  }
}

// -----------------------------------------------------------------------

const ObjectSettings&
GraphicsSystem::GraphicsObjectSettings::getObjectSettingsFor(int obj_num) {
  return data.at(position[obj_num]);
}

// -----------------------------------------------------------------------
// GraphicsSystemGlobals
// -----------------------------------------------------------------------
GraphicsSystemGlobals::GraphicsSystemGlobals()
    : show_object_1(false),
      show_object_2(false),
      show_weather(false),
      skip_animations(0),
      screen_mode(1),
      cg_table(),
      tone_curves() {}

GraphicsSystemGlobals::GraphicsSystemGlobals(Gameexe& gameexe)
    : show_object_1(gameexe("INIT_OBJECT1_ONOFF_MOD").to_int(0) ? 0 : 1),
      show_object_2(gameexe("INIT_OBJECT2_ONOFF_MOD").to_int(0) ? 0 : 1),
      show_weather(gameexe("INIT_WEATHER_ONOFF_MOD").to_int(0) ? 0 : 1),
      skip_animations(0),
      screen_mode(1),
      cg_table(gameexe),
      tone_curves(gameexe) {}

// -----------------------------------------------------------------------
// GraphicsObjectImpl
// -----------------------------------------------------------------------
struct GraphicsSystem::GraphicsObjectImpl {
  explicit GraphicsObjectImpl(int objects_in_layer);

  // Foreground objects
  LazyArray<GraphicsObject> foreground_objects;

  // Background objects
  LazyArray<GraphicsObject> background_objects;

  // Foreground objects (at the time of the last save)
  LazyArray<GraphicsObject> saved_foreground_objects;

  // Background objects (at the time of the last save)
  LazyArray<GraphicsObject> saved_background_objects;

  // Whether we restore |old_graphics_stack| using the old method instead of
  // replaying the new graphics stack format.
  bool use_old_graphics_stack;

  // List of commands in RealLive bytecode to rebuild the graphics stack at the
  // current moment.
  std::deque<std::string> graphics_stack;

  // Commands to rebuild the graphics stack (at the time of the last savepoint)
  std::deque<std::string> saved_graphics_stack;

  // Old style graphics stack implementation.
  std::vector<GraphicsStackFrame> old_graphics_stack;
};

// -----------------------------------------------------------------------

GraphicsSystem::GraphicsObjectImpl::GraphicsObjectImpl(int size)
    : foreground_objects(size),
      background_objects(size),
      saved_foreground_objects(size),
      saved_background_objects(size),
      use_old_graphics_stack(false) {}

// -----------------------------------------------------------------------
// GraphicsSystem
// -----------------------------------------------------------------------
GraphicsSystem::GraphicsSystem(System& system, Gameexe& gameexe)
    : screen_update_mode_(SCREENUPDATEMODE_AUTOMATIC),
      background_type_(BACKGROUND_DC0),
      screen_needs_refresh_(false),
      object_state_dirty_(false),
      is_responsible_for_update_(true),
      display_subtitle_(gameexe("SUBTITLE").to_int(0)),
      hide_interface_(false),
      globals_(gameexe),
      time_at_last_queue_change_(0),
      graphics_object_settings_(new GraphicsObjectSettings(gameexe)),
      graphics_object_impl_(new GraphicsObjectImpl(
          graphics_object_settings_->objects_in_a_layer)),
      use_custom_mouse_cursor_(gameexe("MOUSE_CURSOR").exists()),
      show_cursor_from_bytecode_(true),
      cursor_(gameexe("MOUSE_CURSOR").to_int(0)),
      system_(system),
      preloaded_hik_scripts_(32),
      preloaded_g00_(256),
      image_cache_(10) {}

// -----------------------------------------------------------------------

GraphicsSystem::~GraphicsSystem() {}

// -----------------------------------------------------------------------

void GraphicsSystem::setIsResponsibleForUpdate(bool in) {
  is_responsible_for_update_ = in;
}

// -----------------------------------------------------------------------

void GraphicsSystem::markScreenAsDirty(GraphicsUpdateType type) {
  switch (screenUpdateMode()) {
    case SCREENUPDATEMODE_AUTOMATIC:
    case SCREENUPDATEMODE_SEMIAUTOMATIC: {
      // Perform a blit of DC0 to the screen, and update it.
      screen_needs_refresh_ = true;
      break;
    }
    case SCREENUPDATEMODE_MANUAL: {
      // Don't really do anything.
      break;
    }
    default: {
      ostringstream oss;
      oss << "Invalid screen update mode value: " << screenUpdateMode();
      throw SystemError(oss.str());
    }
  }
}

// -----------------------------------------------------------------------

void GraphicsSystem::forceRefresh() {
  screen_needs_refresh_ = true;

  if (screen_update_mode_ == SCREENUPDATEMODE_MANUAL) {
    // Note: SDLEventSystem can also setForceWait(), in the case of automatic
    // mode.
    system().setForceWait(true);
  }
}

// -----------------------------------------------------------------------

void GraphicsSystem::setScreenUpdateMode(DCScreenUpdateMode u) {
  screen_update_mode_ = u;
}

// -----------------------------------------------------------------------

void GraphicsSystem::QueueShakeSpec(int spec) {
  Gameexe& gameexe = system().gameexe();

  if (gameexe("SHAKE", spec).exists()) {
    vector<int> spec_vector = gameexe("SHAKE", spec).to_intVector();

    int x, y, time;
    vector<int>::const_iterator it = spec_vector.begin();
    while (it != spec_vector.end()) {
      x = *it++;
      if (it != spec_vector.end()) {
        y = *it++;
        if (it != spec_vector.end()) {
          time = *it++;
          screen_shake_queue_.push(std::make_pair(Point(x, y), time));
        }
      }
    }

    forceRefresh();
    time_at_last_queue_change_ = system().event().getTicks();
  }
}

// -----------------------------------------------------------------------

Point GraphicsSystem::GetScreenOrigin() {
  if (screen_shake_queue_.empty()) {
    return Point(0, 0);
  } else {
    return screen_shake_queue_.front().first;
  }
}

// -----------------------------------------------------------------------

bool GraphicsSystem::IsShaking() const { return !screen_shake_queue_.empty(); }

// -----------------------------------------------------------------------

int GraphicsSystem::CurrentShakingFrameTime() const {
  if (screen_shake_queue_.empty()) {
    return 10;
  } else {
    return screen_shake_queue_.front().second;
  }
}

// -----------------------------------------------------------------------

int GraphicsSystem::useCustomCursor() {
  return use_custom_mouse_cursor_ &&
         system().gameexe()("MOUSE_CURSOR", cursor_, "NAME").to_string("") !=
             "";
}

// -----------------------------------------------------------------------

void GraphicsSystem::setCursor(int cursor) {
  cursor_ = cursor;
  mouse_cursor_.reset();
}

// -----------------------------------------------------------------------

void GraphicsSystem::addGraphicsStackCommand(const std::string& command) {
  graphics_object_impl_->graphics_stack.push_back(command);

  // RealLive only allows 127 commands to be on the stack so game programmers
  // can be lazy and not clear it.
  if (graphics_object_impl_->graphics_stack.size() > 127)
    graphics_object_impl_->graphics_stack.pop_front();
}

// -----------------------------------------------------------------------

int GraphicsSystem::stackSize() const {
  // I don't think this will ever be accurate in the face of multi()
  // commands. I'm not sure if this matters because the only use of stackSize()
  // appears to be this recurring pattern in RL bytecode:
  //
  //   x = stackSize()
  //   ... large graphics demo
  //   stackTrunk(x)
  return graphics_object_impl_->graphics_stack.size();
}

// -----------------------------------------------------------------------

void GraphicsSystem::clearStack() {
  graphics_object_impl_->graphics_stack.clear();
}

// -----------------------------------------------------------------------

void GraphicsSystem::stackPop(int items) {
  for (int i = 0; i < items; ++i) {
    if (graphics_object_impl_->graphics_stack.size()) {
      graphics_object_impl_->graphics_stack.pop_back();
    }
  }
}

// -----------------------------------------------------------------------

void GraphicsSystem::replayGraphicsStack(RLMachine& machine) {
  if (graphics_object_impl_->use_old_graphics_stack) {
    // The actual act of replaying the graphics stack will recreate the graphics
    // stack, so clear it.
    vector<GraphicsStackFrame> stack_to_replay;
    stack_to_replay.swap(graphics_object_impl_->old_graphics_stack);
    replayDepricatedGraphicsStackVector(machine, stack_to_replay);
    graphics_object_impl_->use_old_graphics_stack = false;
  } else {
    std::deque<std::string> stack_to_replay;
    stack_to_replay.swap(graphics_object_impl_->graphics_stack);

    machine.set_replaying_graphics_stack(true);
    replayGraphicsStackCommand(machine, stack_to_replay);
    machine.set_replaying_graphics_stack(false);
  }
}

// -----------------------------------------------------------------------

void GraphicsSystem::setHikRenderer(HIKRenderer* renderer) {
  hik_renderer_.reset(renderer);
}

// -----------------------------------------------------------------------

void GraphicsSystem::addRenderable(Renderable* renderable) {
  final_renderers_.insert(renderable);
}

// -----------------------------------------------------------------------

void GraphicsSystem::removeRenderable(Renderable* renderable) {
  final_renderers_.erase(renderable);
}

// -----------------------------------------------------------------------

void GraphicsSystem::setWindowSubtitle(const std::string& cp932str,
                                       int text_encoding) {
  subtitle_ = cp932str;
}

// -----------------------------------------------------------------------

const std::string& GraphicsSystem::windowSubtitle() const { return subtitle_; }

// -----------------------------------------------------------------------

void GraphicsSystem::setShowObject1(const int in) {
  globals_.show_object_1 = in;
}

// -----------------------------------------------------------------------

void GraphicsSystem::setShowObject2(const int in) {
  globals_.show_object_2 = in;
}

// -----------------------------------------------------------------------

void GraphicsSystem::setShowWeather(const int in) {
  globals_.show_weather = in;
}

// -----------------------------------------------------------------------

void GraphicsSystem::setScreenMode(const int in) {
  bool changed = globals_.screen_mode != in;

  globals_.screen_mode = in;

  if (changed) {
    NotificationService::current()->Notify(
        NotificationType::FULLSCREEN_STATE_CHANGED,
        Source<GraphicsSystem>(this),
        Details<const int>(&in));
  }
}

// -----------------------------------------------------------------------

void GraphicsSystem::toggleFullscreen() { setScreenMode(screenMode() ? 0 : 1); }

// -----------------------------------------------------------------------

void GraphicsSystem::toggleInterfaceHidden() {
  hide_interface_ = !hide_interface_;
}

// -----------------------------------------------------------------------

bool GraphicsSystem::interfaceHidden() { return hide_interface_; }

// -----------------------------------------------------------------------

ObjectSettings GraphicsSystem::getObjectSettings(const int obj_num) {
  return graphics_object_settings_->getObjectSettingsFor(obj_num);
}

// -----------------------------------------------------------------------

void GraphicsSystem::refresh(std::ostream* tree) {
  beginFrame();
  drawFrame(tree);
  endFrame();
}

boost::shared_ptr<Surface> GraphicsSystem::renderToSurface() {
  beginFrame();
  drawFrame(NULL);
  return endFrameToSurface();
}

void GraphicsSystem::drawFrame(std::ostream* tree) {
  switch (background_type_) {
    case BACKGROUND_DC0: {
      // Display DC0
      getDC(0)->renderToScreen(screenRect(), screenRect(), 255);
      if (tree) {
        // TODO(erg): How do we print the new graphics stack?
        *tree << "Graphic Stack: UNDER CONSTRUCTION" << endl;
      }
      break;
    }
    case BACKGROUND_HIK: {
      if (hik_renderer_) {
        hik_renderer_->render(tree);
      } else {
        getHaikei()->renderToScreen(screenRect(), screenRect(), 255);
        if (tree) {
          *tree << "[Haikei bitmap: " << default_bgr_name_ << "]" << endl;
        }
      }
    }
  }

  renderObjects(tree);

  // Render text
  if (!interfaceHidden())
    system().text().render(tree);
}

// -----------------------------------------------------------------------

void GraphicsSystem::executeGraphicsSystem(RLMachine& machine) {
  // Check to see if any of the graphics objects are reporting that
  // they want to force a redraw
  for (GraphicsObject& obj : foregroundObjects())
    obj.execute(machine);

  if (mouse_cursor_)
    mouse_cursor_->execute(system());

  if (hik_renderer_ && background_type_ == BACKGROUND_HIK)
    hik_renderer_->execute(machine);

  // Possibly update the screen shaking state
  if (!screen_shake_queue_.empty()) {
    unsigned int now = system().event().getTicks();
    unsigned int accumulated_ticks = now - time_at_last_queue_change_;
    while (!screen_shake_queue_.empty() &&
           accumulated_ticks > screen_shake_queue_.front().second) {
      int frame_ticks = screen_shake_queue_.front().second;
      accumulated_ticks -= frame_ticks;
      time_at_last_queue_change_ += frame_ticks;
      screen_shake_queue_.pop();
      forceRefresh();
    }
  }
}

// -----------------------------------------------------------------------

void GraphicsSystem::reset() {
  clearAllObjects();
  clearAllDCs();

  preloaded_hik_scripts_.clear();
  preloaded_g00_.clear();
  hik_renderer_.reset();
  background_type_ = BACKGROUND_DC0;

  // Reset the cursor
  show_cursor_from_bytecode_ = true;
  cursor_ = system().gameexe()("MOUSE_CURSOR").to_int(0);
  mouse_cursor_.reset();

  default_grp_name_ = "";
  default_bgr_name_ = "";
  screen_update_mode_ = SCREENUPDATEMODE_AUTOMATIC;
  background_type_ = BACKGROUND_DC0;
  subtitle_ = "";
  hide_interface_ = false;
}

boost::shared_ptr<const Surface> GraphicsSystem::GetEmojiSurface() {
  GameexeFilteringIterator it = system().gameexe().filtering_begin("E_MOJI.");
  GameexeFilteringIterator end = system().gameexe().filtering_end();
  for (; it != end; ++it) {
    // Try to interpret each key as a filename.
    std::string file_name = it->to_string("");
    boost::shared_ptr<const Surface> surface = getSurfaceNamed(file_name);
    if (surface)
      return surface;
  }

  return boost::shared_ptr<const Surface>();
}

void GraphicsSystem::PreloadHIKScript(
    System& system,
    int slot,
    const std::string& name,
    const boost::filesystem::path& file_path) {
  HIKScript* script = new HIKScript(system, file_path);
  script->EnsureUploaded();

  preloaded_hik_scripts_[slot] =
      std::make_pair(name, boost::shared_ptr<HIKScript>(script));
}

void GraphicsSystem::ClearPreloadedHIKScript(int slot) {
  preloaded_hik_scripts_[slot] =
      std::make_pair("", boost::shared_ptr<HIKScript>());
}

void GraphicsSystem::ClearAllPreloadedHIKScripts() {
  preloaded_hik_scripts_.clear();
}

boost::shared_ptr<HIKScript> GraphicsSystem::GetHIKScript(
    System& system,
    const std::string& name,
    const boost::filesystem::path& file_path) {
  for (HIKArrayItem& item : preloaded_hik_scripts_) {
    if (item.first == name)
      return item.second;
  }

  return boost::shared_ptr<HIKScript>(new HIKScript(system, file_path));
}

void GraphicsSystem::PreloadG00(int slot, const std::string& name) {
  // We first check our implicit cache just in case so we don't load it twice.
  boost::shared_ptr<const Surface> surface = image_cache_.fetch(name);
  if (!surface)
    surface = loadSurfaceFromFile(name);

  if (surface)
    surface->EnsureUploaded();

  preloaded_g00_[slot] = std::make_pair(name, surface);
}

void GraphicsSystem::ClearPreloadedG00(int slot) {
  preloaded_g00_[slot] = std::make_pair("", boost::shared_ptr<const Surface>());
}

void GraphicsSystem::ClearAllPreloadedG00() { preloaded_g00_.clear(); }

boost::shared_ptr<const Surface> GraphicsSystem::GetPreloadedG00(
    const std::string& name) {
  for (G00ArrayItem& item : preloaded_g00_) {
    if (item.first == name)
      return item.second;
  }

  return boost::shared_ptr<const Surface>();
}

// -----------------------------------------------------------------------

boost::shared_ptr<const Surface> GraphicsSystem::getSurfaceNamedAndMarkViewed(
    RLMachine& machine,
    const std::string& short_filename) {
  // Record that we viewed this CG.
  cgTable().setViewed(machine, short_filename);

  return getSurfaceNamed(short_filename);
}

// -----------------------------------------------------------------------

boost::shared_ptr<const Surface> GraphicsSystem::getSurfaceNamed(
    const std::string& short_filename) {
  // Check if this is in the script controlled cache.
  boost::shared_ptr<const Surface> cached_surface =
      GetPreloadedG00(short_filename);
  if (cached_surface)
    return cached_surface;

  // First check to see if this surface is already in our internal cache
  cached_surface = image_cache_.fetch(short_filename);
  if (cached_surface)
    return cached_surface;

  boost::shared_ptr<const Surface> surface_to_ret =
      loadSurfaceFromFile(short_filename);
  image_cache_.insert(short_filename, surface_to_ret);
  return surface_to_ret;
}

// -----------------------------------------------------------------------

void GraphicsSystem::clearAndPromoteObjects() {
  typedef LazyArray<GraphicsObject>::full_iterator FullIterator;

  FullIterator bg = graphics_object_impl_->background_objects.full_begin();
  FullIterator bg_end = graphics_object_impl_->background_objects.full_end();
  FullIterator fg = graphics_object_impl_->foreground_objects.full_begin();
  FullIterator fg_end = graphics_object_impl_->foreground_objects.full_end();
  for (; bg != bg_end && fg != fg_end; bg++, fg++) {
    if (fg.valid() && !fg->wipeCopy()) {
      fg->clearObject();
    }

    if (bg.valid()) {
      *fg = *bg;
      bg->clearObject();
    }
  }
}

// -----------------------------------------------------------------------

GraphicsObject& GraphicsSystem::getObject(int layer, int obj_number) {
  if (layer < 0 || layer > 1)
    throw rlvm::Exception("Invalid layer number");

  if (layer == OBJ_BG)
    return graphics_object_impl_->background_objects[obj_number];
  else
    return graphics_object_impl_->foreground_objects[obj_number];
}

// -----------------------------------------------------------------------

void GraphicsSystem::setObject(int layer, int obj_number, GraphicsObject& obj) {
  if (layer < 0 || layer > 1)
    throw rlvm::Exception("Invalid layer number");

  if (layer == OBJ_BG)
    graphics_object_impl_->background_objects[obj_number] = obj;
  else
    graphics_object_impl_->foreground_objects[obj_number] = obj;
}

// -----------------------------------------------------------------------

void GraphicsSystem::clearObject(int obj_number) {
  graphics_object_impl_->foreground_objects.deleteAt(obj_number);
  graphics_object_impl_->background_objects.deleteAt(obj_number);
}

// -----------------------------------------------------------------------

void GraphicsSystem::clearAllObjects() {
  graphics_object_impl_->foreground_objects.clear();
  graphics_object_impl_->background_objects.clear();
}

// -----------------------------------------------------------------------

void GraphicsSystem::resetAllObjectsProperties() {
  for (GraphicsObject& object : graphics_object_impl_->foreground_objects)
    object.resetProperties();

  for (GraphicsObject& object : graphics_object_impl_->background_objects)
    object.resetProperties();
}

// -----------------------------------------------------------------------

int GraphicsSystem::objectLayerSize() {
  return graphics_object_settings_->objects_in_a_layer;
}

// -----------------------------------------------------------------------

LazyArray<GraphicsObject>& GraphicsSystem::backgroundObjects() {
  return graphics_object_impl_->background_objects;
}

// -----------------------------------------------------------------------

LazyArray<GraphicsObject>& GraphicsSystem::foregroundObjects() {
  return graphics_object_impl_->foreground_objects;
}

// -----------------------------------------------------------------------

bool GraphicsSystem::animationsPlaying() const {
  for (GraphicsObject& object : graphics_object_impl_->foreground_objects) {
    if (object.hasObjectData()) {
      GraphicsObjectData& data = object.objectData();
      if (data.isAnimation() && data.currentlyPlaying())
        return true;
    }
  }

  return false;
}

// -----------------------------------------------------------------------

void GraphicsSystem::takeSavepointSnapshot() {
  foregroundObjects().copyTo(graphics_object_impl_->saved_foreground_objects);
  backgroundObjects().copyTo(graphics_object_impl_->saved_background_objects);
  graphics_object_impl_->saved_graphics_stack =
      graphics_object_impl_->graphics_stack;
}

// -----------------------------------------------------------------------

void GraphicsSystem::clearAllDCs() {
  getDC(0)->fill(RGBAColour::Black());

  for (int i = 1; i < 16; ++i)
    freeDC(i);
}

// -----------------------------------------------------------------------

void GraphicsSystem::renderObjects(std::ostream* tree) {
  // The tuple is order, layer, depth, objid, GraphicsObject. Tuples are easy
  // to sort.
  typedef std::vector<std::tuple<int, int, int, int, GraphicsObject*>>
      ToRenderVec;
  ToRenderVec to_render;

  // Collate all objects that we might want to render.
  AllocatedLazyArrayIterator<GraphicsObject> it =
      graphics_object_impl_->foreground_objects.begin();
  AllocatedLazyArrayIterator<GraphicsObject> end =
      graphics_object_impl_->foreground_objects.end();
  for (; it != end; ++it) {
    const ObjectSettings& settings = getObjectSettings(it.pos());
    if (settings.obj_on_off == 1 && showObject1() == false)
      continue;
    else if (settings.obj_on_off == 2 && showObject2() == false)
      continue;
    else if (settings.weather_on_off && showWeather() == false)
      continue;
    else if (settings.space_key && interfaceHidden())
      continue;

    to_render.push_back(std::make_tuple(
        it->zOrder(), it->zLayer(), it->zDepth(), it.pos(), &*it));
  }

  // Sort by all the ordering values.
  std::sort(to_render.begin(), to_render.end());

  for (ToRenderVec::iterator it = to_render.begin(); it != to_render.end();
       ++it) {
    get<4>(*it)->render(get<3>(*it), NULL, tree);
  }
}

// -----------------------------------------------------------------------

boost::shared_ptr<MouseCursor> GraphicsSystem::currentCursor() {
  if (!use_custom_mouse_cursor_ || !show_cursor_from_bytecode_)
    return boost::shared_ptr<MouseCursor>();

  if (use_custom_mouse_cursor_ && !mouse_cursor_) {
    MouseCursorCache::iterator it = cursor_cache_.find(cursor_);
    if (it != cursor_cache_.end()) {
      mouse_cursor_ = it->second;
    } else {
      boost::shared_ptr<const Surface> cursor_surface;
      GameexeInterpretObject cursor =
          system().gameexe()("MOUSE_CURSOR", cursor_);
      GameexeInterpretObject name_key = cursor("NAME");

      if (name_key.exists()) {
        int count = cursor("CONT").to_int(1);
        int speed = cursor("SPEED").to_int(800);

        cursor_surface = getSurfaceNamed(name_key);
        mouse_cursor_.reset(
            new MouseCursor(system(), cursor_surface, count, speed));
        cursor_cache_[cursor_] = mouse_cursor_;
      } else {
        mouse_cursor_.reset();
      }
    }
  }

  return mouse_cursor_;
}

// -----------------------------------------------------------------------

void GraphicsSystem::mouseMotion(const Point& new_location) {
  if (use_custom_mouse_cursor_ && show_cursor_from_bytecode_)
    markScreenAsDirty(GUT_MOUSE_MOTION);

  cursor_pos_ = new_location;
}

// -----------------------------------------------------------------------

GraphicsObjectData* GraphicsSystem::buildObjOfFile(
    const std::string& filename) {
  // Get the path to get the file type (which won't be in filename)
  fs::path full_path = system().findFile(filename, OBJ_FILETYPES);
  if (full_path.empty()) {
    ostringstream oss;
    oss << "Could not find Object compatible file \"" << filename << "\".";
    throw rlvm::Exception(oss.str());
  }

  string file_str = full_path.string();
  if (iends_with(file_str, "g00") || iends_with(file_str, "pdt")) {
    return new GraphicsObjectOfFile(system(), filename);
  } else if (iends_with(file_str, "anm")) {
    return new AnmGraphicsObjectData(system(), filename);
  } else {
    ostringstream oss;
    oss << "Don't know how to handle object file: \"" << filename << "\"";
    throw rlvm::Exception(oss.str());
  }
}

// -----------------------------------------------------------------------

template <class Archive>
void GraphicsSystem::save(Archive& ar, unsigned int version) const {
  ar& subtitle_& default_grp_name_& default_bgr_name_& graphics_object_impl_
      ->saved_graphics_stack& graphics_object_impl_->saved_background_objects&
            graphics_object_impl_->saved_foreground_objects;
}

// -----------------------------------------------------------------------

template <class Archive>
void GraphicsSystem::load(Archive& ar, unsigned int version) {
  ar& subtitle_;
  if (version > 0) {
    ar& default_grp_name_;
    ar& default_bgr_name_;
    graphics_object_impl_->use_old_graphics_stack = false;
    ar& graphics_object_impl_->graphics_stack;
  } else {
    graphics_object_impl_->use_old_graphics_stack = true;
    ar& graphics_object_impl_->old_graphics_stack;
  }

  ar& graphics_object_impl_->background_objects& graphics_object_impl_
      ->foreground_objects;

  // Now alert all subclasses that we've set the subtitle
  setWindowSubtitle(subtitle_,
                    Serialization::g_current_machine->GetTextEncoding());
}

// -----------------------------------------------------------------------

template void GraphicsSystem::load<boost::archive::text_iarchive>(
    boost::archive::text_iarchive& ar,
    unsigned int version);
template void GraphicsSystem::save<boost::archive::text_oarchive>(
    boost::archive::text_oarchive& ar,
    unsigned int version) const;
