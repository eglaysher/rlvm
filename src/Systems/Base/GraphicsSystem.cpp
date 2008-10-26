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

#include "Precompiled.hpp"

// -----------------------------------------------------------------------

#include "Systems/Base/GraphicsSystem.hpp"

#include "LazyArray.hpp"
#include "MachineBase/RLMachine.hpp"
#include "MachineBase/Serialization.hpp"
#include "MachineBase/StackFrame.hpp"
#include "Modules/Module_Grp.hpp"
#include "Systems/Base/AnmGraphicsObjectData.hpp"
#include "Systems/Base/CGMTable.hpp"
#include "Systems/Base/GraphicsObject.hpp"
#include "Systems/Base/GraphicsObjectData.hpp"
#include "Systems/Base/GraphicsObjectOfFile.hpp"
#include "Systems/Base/GraphicsStackFrame.hpp"
#include "Systems/Base/MouseCursor.hpp"
#include "Systems/Base/ObjectSettings.hpp"
#include "Systems/Base/ObjectSettings.hpp"
#include "Systems/Base/Surface.hpp"
#include "Systems/Base/System.hpp"
#include "Systems/Base/SystemError.hpp"
#include "Systems/Base/TextSystem.hpp"
#include "Utilities.h"
#include "libReallive/gameexe.h"

#include <algorithm>
#include <boost/algorithm/string.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/serialization/scoped_ptr.hpp>
#include <boost/serialization/vector.hpp>
#include <iostream>
#include <iterator>
#include <list>
#include <sstream>
#include <vector>

using boost::iends_with;
using boost::lexical_cast;
using std::cerr;
using std::cout;
using std::endl;
using std::fill;
using std::ostringstream;
using std::vector;

namespace fs = boost::filesystem;

// -----------------------------------------------------------------------
// GraphicsSystem::GraphicsObjectSettings
// -----------------------------------------------------------------------
/// Impl object
struct GraphicsSystem::GraphicsObjectSettings
{
  /// Each is a valid index into data, refering to
  unsigned char position[OBJECTS_IN_A_LAYER];

  std::vector<ObjectSettings> data;

  std::vector<GraphicsStackFrame> graphics_stack;

  GraphicsObjectSettings(Gameexe& gameexe);

  const ObjectSettings& getObjectSettingsFor(int obj_num);
};

// -----------------------------------------------------------------------

GraphicsSystem::GraphicsObjectSettings::GraphicsObjectSettings(
  Gameexe& gameexe)
{
  // First we populate everything with the special value
  fill(position, position + OBJECTS_IN_A_LAYER, 0);
  if(gameexe.exists("OBJECT.999"))
    data.push_back(ObjectSettings(gameexe("OBJECT.999")));
  else
    data.push_back(ObjectSettings());

  // Read the #OBJECT.xxx entries from the Gameexe
  GameexeFilteringIterator it = gameexe.filtering_begin("OBJECT.");
  GameexeFilteringIterator end = gameexe.filtering_end();
  for(; it != end; ++it)
  {
    string s = it->key().substr(it->key().find_first_of(".") + 1);
    std::list<int> object_nums;
    string::size_type poscolon = s.find_first_of(":");
    if ( poscolon != string::npos )
    {
      int obj_num_first = lexical_cast<int>(s.substr(0, poscolon));
      int obj_num_last = lexical_cast<int>(s.substr(poscolon + 1));
      while ( obj_num_first <= obj_num_last )
      {
        object_nums.push_back(obj_num_first++);
      }
    }
    else
    {
      object_nums.push_back(lexical_cast<int>(s));
    }

    for( std::list<int>::const_iterator intit = object_nums.begin();
         intit != object_nums.end(); ++intit )
    {
      int obj_num = *intit;
      if(obj_num != 999 && obj_num < OBJECTS_IN_A_LAYER)
      {
        position[obj_num] = data.size();
        data.push_back(ObjectSettings(*it));
      }
    }
  }
}

// -----------------------------------------------------------------------

