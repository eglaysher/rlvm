// This file is part of RLVM, a RealLive virtual machine clone.
//
// -----------------------------------------------------------------------
//
// Copyright (C) 2006, 2007 Elliot Glaysher
//  
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
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
#include "Systems/Base/GraphicsStackFrame.hpp"
#include "Systems/Base/ObjectSettings.hpp"
#include "libReallive/gameexe.h"

#include "Modules/Module_Grp.hpp"

#include <vector>
#include <list>
#include <algorithm>
#include <boost/lexical_cast.hpp>

#include "json/value.h"
#include <iostream>

using std::cout;
using std::endl;
using boost::lexical_cast;
using std::fill;
using std::vector;

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
// GraphicsSystem
// -----------------------------------------------------------------------
GraphicsSystem::GraphicsSystem(Gameexe& gameexe) 
  : m_screenUpdateMode(SCREENUPDATEMODE_AUTOMATIC),
	m_displaySubtitle(gameexe("SUBTITLE").to_int(0)),
	m_showObject1(gameexe("INIT_OBJECT1_ONOFF_MOD").to_int(0) ? 0 : 1),
	m_showObject2(gameexe("INIT_OBJECT2_ONOFF_MOD").to_int(0) ? 0 : 1),
    m_showWeather(gameexe("INIT_WEATHER_ONOFF_MOD").to_int(0) ? 0 : 1),
    m_hideInterface(false),
	m_graphicsObjectSettings(new GraphicsObjectSettings(gameexe))
{
}

// -----------------------------------------------------------------------

GraphicsSystem::~GraphicsSystem()
{}

// -----------------------------------------------------------------------

GraphicsStackFrame& GraphicsSystem::addGraphicsStackFrame(const std::string& name)
{
  m_graphicsObjectSettings->graphicsStack.push_back(GraphicsStackFrame(name));
  return m_graphicsObjectSettings->graphicsStack.back();
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
  m_showObject1 = in;
}

// -----------------------------------------------------------------------

void GraphicsSystem::setShowObject2(const int in)
{
  m_showObject2 = in;
}

// -----------------------------------------------------------------------

void GraphicsSystem::setShowWeather(const int in)
{
  m_showWeather = in;
}

// -----------------------------------------------------------------------

void GraphicsSystem::toggleInterfaceHidden()
{
  m_hideInterface = !m_hideInterface;
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
void GraphicsSystem::markScreenAsDirty() { }
void GraphicsSystem::markScreenForRefresh() { }
void GraphicsSystem::beginFrame() { }
void GraphicsSystem::endFrame() { }

// -----------------------------------------------------------------------

boost::shared_ptr<Surface> GraphicsSystem::renderToSurfaceWithBg(
  RLMachine& machine, boost::shared_ptr<Surface> bg) 
{ return boost::shared_ptr<Surface>(); }

// -----------------------------------------------------------------------

void GraphicsSystem::saveGlobals(Json::Value& graphics)
{
  graphics["showObject1"] = showObject1();
  graphics["showObject2"] = showObject2();
  graphics["showWeather"] = showWeather();
}

// -----------------------------------------------------------------------

void GraphicsSystem::loadGlobals(const Json::Value& textSys)
{
  setShowObject1(textSys["showObject1"].asInt());
  setShowObject2(textSys["showObject2"].asInt());
  setShowWeather(textSys["showWeather"].asInt());
}

// -----------------------------------------------------------------------

void GraphicsSystem::saveGameValues(Json::Value& graphicsSys)
{
  Json::Value graphicsValue(Json::arrayValue);
  
  std::vector<GraphicsStackFrame>::iterator it = 
    m_graphicsObjectSettings->graphicsStack.begin();
  std::vector<GraphicsStackFrame>::iterator end = 
    m_graphicsObjectSettings->graphicsStack.end();
  for(; it != end; ++it) 
  { 
    Json::Value frame(Json::objectValue);
    it->serializeTo(frame);
    graphicsValue.push_back(frame);
  }

  graphicsSys["stack"] = graphicsValue;
}

// -----------------------------------------------------------------------

void GraphicsSystem::loadGameValues(RLMachine& machine,
                                    const Json::Value& graphicsSys)
{
  replayGraphicsStack(machine, graphicsSys["stack"]);
}

// -----------------------------------------------------------------------

void GraphicsSystem::reset()
{
  m_defaultGrpName = "";
  m_defaultBgrName = "";
  m_screenUpdateMode = SCREENUPDATEMODE_AUTOMATIC;
  m_subtitle = "";
  m_hideInterface = false;
}
