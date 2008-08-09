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
#include "Systems/Base/System.hpp"
#include "Systems/Base/SystemError.hpp"
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

  std::vector<GraphicsStackFrame> graphicsStack;

  GraphicsObjectSettings(Gameexe& gameexe);

  const ObjectSettings& getObjectSettingsFor(int objNum);
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
      int objNumFirst = lexical_cast<int>(s.substr(0, poscolon));
      int objNumLast = lexical_cast<int>(s.substr(poscolon + 1));
      while ( objNumFirst <= objNumLast )
      {
        object_nums.push_back(objNumFirst++);
      }
    }
    else
    {
      object_nums.push_back(lexical_cast<int>(s));
    }

    for( std::list<int>::const_iterator intit = object_nums.begin();
         intit != object_nums.end(); ++intit )
    {
      int objNum = *intit;
      if(objNum != 999 && objNum < OBJECTS_IN_A_LAYER)
      {
        position[objNum] = data.size();
        data.push_back(ObjectSettings(*it));
      }
    }
  }
}

// -----------------------------------------------------------------------

const ObjectSettings& GraphicsSystem::GraphicsObjectSettings::getObjectSettingsFor(
  int objNum)
{
  return data[position[objNum]];
}

// -----------------------------------------------------------------------

// -----------------------------------------------------------------------
// GraphicsSystemGlobals
// -----------------------------------------------------------------------
GraphicsSystemGlobals::GraphicsSystemGlobals()
  : showObject1(false), showObject2(false), showWeather(false)
{}

GraphicsSystemGlobals::GraphicsSystemGlobals(Gameexe& gameexe)
  :	showObject1(gameexe("INIT_OBJECT1_ONOFF_MOD").to_int(0) ? 0 : 1),
    showObject2(gameexe("INIT_OBJECT2_ONOFF_MOD").to_int(0) ? 0 : 1),
    showWeather(gameexe("INIT_WEATHER_ONOFF_MOD").to_int(0) ? 0 : 1)
{}

// -----------------------------------------------------------------------
// GraphicsObjectImpl
// -----------------------------------------------------------------------
struct GraphicsSystem::GraphicsObjectImpl
{
  GraphicsObjectImpl();

  /// Foreground objects
  LazyArray<GraphicsObject> foreground_objects_;

  /// Background objects
  LazyArray<GraphicsObject> background_objects_;

  /// Foreground objects (at the time of the last save)
  LazyArray<GraphicsObject> saved_foreground_objects_;

  /// Background objects (at the time of the last save)
  LazyArray<GraphicsObject> saved_background_objects_;
};

// -----------------------------------------------------------------------

GraphicsSystem::GraphicsObjectImpl::GraphicsObjectImpl()
  : foreground_objects_(256), background_objects_(256),
    saved_foreground_objects_(256), saved_background_objects_(256)
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
    cg_table_(new CGMTable(gameexe)),
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
  graphics_object_settings_->graphicsStack.push_back(GraphicsStackFrame(name));
  return graphics_object_settings_->graphicsStack.back();
}

// -----------------------------------------------------------------------

vector<GraphicsStackFrame>& GraphicsSystem::graphicsStack()
{
  return graphics_object_settings_->graphicsStack;
}

// -----------------------------------------------------------------------

int GraphicsSystem::stackSize() const
{
  return graphics_object_settings_->graphicsStack.size();
}

// -----------------------------------------------------------------------

void GraphicsSystem::clearStack()
{
  graphics_object_settings_->graphicsStack.clear();
}

// -----------------------------------------------------------------------

void GraphicsSystem::stackPop(int items)
{
  for(int i = 0; i < items; ++i)
    graphics_object_settings_->graphicsStack.pop_back();
}

// -----------------------------------------------------------------------

void GraphicsSystem::replayGraphicsStack(RLMachine& machine)
{
  vector<GraphicsStackFrame> stackToReplay;
  stackToReplay.swap(graphics_object_settings_->graphicsStack);

  //
  replayGraphicsStackVector(machine, stackToReplay);
}

// -----------------------------------------------------------------------

void GraphicsSystem::setWindowSubtitle(const std::string& cp932str,
                                       int textEncoding)
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
  globals_.showObject1 = in;
}

// -----------------------------------------------------------------------

void GraphicsSystem::setShowObject2(const int in)
{
  globals_.showObject2 = in;
}

// -----------------------------------------------------------------------

void GraphicsSystem::setShowWeather(const int in)
{
  globals_.showWeather = in;
}

// -----------------------------------------------------------------------

void GraphicsSystem::toggleInterfaceHidden()
{
  hide_interface_ = !hide_interface_;
//  cerr << "Setting interface hidden to " << hide_interface_ << endl;
}

// -----------------------------------------------------------------------

bool GraphicsSystem::interfaceHidden()
{
  return hide_interface_;
}

// -----------------------------------------------------------------------

ObjectSettings GraphicsSystem::getObjectSettings(const int objNum)
{
  return graphics_object_settings_->getObjectSettingsFor(objNum);
}

// -----------------------------------------------------------------------

// Default implementations for some functions (which probably have
// default implementations because I'm lazy, and these really should
// be pure virtual)
void GraphicsSystem::beginFrame() { }
void GraphicsSystem::endFrame(RLMachine& machine) { }

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
  typedef LazyArray<GraphicsObject>::fullIterator FullIterator;

  FullIterator bg = graphics_object_impl_->background_objects_.full_begin();
  FullIterator bgEnd = graphics_object_impl_->background_objects_.full_end();
  FullIterator fg = graphics_object_impl_->foreground_objects_.full_begin();
  FullIterator fgEnd = graphics_object_impl_->foreground_objects_.full_end();
  for(; bg != bgEnd && fg != fgEnd; bg++, fg++)
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
  typedef LazyArray<GraphicsObject>::fullIterator FullIterator;

  FullIterator bg = graphics_object_impl_->background_objects_.full_begin();
  FullIterator bgEnd = graphics_object_impl_->background_objects_.full_end();
  FullIterator fg = graphics_object_impl_->foreground_objects_.full_begin();
  FullIterator fgEnd = graphics_object_impl_->foreground_objects_.full_end();
  for(; bg != bgEnd && fg != fgEnd; bg++, fg++)
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