const ObjectSettings& GraphicsSystem::GraphicsObjectSettings::getObjectSettingsFor(
  int obj_num)
{
  return data[position[obj_num]];
}

// -----------------------------------------------------------------------
// GraphicsSystemGlobals
// -----------------------------------------------------------------------
GraphicsSystemGlobals::GraphicsSystemGlobals()
  : show_object_1(false), show_object_2(false), show_weather(false),
    cg_table()
{}

GraphicsSystemGlobals::GraphicsSystemGlobals(Gameexe& gameexe)
  :	show_object_1(gameexe("INIT_OBJECT1_ONOFF_MOD").to_int(0) ? 0 : 1),
    show_object_2(gameexe("INIT_OBJECT2_ONOFF_MOD").to_int(0) ? 0 : 1),
    show_weather(gameexe("INIT_WEATHER_ONOFF_MOD").to_int(0) ? 0 : 1),
    cg_table(gameexe)
{}

// -----------------------------------------------------------------------
// GraphicsObjectImpl
// -----------------------------------------------------------------------
struct GraphicsSystem::GraphicsObjectImpl
{
  GraphicsObjectImpl();

  /// Foreground objects
  LazyArray<GraphicsObject> foreground_objects;

  /// Background objects
  LazyArray<GraphicsObject> background_objects;

  /// Foreground objects (at the time of the last save)
  LazyArray<GraphicsObject> saved_foreground_objects;

  /// Background objects (at the time of the last save)
  LazyArray<GraphicsObject> saved_background_objects;
};

// -----------------------------------------------------------------------

GraphicsSystem::GraphicsObjectImpl::GraphicsObjectImpl()
  : foreground_objects(256), background_objects(256),
    saved_foreground_objects(256), saved_background_objects(256)
{}

// -----------------------------------------------------------------------
// GraphicsSystem
// -----------------------------------------------------------------------
GraphicsSystem::GraphicsSystem(System& system, Gameexe& gameexe)
  : screen_update_mode_(SCREENUPDATEMODE_AUTOMATIC),
    screen_needs_refresh_(false),
    is_responsible_for_update_(true),
    display_subtitle_(gameexe("SUBTITLE").to_int(0)),
    hide_interface_(false),
    globals_(gameexe),
    graphics_object_settings_(new GraphicsObjectSettings(gameexe)),
    graphics_object_impl_(new GraphicsObjectImpl),
    use_custom_mouse_cursor_(gameexe("MOUSE_CURSOR").exists()),
    show_curosr_(true),
    cursor_(gameexe("MOUSE_CURSOR").to_int(0)),
    system_(system)
{}

// -----------------------------------------------------------------------

GraphicsSystem::~GraphicsSystem()
{}

// -----------------------------------------------------------------------

void GraphicsSystem::setIsResponsibleForUpdate(bool in)
{
  is_responsible_for_update_ = in;
}

// -----------------------------------------------------------------------

void GraphicsSystem::markScreenAsDirty(GraphicsUpdateType type)
{
  switch(screenUpdateMode())
  {
  case SCREENUPDATEMODE_AUTOMATIC:
  case SCREENUPDATEMODE_SEMIAUTOMATIC:
  {
    // Perform a blit of DC0 to the screen, and update it.
    screen_needs_refresh_ = true;
    break;
  }
  case SCREENUPDATEMODE_MANUAL:
  {
    // Don't really do anything.
    break;
  }
  default:
  {
    ostringstream oss;
    oss << "Invalid screen update mode value: " << screenUpdateMode();
    throw SystemError(oss.str());
  }
  }
}

// -----------------------------------------------------------------------

void GraphicsSystem::forceRefresh()
{
  screen_needs_refresh_ = true;
}

// -----------------------------------------------------------------------

void GraphicsSystem::setScreenUpdateMode(DCScreenUpdateMode u)
{
  screen_update_mode_ = u;
}

// -----------------------------------------------------------------------

int GraphicsSystem::useCustomCursor() {
  return use_custom_mouse_cursor_ &&
    system().gameexe()("MOUSE_CURSOR", cursor_, "NAME").exists();
}

