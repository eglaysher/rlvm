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

  const ObjectSettings& GetObjectSettingsFor(int obj_num);
};

// -----------------------------------------------------------------------

GraphicsSystem::GraphicsObjectSettings::GraphicsObjectSettings(
    Gameexe& gameexe) {
  if (gameexe.Exists("OBJECT_MAX"))
    objects_in_a_layer = gameexe("OBJECT_MAX");
  else
    objects_in_a_layer = 256;

  // First we populate everything with the special value
  position.reset(new unsigned char[objects_in_a_layer]);
  fill(position.get(), position.get() + objects_in_a_layer, 0);

  if (gameexe.Exists("OBJECT.999"))
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
GraphicsSystem::GraphicsObjectSettings::GetObjectSettingsFor(int obj_num) {
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
    : show_object_1(gameexe("INIT_OBJECT1_ONOFF_MOD").ToInt(0) ? 0 : 1),
      show_object_2(gameexe("INIT_OBJECT2_ONOFF_MOD").ToInt(0) ? 0 : 1),
      show_weather(gameexe("INIT_WEATHER_ONOFF_MOD").ToInt(0) ? 0 : 1),
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
      display_subtitle_(gameexe("SUBTITLE").ToInt(0)),
      interface_hidden_(false),
      globals_(gameexe),
      time_at_last_queue_change_(0),
      graphics_object_settings_(new GraphicsObjectSettings(gameexe)),
      graphics_object_impl_(new GraphicsObjectImpl(
          graphics_object_settings_->objects_in_a_layer)),
      use_custom_mouse_cursor_(gameexe("MOUSE_CURSOR").Exists()),
      show_cursor_from_bytecode_(true),
      cursor_(gameexe("MOUSE_CURSOR").ToInt(0)),
      system_(system),
      preloaded_hik_scripts_(32),
      preloaded_g00_(256),
      image_cache_(10) {}

// -----------------------------------------------------------------------

GraphicsSystem::~GraphicsSystem() {}

// -----------------------------------------------------------------------

void GraphicsSystem::MarkScreenAsDirty(GraphicsUpdateType type) {
  switch (screen_update_mode()) {
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
      oss << "Invalid screen update mode value: " << screen_update_mode();
      throw SystemError(oss.str());
    }
  }
}

// -----------------------------------------------------------------------

void GraphicsSystem::ForceRefresh() {
  screen_needs_refresh_ = true;

  if (screen_update_mode_ == SCREENUPDATEMODE_MANUAL) {
    // Note: SDLEventSystem can also set_force_wait(), in the case of automatic
    // mode.
    system().set_force_wait(true);
  }
}

void GraphicsSystem::OnScreenRefreshed() {
  screen_needs_refresh_ = false;
  object_state_dirty_ = false;
}

// -----------------------------------------------------------------------

void GraphicsSystem::SetScreenUpdateMode(DCScreenUpdateMode u) {
  screen_update_mode_ = u;
}

// -----------------------------------------------------------------------

void GraphicsSystem::QueueShakeSpec(int spec) {
  Gameexe& gameexe = system().gameexe();

  if (gameexe("SHAKE", spec).Exists()) {
    vector<int> spec_vector = gameexe("SHAKE", spec).ToIntVector();

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

    ForceRefresh();
    time_at_last_queue_change_ = system().event().GetTicks();
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

int GraphicsSystem::ShouldUseCustomCursor() {
  return use_custom_mouse_cursor_ &&
         system().gameexe()("MOUSE_CURSOR", cursor_, "NAME").ToString("") != "";
}

// -----------------------------------------------------------------------

void GraphicsSystem::SetCursor(int cursor) {
  cursor_ = cursor;
  mouse_cursor_.reset();
}

// -----------------------------------------------------------------------

void GraphicsSystem::AddGraphicsStackCommand(const std::string& command) {
  graphics_object_impl_->graphics_stack.push_back(command);

  // RealLive only allows 127 commands to be on the stack so game programmers
  // can be lazy and not clear it.
  if (graphics_object_impl_->graphics_stack.size() > 127)
    graphics_object_impl_->graphics_stack.pop_front();
}

// -----------------------------------------------------------------------

int GraphicsSystem::StackSize() const {
  // I don't think this will ever be accurate in the face of multi()
  // commands. I'm not sure if this matters because the only use of StackSize()
  // appears to be this recurring pattern in RL bytecode:
  //
  //   x = stackSize()
  //   ... large graphics demo
  //   stackTrunk(x)
  return graphics_object_impl_->graphics_stack.size();
}

// -----------------------------------------------------------------------

void GraphicsSystem::ClearStack() {
  graphics_object_impl_->graphics_stack.clear();
}

// -----------------------------------------------------------------------

void GraphicsSystem::StackPop(int items) {
  for (int i = 0; i < items; ++i) {
    if (graphics_object_impl_->graphics_stack.size()) {
      graphics_object_impl_->graphics_stack.pop_back();
    }
  }
}

// -----------------------------------------------------------------------

void GraphicsSystem::ReplayGraphicsStack(RLMachine& machine) {
  if (graphics_object_impl_->use_old_graphics_stack) {
    // The actual act of replaying the graphics stack will recreate the graphics
    // stack, so clear it.
    vector<GraphicsStackFrame> stack_to_replay;
    stack_to_replay.swap(graphics_object_impl_->old_graphics_stack);
    ReplayDepricatedGraphicsStackVector(machine, stack_to_replay);
    graphics_object_impl_->use_old_graphics_stack = false;
  } else {
    std::deque<std::string> stack_to_replay;
    stack_to_replay.swap(graphics_object_impl_->graphics_stack);

    machine.set_replaying_graphics_stack(true);
    ReplayGraphicsStackCommand(machine, stack_to_replay);
    machine.set_replaying_graphics_stack(false);
  }
}

// -----------------------------------------------------------------------

void GraphicsSystem::SetHikRenderer(HIKRenderer* renderer) {
  hik_renderer_.reset(renderer);
}

// -----------------------------------------------------------------------

void GraphicsSystem::AddRenderable(Renderable* renderable) {
  final_renderers_.insert(renderable);
}

// -----------------------------------------------------------------------

void GraphicsSystem::RemoveRenderable(Renderable* renderable) {
  final_renderers_.erase(renderable);
}

// -----------------------------------------------------------------------

void GraphicsSystem::SetWindowSubtitle(const std::string& cp932str,
                                       int text_encoding) {
  subtitle_ = cp932str;
}

// -----------------------------------------------------------------------

void GraphicsSystem::SetScreenMode(const int in) {
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

void GraphicsSystem::ToggleFullscreen() { SetScreenMode(screen_mode() ? 0 : 1); }

// -----------------------------------------------------------------------

void GraphicsSystem::ToggleInterfaceHidden() {
  interface_hidden_ = !interface_hidden_;
}

// -----------------------------------------------------------------------

const ObjectSettings& GraphicsSystem::GetObjectSettings(const int obj_num) {
  return graphics_object_settings_->GetObjectSettingsFor(obj_num);
}

// -----------------------------------------------------------------------

void GraphicsSystem::Refresh(std::ostream* tree) {
  BeginFrame();
  DrawFrame(tree);
  EndFrame();
}

boost::shared_ptr<Surface> GraphicsSystem::RenderToSurface() {
  BeginFrame();
  DrawFrame(NULL);
  return EndFrameToSurface();
}

void GraphicsSystem::DrawFrame(std::ostream* tree) {
  switch (background_type_) {
    case BACKGROUND_DC0: {
      // Display DC0
      GetDC(0)->RenderToScreen(screen_rect(), screen_rect(), 255);
      if (tree) {
        // TODO(erg): How do we print the new graphics stack?
        *tree << "Graphic Stack: UNDER CONSTRUCTION" << endl;
      }
      break;
    }
    case BACKGROUND_HIK: {
      if (hik_renderer_) {
        hik_renderer_->Render(tree);
      } else {
        GetHaikei()->RenderToScreen(screen_rect(), screen_rect(), 255);
        if (tree) {
          *tree << "[Haikei bitmap: " << default_bgr_name_ << "]" << endl;
        }
      }
    }
  }

  RenderObjects(tree);

  // Render text
  if (!is_interface_hidden())
    system().text().render(tree);
}

// -----------------------------------------------------------------------

void GraphicsSystem::ExecuteGraphicsSystem(RLMachine& machine) {
  // Check to see if any of the graphics objects are reporting that
  // they want to force a redraw
  for (GraphicsObject& obj : GetForegroundObjects())
    obj.Execute(machine);

  if (mouse_cursor_)
    mouse_cursor_->Execute(system());

  if (hik_renderer_ && background_type_ == BACKGROUND_HIK)
    hik_renderer_->Execute(machine);

  // Possibly update the screen shaking state
  if (!screen_shake_queue_.empty()) {
    unsigned int now = system().event().GetTicks();
    unsigned int accumulated_ticks = now - time_at_last_queue_change_;
    while (!screen_shake_queue_.empty() &&
           accumulated_ticks > screen_shake_queue_.front().second) {
      int frame_ticks = screen_shake_queue_.front().second;
      accumulated_ticks -= frame_ticks;
      time_at_last_queue_change_ += frame_ticks;
      screen_shake_queue_.pop();
      ForceRefresh();
    }
  }
}

// -----------------------------------------------------------------------

void GraphicsSystem::Reset() {
  ClearAllObjects();
  ClearAllDCs();

  preloaded_hik_scripts_.Clear();
  preloaded_g00_.Clear();
  hik_renderer_.reset();
  background_type_ = BACKGROUND_DC0;

  // Reset the cursor
  show_cursor_from_bytecode_ = true;
  cursor_ = system().gameexe()("MOUSE_CURSOR").ToInt(0);
  mouse_cursor_.reset();

  default_grp_name_ = "";
  default_bgr_name_ = "";
  screen_update_mode_ = SCREENUPDATEMODE_AUTOMATIC;
  background_type_ = BACKGROUND_DC0;
  subtitle_ = "";
  interface_hidden_ = false;
}

boost::shared_ptr<const Surface> GraphicsSystem::GetEmojiSurface() {
  GameexeFilteringIterator it = system().gameexe().filtering_begin("E_MOJI.");
  GameexeFilteringIterator end = system().gameexe().filtering_end();
  for (; it != end; ++it) {
    // Try to interpret each key as a filename.
    std::string file_name = it->ToString("");
    boost::shared_ptr<const Surface> surface = GetSurfaceNamed(file_name);
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
  preloaded_hik_scripts_.Clear();
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
    surface = LoadSurfaceFromFile(name);

  if (surface)
    surface->EnsureUploaded();

  preloaded_g00_[slot] = std::make_pair(name, surface);
}

void GraphicsSystem::ClearPreloadedG00(int slot) {
  preloaded_g00_[slot] = std::make_pair("", boost::shared_ptr<const Surface>());
}

void GraphicsSystem::ClearAllPreloadedG00() { preloaded_g00_.Clear(); }

boost::shared_ptr<const Surface> GraphicsSystem::GetPreloadedG00(
    const std::string& name) {
  for (G00ArrayItem& item : preloaded_g00_) {
    if (item.first == name)
      return item.second;
  }

  return boost::shared_ptr<const Surface>();
}

// -----------------------------------------------------------------------

boost::shared_ptr<const Surface> GraphicsSystem::GetSurfaceNamedAndMarkViewed(
    RLMachine& machine,
    const std::string& short_filename) {
  // Record that we viewed this CG.
  cg_table().SetViewed(machine, short_filename);

  return GetSurfaceNamed(short_filename);
}

// -----------------------------------------------------------------------

boost::shared_ptr<const Surface> GraphicsSystem::GetSurfaceNamed(
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
      LoadSurfaceFromFile(short_filename);
  image_cache_.insert(short_filename, surface_to_ret);
  return surface_to_ret;
}

// -----------------------------------------------------------------------

void GraphicsSystem::ClearAndPromoteObjects() {
  typedef LazyArray<GraphicsObject>::full_iterator FullIterator;

  FullIterator bg = graphics_object_impl_->background_objects.full_begin();
  FullIterator bg_end = graphics_object_impl_->background_objects.full_end();
  FullIterator fg = graphics_object_impl_->foreground_objects.full_begin();
  FullIterator fg_end = graphics_object_impl_->foreground_objects.full_end();
  for (; bg != bg_end && fg != fg_end; bg++, fg++) {
    if (fg.valid() && !fg->wipe_copy()) {
      fg->ClearObject();
    }

    if (bg.valid()) {
      *fg = *bg;
      bg->ClearObject();
    }
  }
}

// -----------------------------------------------------------------------

GraphicsObject& GraphicsSystem::GetObject(int layer, int obj_number) {
  if (layer < 0 || layer > 1)
    throw rlvm::Exception("Invalid layer number");

  if (layer == OBJ_BG)
    return graphics_object_impl_->background_objects[obj_number];
  else
    return graphics_object_impl_->foreground_objects[obj_number];
}

// -----------------------------------------------------------------------

void GraphicsSystem::SetObject(int layer, int obj_number, GraphicsObject& obj) {
  if (layer < 0 || layer > 1)
    throw rlvm::Exception("Invalid layer number");

  if (layer == OBJ_BG)
    graphics_object_impl_->background_objects[obj_number] = obj;
  else
    graphics_object_impl_->foreground_objects[obj_number] = obj;
}

// -----------------------------------------------------------------------

void GraphicsSystem::ClearObject(int obj_number) {
  graphics_object_impl_->foreground_objects.DeleteAt(obj_number);
  graphics_object_impl_->background_objects.DeleteAt(obj_number);
}

// -----------------------------------------------------------------------

void GraphicsSystem::ClearAllObjects() {
  graphics_object_impl_->foreground_objects.Clear();
  graphics_object_impl_->background_objects.Clear();
}

// -----------------------------------------------------------------------

void GraphicsSystem::ResetAllObjectsProperties() {
  for (GraphicsObject& object : graphics_object_impl_->foreground_objects)
    object.ResetProperties();

  for (GraphicsObject& object : graphics_object_impl_->background_objects)
    object.ResetProperties();
}

// -----------------------------------------------------------------------

int GraphicsSystem::GetObjectLayerSize() {
  return graphics_object_settings_->objects_in_a_layer;
}

// -----------------------------------------------------------------------

LazyArray<GraphicsObject>& GraphicsSystem::GetBackgroundObjects() {
  return graphics_object_impl_->background_objects;
}

// -----------------------------------------------------------------------

LazyArray<GraphicsObject>& GraphicsSystem::GetForegroundObjects() {
  return graphics_object_impl_->foreground_objects;
}

// -----------------------------------------------------------------------

bool GraphicsSystem::AnimationsPlaying() const {
  for (GraphicsObject& object : graphics_object_impl_->foreground_objects) {
    if (object.has_object_data()) {
      GraphicsObjectData& data = object.GetObjectData();
      if (data.IsAnimation() && data.is_currently_playing())
        return true;
    }
  }

  return false;
}

// -----------------------------------------------------------------------

void GraphicsSystem::TakeSavepointSnapshot() {
  GetForegroundObjects().CopyTo(graphics_object_impl_->saved_foreground_objects);
  GetBackgroundObjects().CopyTo(graphics_object_impl_->saved_background_objects);
  graphics_object_impl_->saved_graphics_stack =
      graphics_object_impl_->graphics_stack;
}

// -----------------------------------------------------------------------

void GraphicsSystem::ClearAllDCs() {
  GetDC(0)->Fill(RGBAColour::Black());

  for (int i = 1; i < 16; ++i)
    FreeDC(i);
}

// -----------------------------------------------------------------------

void GraphicsSystem::RenderObjects(std::ostream* tree) {
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
    const ObjectSettings& settings = GetObjectSettings(it.pos());
    if (settings.obj_on_off == 1 && should_show_object1() == false)
      continue;
    else if (settings.obj_on_off == 2 && should_show_object2() == false)
      continue;
    else if (settings.weather_on_off && should_show_weather() == false)
      continue;
    else if (settings.space_key && is_interface_hidden())
      continue;

    to_render.push_back(std::make_tuple(
        it->z_order(), it->z_layer(), it->z_depth(), it.pos(), &*it));
  }

  // Sort by all the ordering values.
  std::sort(to_render.begin(), to_render.end());

  for (ToRenderVec::iterator it = to_render.begin(); it != to_render.end();
       ++it) {
    get<4>(*it)->Render(get<3>(*it), NULL, tree);
  }
}

// -----------------------------------------------------------------------

boost::shared_ptr<MouseCursor> GraphicsSystem::GetCurrentCursor() {
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

      if (name_key.Exists()) {
        int count = cursor("CONT").ToInt(1);
        int speed = cursor("SPEED").ToInt(800);

        cursor_surface = GetSurfaceNamed(name_key);
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

void GraphicsSystem::SetScreenSize(const Size& size) {
  screen_size_ = size;
  screen_rect_ = Rect(Point(0, 0), size);
}

// -----------------------------------------------------------------------

void GraphicsSystem::MouseMotion(const Point& new_location) {
  if (use_custom_mouse_cursor_ && show_cursor_from_bytecode_)
    MarkScreenAsDirty(GUT_MOUSE_MOTION);

  cursor_pos_ = new_location;
}

// -----------------------------------------------------------------------

GraphicsObjectData* GraphicsSystem::BuildObjOfFile(
    const std::string& filename) {
  // Get the path to get the file type (which won't be in filename)
  fs::path full_path = system().FindFile(filename, OBJ_FILETYPES);
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
  SetWindowSubtitle(subtitle_,
                    Serialization::g_current_machine->GetTextEncoding());
}

// -----------------------------------------------------------------------

template void GraphicsSystem::load<boost::archive::text_iarchive>(
    boost::archive::text_iarchive& ar,
    unsigned int version);
template void GraphicsSystem::save<boost::archive::text_oarchive>(
    boost::archive::text_oarchive& ar,
    unsigned int version) const;
