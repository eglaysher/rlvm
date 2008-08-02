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

#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/serialization/scoped_ptr.hpp>
#include <boost/serialization/vector.hpp>

#include "Systems/Base/GraphicsSystem.hpp"
#include "Systems/Base/GraphicsObject.hpp"
#include "Systems/Base/GraphicsObjectData.hpp"
#include "Systems/Base/GraphicsObjectOfFile.hpp"
#include "Systems/Base/GraphicsStackFrame.hpp"
#include "Systems/Base/ObjectSettings.hpp"
#include "Systems/Base/AnmGraphicsObjectData.hpp"
#include "Systems/Base/ObjectSettings.hpp"
#include "Systems/Base/SystemError.hpp"
#include "Systems/Base/System.hpp"
#include "Systems/Base/MouseCursor.hpp"
#include "libReallive/gameexe.h"

#include "MachineBase/RLMachine.hpp"
#include "MachineBase/Serialization.hpp"
#include "MachineBase/StackFrame.hpp"
#include "Modules/Module_Grp.hpp"
#include "Utilities.h"
#include "LazyArray.hpp"

#include <sstream>
#include <vector>
#include <list>
#include <algorithm>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>
#include <iterator>
#include <iostream>

using std::cerr;
using std::cout;
using std::endl;
using boost::iends_with;
using boost::lexical_cast;
using std::fill;
using std::vector;
using std::ostringstream;

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
  
  for ( std::list<int>::const_iterator intit = object_nums.begin(); intit != object_nums.end(); ++intit )
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
  LazyArray<GraphicsObject> m_foregroundObjects;

  /// Background objects
  LazyArray<GraphicsObject> m_backgroundObjects;

  /// Foreground objects (at the time of the last save)
  LazyArray<GraphicsObject> m_savedForegroundObjects;

  /// Background objects (at the time of the last save)
  LazyArray<GraphicsObject> m_savedBackgroundObjects;
};

// -----------------------------------------------------------------------

GraphicsSystem::GraphicsObjectImpl::GraphicsObjectImpl()
  : m_foregroundObjects(256), m_backgroundObjects(256),
    m_savedForegroundObjects(256), m_savedBackgroundObjects(256)
{}

// -----------------------------------------------------------------------
// GraphicsSystem
// -----------------------------------------------------------------------
GraphicsSystem::GraphicsSystem(System& system, Gameexe& gameexe) 
  : m_screenUpdateMode(SCREENUPDATEMODE_AUTOMATIC),
    m_screenNeedsRefresh(false),
    m_isResponsibleForUpdate(true),
    m_displaySubtitle(gameexe("SUBTITLE").to_int(0)),
    m_hideInterface(false),
    m_globals(gameexe),
    m_graphicsObjectSettings(new GraphicsObjectSettings(gameexe)),
    m_graphicsObjectImpl(new GraphicsObjectImpl),
    m_useCustomMouseCursor(gameexe("MOUSE_CURSOR").exists()),
    m_showCurosr(true),
    m_cursor(gameexe("MOUSE_CURSOR").to_int(0)),
    m_system(system)
{}

// -----------------------------------------------------------------------

GraphicsSystem::~GraphicsSystem()
{}

// -----------------------------------------------------------------------

void GraphicsSystem::setIsResponsibleForUpdate(bool in)
{ 
  m_isResponsibleForUpdate = in; 
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
    m_screenNeedsRefresh = true;
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
  m_screenNeedsRefresh = true;
}

// -----------------------------------------------------------------------

void GraphicsSystem::setScreenUpdateMode(DCScreenUpdateMode u)
{
  m_screenUpdateMode = u; 
}

// -----------------------------------------------------------------------

int GraphicsSystem::useCustomCursor() {
  return m_useCustomMouseCursor && 
    system().gameexe()("MOUSE_CURSOR", m_cursor, "NAME").exists();
}

// -----------------------------------------------------------------------

void GraphicsSystem::setCursor(RLMachine& machine, int cursor)
{
  m_cursor = cursor;
  m_mouseCursor.reset();
}

// -----------------------------------------------------------------------

