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
#include "Systems/Base/ObjectSettings.hpp"
#include "libReallive/gameexe.h"

#include <vector>
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
	const std::string& key = it->key();
	string s = key.substr(key.find_first_of(".") + 1);
	int objNum = lexical_cast<int>(s);
	if(objNum != 999 && objNum < OBJECTS_IN_A_LAYER)
	{
	  position[objNum] = data.size();
	  data.push_back(ObjectSettings(*it));
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

void GraphicsSystem::setWindowSubtitle(const std::string& utf8encoded)
{
  m_subtitle = utf8encoded;
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

void GraphicsSystem::saveGlobals(Json::Value& system)
{
  Json::Value graphics(Json::objectValue);
  graphics["showObject1"] = showObject1();
  graphics["showObject2"] = showObject2();
  graphics["showWeather"] = showWeather();

  system["graphics"] = graphics;
}

// -----------------------------------------------------------------------

void GraphicsSystem::loadGlobals(const Json::Value& system)
{
  // Right now, noop
  Json::Value textSys = system["graphics"];
  setShowObject1(textSys["showObject1"].asInt());
  setShowObject2(textSys["showObject2"].asInt());
  setShowWeather(textSys["showWeather"].asInt());
}