// -----------------------------------------------------------------------

void GraphicsSystem::setCursor(RLMachine& machine, int cursor)
{
  cursor_ = cursor;
  mouse_cursor_.reset();
}

// -----------------------------------------------------------------------

GraphicsStackFrame& GraphicsSystem::addGraphicsStackFrame(
  const std::string& name)
{
  graphics_object_settings_->graphics_stack.push_back(GraphicsStackFrame(name));
  return graphics_object_settings_->graphics_stack.back();
}

// -----------------------------------------------------------------------

vector<GraphicsStackFrame>& GraphicsSystem::graphicsStack()
{
  return graphics_object_settings_->graphics_stack;
}

// -----------------------------------------------------------------------

int GraphicsSystem::stackSize() const
{
  return graphics_object_settings_->graphics_stack.size();
}

// -----------------------------------------------------------------------

void GraphicsSystem::clearStack()
{
  graphics_object_settings_->graphics_stack.clear();
}

// -----------------------------------------------------------------------

void GraphicsSystem::stackPop(int items)
{
  for(int i = 0; i < items; ++i)
    graphics_object_settings_->graphics_stack.pop_back();
}

// -----------------------------------------------------------------------

void GraphicsSystem::replayGraphicsStack(RLMachine& machine)
{
  // The actual act of replaying the graphics stack will recreate the graphics
  // stack, so clear it.
  vector<GraphicsStackFrame> stack_to_replay;
  stack_to_replay.swap(graphics_object_settings_->graphics_stack);
  replayGraphicsStackVector(machine, stack_to_replay);
}

// -----------------------------------------------------------------------

void GraphicsSystem::setWindowSubtitle(const std::string& cp932str,
                                       int text_encoding)
{
  subtitle_ = cp932str;
}

// -----------------------------------------------------------------------

const std::string& GraphicsSystem::windowSubtitle() const
{
  return subtitle_;
}

// -----------------------------------------------------------------------

void GraphicsSystem::setShowObject1(const int in)
{
  globals_.show_object_1 = in;
}

// -----------------------------------------------------------------------

void GraphicsSystem::setShowObject2(const int in)
{
  globals_.show_object_2 = in;
}

// -----------------------------------------------------------------------

void GraphicsSystem::setShowWeather(const int in)
{
  globals_.show_weather = in;
}

// -----------------------------------------------------------------------

void GraphicsSystem::toggleInterfaceHidden()
{
  hide_interface_ = !hide_interface_;
}

// -----------------------------------------------------------------------

bool GraphicsSystem::interfaceHidden()
{
  return hide_interface_;
}

// -----------------------------------------------------------------------

ObjectSettings GraphicsSystem::getObjectSettings(const int obj_num)
{
  return graphics_object_settings_->getObjectSettingsFor(obj_num);
}

// -----------------------------------------------------------------------

// Default implementations for some functions (which probably have
// default implementations because I'm lazy, and these really should
// be pure virtual)
void GraphicsSystem::beginFrame() { }
void GraphicsSystem::endFrame(RLMachine& machine) { }

void GraphicsSystem::refresh(RLMachine& machine, std::ostream* tree) {
  beginFrame();

  // Display DC0
  getDC(0)->renderToScreen(screenRect(), screenRect(), 255);
  if (tree) {
    *tree << "Graphic Stack:" << endl;
    const vector<GraphicsStackFrame>& gstack = graphicsStack();
    for (vector<GraphicsStackFrame>::const_iterator it = gstack.begin();
         it != gstack.end(); ++it) {
      *tree << "  " << *it << endl;
    }
  }

  renderObjects(machine, tree);

  // Render text
  if(!interfaceHidden())
    machine.system().text().render(machine, tree);

  endFrame(machine);
}

// -----------------------------------------------------------------------

void GraphicsSystem::dumpRenderTree(std::ostream& tree) {
  // TODO
}

// -----------------------------------------------------------------------