GraphicsStackFrame& GraphicsSystem::addGraphicsStackFrame(const std::string& name)
{
  m_graphicsObjectSettings->graphicsStack.push_back(GraphicsStackFrame(name));
  return m_graphicsObjectSettings->graphicsStack.back();
}

// -----------------------------------------------------------------------

vector<GraphicsStackFrame>& GraphicsSystem::graphicsStack()
{
  return m_graphicsObjectSettings->graphicsStack;
}

// -----------------------------------------------------------------------

int GraphicsSystem::stackSize() const
{
  return m_graphicsObjectSettings->graphicsStack.size();
}

// -----------------------------------------------------------------------

void GraphicsSystem::clearStack() 
{
  m_graphicsObjectSettings->graphicsStack.clear();
}

// -----------------------------------------------------------------------

void GraphicsSystem::stackPop(int items)
{
  for(int i = 0; i < items; ++i)
    m_graphicsObjectSettings->graphicsStack.pop_back();
}

// -----------------------------------------------------------------------

void GraphicsSystem::replayGraphicsStack(RLMachine& machine)
{
  vector<GraphicsStackFrame> stackToReplay;
  stackToReplay.swap(m_graphicsObjectSettings->graphicsStack);

  // 
  replayGraphicsStackVector(machine, stackToReplay);
}

// -----------------------------------------------------------------------

void GraphicsSystem::setWindowSubtitle(const std::string& cp932str,
                                       int textEncoding)
{
  m_subtitle = cp932str;
}

// -----------------------------------------------------------------------

const std::string& GraphicsSystem::windowSubtitle() const
{
  return m_subtitle;
}

// -----------------------------------------------------------------------

void GraphicsSystem::setShowObject1(const int in)
{
  m_globals.showObject1 = in;
}

// -----------------------------------------------------------------------

void GraphicsSystem::setShowObject2(const int in)
{
  m_globals.showObject2 = in;
}

// -----------------------------------------------------------------------

void GraphicsSystem::setShowWeather(const int in)
{
  m_globals.showWeather = in;
}

// -----------------------------------------------------------------------

void GraphicsSystem::toggleInterfaceHidden()
{
  m_hideInterface = !m_hideInterface;
//  cerr << "Setting interface hidden to " << m_hideInterface << endl;
}

// -----------------------------------------------------------------------

bool GraphicsSystem::interfaceHidden()
{
  return m_hideInterface;
}

// -----------------------------------------------------------------------

ObjectSettings GraphicsSystem::getObjectSettings(const int objNum)
{
  return m_graphicsObjectSettings->getObjectSettingsFor(objNum);
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
  m_defaultGrpName = "";
  m_defaultBgrName = "";
  m_screenUpdateMode = SCREENUPDATEMODE_AUTOMATIC;
  m_subtitle = "";
  m_hideInterface = false;
}

// -----------------------------------------------------------------------

void GraphicsSystem::promoteObjects()
{
  typedef LazyArray<GraphicsObject>::fullIterator FullIterator;

  FullIterator bg = m_graphicsObjectImpl->m_backgroundObjects.full_begin();
  FullIterator bgEnd = m_graphicsObjectImpl->m_backgroundObjects.full_end();
  FullIterator fg = m_graphicsObjectImpl->m_foregroundObjects.full_begin();
  FullIterator fgEnd = m_graphicsObjectImpl->m_foregroundObjects.full_end();
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

  FullIterator bg = m_graphicsObjectImpl->m_backgroundObjects.full_begin();
  FullIterator bgEnd = m_graphicsObjectImpl->m_backgroundObjects.full_end();
  FullIterator fg = m_graphicsObjectImpl->m_foregroundObjects.full_begin();
  FullIterator fgEnd = m_graphicsObjectImpl->m_foregroundObjects.full_end();
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
    return m_graphicsObjectImpl->m_backgroundObjects[objNumber];
  else
    return m_graphicsObjectImpl->m_foregroundObjects[objNumber];
}

// -----------------------------------------------------------------------

void GraphicsSystem::setObject(int layer, int objNumber, GraphicsObject& obj)
{
  if(layer < 0 || layer > 1)
    throw rlvm::Exception("Invalid layer number");

  if(layer == OBJ_BG_LAYER)
    m_graphicsObjectImpl->m_backgroundObjects[objNumber] = obj;
  else
    m_graphicsObjectImpl->m_foregroundObjects[objNumber] = obj;
}