GraphicsObject& GraphicsSystem::getObject(int layer, int objNumber)
{
  if(layer < 0 || layer > 1)
    throw rlvm::Exception("Invalid layer number");

  if(layer == OBJ_BG_LAYER)
    return graphics_object_impl_->background_objects_[objNumber];
  else
    return graphics_object_impl_->foreground_objects_[objNumber];
}

// -----------------------------------------------------------------------

void GraphicsSystem::setObject(int layer, int objNumber, GraphicsObject& obj)
{
  if(layer < 0 || layer > 1)
    throw rlvm::Exception("Invalid layer number");

  if(layer == OBJ_BG_LAYER)
    graphics_object_impl_->background_objects_[objNumber] = obj;
  else
    graphics_object_impl_->foreground_objects_[objNumber] = obj;
}

// -----------------------------------------------------------------------

void GraphicsSystem::clearAllObjects()
{
  graphics_object_impl_->foreground_objects_.clear();
  graphics_object_impl_->background_objects_.clear();
}

// -----------------------------------------------------------------------

LazyArray<GraphicsObject>& GraphicsSystem::backgroundObjects()
{
  return graphics_object_impl_->background_objects_;
}

// -----------------------------------------------------------------------

LazyArray<GraphicsObject>& GraphicsSystem::foregroundObjects()
{
  return graphics_object_impl_->foreground_objects_;
}

// -----------------------------------------------------------------------

void GraphicsSystem::takeSavepointSnapshot()
{
  foregroundObjects().copyTo(graphics_object_impl_->saved_foreground_objects_);
  backgroundObjects().copyTo(graphics_object_impl_->saved_background_objects_);
}

// -----------------------------------------------------------------------

void GraphicsSystem::renderObjects(RLMachine& machine)
{
  // Render all visible foreground objects
  AllocatedLazyArrayIterator<GraphicsObject> it =
    graphics_object_impl_->foreground_objects_.allocated_begin();
  AllocatedLazyArrayIterator<GraphicsObject> end =
    graphics_object_impl_->foreground_objects_.allocated_end();
  for(; it != end; ++it)
  {
    const ObjectSettings& settings = getObjectSettings(it.pos());
    if(settings.objOnOff == 1 && showObject1() == false)
      continue;
    else if(settings.objOnOff == 2 && showObject2() == false)
      continue;
    else if(settings.weatherOnOff && showWeather() == false)
      continue;
    else if(settings.spaceKey && interfaceHidden())
      continue;

    it->render(machine);
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
      boost::shared_ptr<Surface> cursorSurface;
      GameexeInterpretObject cursorKey =
        system().gameexe()("MOUSE_CURSOR", cursor_, "NAME");

      if (cursorKey.exists())
      {
        string cursorName = cursorKey;
        fs::path cursorPath = findFile(machine, cursorName, IMAGE_FILETYPES);
        cursorSurface = loadSurfaceFromFile(cursorPath);
        mouse_cursor_.reset(new MouseCursor(cursorSurface));
        cursor_cache_[cursor_] = mouse_cursor_;
      }
      else
        mouse_cursor_.reset();
    }
  }

  return mouse_cursor_;
}

// -----------------------------------------------------------------------

void GraphicsSystem::mouseMotion(const Point& newLocation)
{
  if(use_custom_mouse_cursor_ && show_curosr_)
    markScreenAsDirty(GUT_MOUSE_MOTION);

  cursor_pos_ = newLocation;
}

// -----------------------------------------------------------------------

GraphicsObjectData* GraphicsSystem::buildObjOfFile(RLMachine& machine,
                                                   const std::string& filename)
{
  // Get the path to get the file type (which won't be in filename)
  fs::path fullPath = findFile(machine, filename);
  string fileStr = fullPath.file_string();
  if(iends_with(fileStr, "g00") || iends_with(fileStr, "pdt"))
  {
    return new GraphicsObjectOfFile(machine, filename);
  }
  else if(iends_with(fileStr, "anm"))
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

Rect GraphicsSystem::screenRect() const
{
  return Rect(Point(0, 0), screenSize());
}

// -----------------------------------------------------------------------

template<class Archive>
void GraphicsSystem::save(Archive& ar, unsigned int version) const
{
  ar
    & subtitle_
    & graphics_object_settings_->graphicsStack
    & graphics_object_impl_->saved_background_objects_
    & graphics_object_impl_->saved_foreground_objects_;
}

// -----------------------------------------------------------------------

template<class Archive>
void GraphicsSystem::load(Archive& ar, unsigned int version)
{
  ar
    & subtitle_
    & graphicsStack()
    & graphics_object_impl_->background_objects_
    & graphics_object_impl_->foreground_objects_;

  // Now alert all subclasses that we've set the subtitle
  setWindowSubtitle(subtitle_,
                    Serialization::g_currentMachine->getTextEncoding());
}

// -----------------------------------------------------------------------

template void GraphicsSystem::load<boost::archive::text_iarchive>(
  boost::archive::text_iarchive & ar, unsigned int version);
template void GraphicsSystem::save<boost::archive::text_oarchive>(
  boost::archive::text_oarchive & ar, unsigned int version) const;