boost::shared_ptr<Surface> GraphicsSystem::renderToSurfaceWithBg(
  RLMachine& machine, boost::shared_ptr<Surface> bg)
{ return boost::shared_ptr<Surface>(); }

// -----------------------------------------------------------------------

void GraphicsSystem::reset()
{
  default_grp_name_ = "";
  default_bgr_name_ = "";
  screen_update_mode_ = SCREENUPDATEMODE_AUTOMATIC;
  subtitle_ = "";
  hide_interface_ = false;
}

// -----------------------------------------------------------------------

void GraphicsSystem::promoteObjects()
{
  typedef LazyArray<GraphicsObject>::full_iterator FullIterator;

  FullIterator bg = graphics_object_impl_->background_objects.full_begin();
  FullIterator bg_end = graphics_object_impl_->background_objects.full_end();
  FullIterator fg = graphics_object_impl_->foreground_objects.full_begin();
  FullIterator fg_end = graphics_object_impl_->foreground_objects.full_end();
  for(; bg != bg_end && fg != fg_end; bg++, fg++)
  {
    if(bg.valid())
    {
      *fg = *bg;
      bg->deleteObject();
    }
  }
}

// -----------------------------------------------------------------------

/// @todo The looping constructs here totally defeat the purpose of
///       LazyArray, and make it a bit worse.
void GraphicsSystem::clearAndPromoteObjects()
{
  typedef LazyArray<GraphicsObject>::full_iterator FullIterator;

  FullIterator bg = graphics_object_impl_->background_objects.full_begin();
  FullIterator bg_end = graphics_object_impl_->background_objects.full_end();
  FullIterator fg = graphics_object_impl_->foreground_objects.full_begin();
  FullIterator fg_end = graphics_object_impl_->foreground_objects.full_end();
  for(; bg != bg_end && fg != fg_end; bg++, fg++)
  {
    if(fg.valid() && !fg->wipeCopy())
    {
      fg->deleteObject();
    }

    if(bg.valid())
    {
      *fg = *bg;
      bg->deleteObject();
    }
  }
}

// -----------------------------------------------------------------------

GraphicsObject& GraphicsSystem::getObject(int layer, int obj_number)
{
  if(layer < 0 || layer > 1)
    throw rlvm::Exception("Invalid layer number");

  if(layer == OBJ_BG_LAYER)
    return graphics_object_impl_->background_objects[obj_number];
  else
    return graphics_object_impl_->foreground_objects[obj_number];
}

// -----------------------------------------------------------------------

void GraphicsSystem::setObject(int layer, int obj_number, GraphicsObject& obj)
{
  if(layer < 0 || layer > 1)
    throw rlvm::Exception("Invalid layer number");

  if(layer == OBJ_BG_LAYER)
    graphics_object_impl_->background_objects[obj_number] = obj;
  else
    graphics_object_impl_->foreground_objects[obj_number] = obj;
}

// -----------------------------------------------------------------------

void GraphicsSystem::clearAllObjects()
{
  graphics_object_impl_->foreground_objects.clear();
  graphics_object_impl_->background_objects.clear();
}

// -----------------------------------------------------------------------

LazyArray<GraphicsObject>& GraphicsSystem::backgroundObjects()
{
  return graphics_object_impl_->background_objects;
}

// -----------------------------------------------------------------------

LazyArray<GraphicsObject>& GraphicsSystem::foregroundObjects()
{
  return graphics_object_impl_->foreground_objects;
}

// -----------------------------------------------------------------------

void GraphicsSystem::takeSavepointSnapshot()
{
  foregroundObjects().copyTo(graphics_object_impl_->saved_foreground_objects);
  backgroundObjects().copyTo(graphics_object_impl_->saved_background_objects);
}

// -----------------------------------------------------------------------

void GraphicsSystem::clearAllDCs() {
  getDC(0)->fill(RGBAColour::Black());

  for(int i = 1; i < 16; ++i)
    freeDC(i);
}

// -----------------------------------------------------------------------