// -----------------------------------------------------------------------

void GraphicsSystem::clearAllObjects()
{
  m_graphicsObjectImpl->m_foregroundObjects.clear();
  m_graphicsObjectImpl->m_backgroundObjects.clear();
}

// -----------------------------------------------------------------------

LazyArray<GraphicsObject>& GraphicsSystem::backgroundObjects() 
{
  return m_graphicsObjectImpl->m_backgroundObjects; 
}

// -----------------------------------------------------------------------

LazyArray<GraphicsObject>& GraphicsSystem::foregroundObjects() 
{
  return m_graphicsObjectImpl->m_foregroundObjects; 
}

// -----------------------------------------------------------------------

void GraphicsSystem::takeSavepointSnapshot()
{
  foregroundObjects().copyTo(m_graphicsObjectImpl->m_savedForegroundObjects);
  backgroundObjects().copyTo(m_graphicsObjectImpl->m_savedBackgroundObjects);
}

// -----------------------------------------------------------------------

void GraphicsSystem::renderObjects(RLMachine& machine)
{
  // Render all visible foreground objects
  AllocatedLazyArrayIterator<GraphicsObject> it = 
    m_graphicsObjectImpl->m_foregroundObjects.allocated_begin();
  AllocatedLazyArrayIterator<GraphicsObject> end = 
    m_graphicsObjectImpl->m_foregroundObjects.allocated_end();
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
  if (!m_useCustomMouseCursor || !m_showCurosr)
    return boost::shared_ptr<MouseCursor>();

  if(m_useCustomMouseCursor && !m_mouseCursor)
  {
    MouseCursorCache::iterator it = m_cursorCache.find(m_cursor);
    if(it != m_cursorCache.end())
      m_mouseCursor = it->second;
    else 
    {
      boost::shared_ptr<Surface> cursorSurface;
      GameexeInterpretObject cursorKey = 
        system().gameexe()("MOUSE_CURSOR", m_cursor, "NAME");

      if (cursorKey.exists())
      {
        string cursorName = cursorKey;
        fs::path cursorPath = findFile(machine, cursorName, IMAGE_FILETYPES);
        cursorSurface = loadSurfaceFromFile(cursorPath);
        m_mouseCursor.reset(new MouseCursor(cursorSurface));
        m_cursorCache[m_cursor] = m_mouseCursor;
      }
      else 
        m_mouseCursor.reset();
    }
  }

  return m_mouseCursor;
}

// -----------------------------------------------------------------------

void GraphicsSystem::mouseMotion(const Point& newLocation)
{
  if(m_useCustomMouseCursor && m_showCurosr)
    markScreenAsDirty(GUT_MOUSE_MOTION);

  m_cursorPos = newLocation;
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

int GraphicsSystem::foregroundAllocated()
{
  AllocatedLazyArrayIterator<GraphicsObject> it = 
	m_graphicsObjectImpl->m_foregroundObjects.allocated_begin();
  AllocatedLazyArrayIterator<GraphicsObject> end = 
	m_graphicsObjectImpl->m_foregroundObjects.allocated_end();
  return std::distance(it, end);
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
    & m_subtitle
    & m_graphicsObjectSettings->graphicsStack
    & m_graphicsObjectImpl->m_savedBackgroundObjects
    & m_graphicsObjectImpl->m_savedForegroundObjects;
}

// -----------------------------------------------------------------------

template<class Archive>
void GraphicsSystem::load(Archive& ar, unsigned int version)
{
  ar
    & m_subtitle
    & graphicsStack() 
    & m_graphicsObjectImpl->m_backgroundObjects
    & m_graphicsObjectImpl->m_foregroundObjects;

  // Now alert all subclasses that we've set the subtitle
  setWindowSubtitle(m_subtitle, Serialization::g_currentMachine->getTextEncoding());
}

// -----------------------------------------------------------------------

template void GraphicsSystem::load<boost::archive::text_iarchive>(
  boost::archive::text_iarchive & ar, unsigned int version);
template void GraphicsSystem::save<boost::archive::text_oarchive>(
  boost::archive::text_oarchive & ar, unsigned int version) const;