void GraphicsSystem::renderObjects(RLMachine& machine, std::ostream* tree)
{
  // Render all visible foreground objects
  AllocatedLazyArrayIterator<GraphicsObject> it =
    graphics_object_impl_->foreground_objects.allocated_begin();
  AllocatedLazyArrayIterator<GraphicsObject> end =
    graphics_object_impl_->foreground_objects.allocated_end();
  for(; it != end; ++it)
  {
    const ObjectSettings& settings = getObjectSettings(it.pos());
    if(settings.obj_on_off == 1 && showObject1() == false)
      continue;
    else if(settings.obj_on_off == 2 && showObject2() == false)
      continue;
    else if(settings.weather_on_off && showWeather() == false)
      continue;
    else if(settings.space_key && interfaceHidden())
      continue;

    it->render(machine, it.pos(), tree);
  }
}

// -----------------------------------------------------------------------

boost::shared_ptr<MouseCursor> GraphicsSystem::currentCursor(RLMachine& machine)
{
  if (!use_custom_mouse_cursor_ || !show_curosr_)
    return boost::shared_ptr<MouseCursor>();

  if(use_custom_mouse_cursor_ && !mouse_cursor_)
  {
    MouseCursorCache::iterator it = cursor_cache_.find(cursor_);
    if(it != cursor_cache_.end())
      mouse_cursor_ = it->second;
    else
    {
      boost::shared_ptr<Surface> cursor_surface;
      GameexeInterpretObject cursor_key =
        system().gameexe()("MOUSE_CURSOR", cursor_, "NAME");

      if (cursor_key.exists())
      {
        cursor_surface = loadSurfaceFromFile(machine, cursor_key);
        mouse_cursor_.reset(new MouseCursor(cursor_surface));
        cursor_cache_[cursor_] = mouse_cursor_;
      }
      else
        mouse_cursor_.reset();
    }
  }

  return mouse_cursor_;
}

// -----------------------------------------------------------------------

void GraphicsSystem::mouseMotion(const Point& new_location)
{
  if(use_custom_mouse_cursor_ && show_curosr_)
    markScreenAsDirty(GUT_MOUSE_MOTION);

  cursor_pos_ = new_location;
}

// -----------------------------------------------------------------------

GraphicsObjectData* GraphicsSystem::buildObjOfFile(RLMachine& machine,
                                                   const std::string& filename)
{
  // Get the path to get the file type (which won't be in filename)
  fs::path full_path = findFile(machine, filename);
  string file_str = full_path.file_string();
  if(iends_with(file_str, "g00") || iends_with(file_str, "pdt"))
  {
    return new GraphicsObjectOfFile(machine, filename);
  }
  else if(iends_with(file_str, "anm"))
  {
    return new AnmGraphicsObjectData(machine, filename);
  }
  else
  {
    ostringstream oss;
    oss << "Don't know how to handle object file: \"" << filename << "\"";
    throw rlvm::Exception(oss.str());
  }
}

// -----------------------------------------------------------------------

template<class Archive>
void GraphicsSystem::save(Archive& ar, unsigned int version) const
{
  ar
    & subtitle_
    & graphics_object_settings_->graphics_stack
    & graphics_object_impl_->saved_background_objects
    & graphics_object_impl_->saved_foreground_objects;
}

// -----------------------------------------------------------------------

template<class Archive>
void GraphicsSystem::load(Archive& ar, unsigned int version)
{
  ar
    & subtitle_
    & graphicsStack()
    & graphics_object_impl_->background_objects
    & graphics_object_impl_->foreground_objects;

  // Now alert all subclasses that we've set the subtitle
  setWindowSubtitle(subtitle_,
                    Serialization::g_current_machine->getTextEncoding());
}

// -----------------------------------------------------------------------

template void GraphicsSystem::load<boost::archive::text_iarchive>(
  boost::archive::text_iarchive & ar, unsigned int version);
template void GraphicsSystem::save<boost::archive::text_oarchive>(
  boost::archive::text_oarchive & ar, unsigned int version) const;
