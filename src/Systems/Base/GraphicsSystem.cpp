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

#include "GraphicsSystem.hpp"
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

namespace {

struct GraphicsData
{
  GraphicsData();
  GraphicsData(const vector<int>& data);

  int layer;
  int spaceKey;
  int objOnOff;
  int timeMod;
  int dispSort;
  int initMod;
  int weatherOnOff;
};

// -----------------------------------------------------------------------

GraphicsData::GraphicsData()
  : layer(0), spaceKey(0), objOnOff(0), timeMod(0), dispSort(0), initMod(0),
	weatherOnOff(0)
{}

// -----------------------------------------------------------------------

GraphicsData::GraphicsData(const vector<int>& data)
{
  if(data.size() > 0)
	layer = data[0];
  if(data.size() > 1)
	spaceKey = data[1];
  if(data.size() > 2)
	objOnOff = data[2];
  if(data.size() > 3)
	timeMod = data[3];
  if(data.size() > 4)
	dispSort = data[4];
  if(data.size() > 5)
	initMod = data[5];
  if(data.size() > 6)
	weatherOnOff = data[6];
}

};

// -----------------------------------------------------------------------
// GraphicsSystem::GraphicsObjectSettings
// -----------------------------------------------------------------------
/// Impl object
struct GraphicsSystem::GraphicsObjectSettings
{
  /// Each is a valid index into data, refering to 
  unsigned char position[OBJECTS_IN_A_LAYER];

  std::vector<GraphicsData> data;

  GraphicsObjectSettings(Gameexe& gameexe);

  const GraphicsData& getGraphicsDataFor(int objNum);
};

// -----------------------------------------------------------------------

GraphicsSystem::GraphicsObjectSettings::GraphicsObjectSettings(
  Gameexe& gameexe)
{
  // First we populate everything with the special value
  fill(position, position + OBJECTS_IN_A_LAYER, 0);
  if(gameexe.exists("OBJECT.999"))
	data.push_back(GraphicsData(gameexe("OBJECT.999")));
  else
	data.push_back(GraphicsData());

  // Read the #OBJECT.xxx entries from the Gameexe
  GameexeFilteringIterator it = gameexe.filtering_begin("OBJECT.");
  GameexeFilteringIterator end = gameexe.filtering_end();
  for(; it != end; ++it)
  {
	const std::string& key = it->key();
	string s = key.substr(key.find_first_of(".") + 1);
	cout << "S: " << s << endl;
	int objNum = lexical_cast<int>(s);
	if(objNum != 999 && objNum < OBJECTS_IN_A_LAYER)
	{
	  position[objNum] = data.size();
	  data.push_back(GraphicsData(*it));
	}	
  }
}

// -----------------------------------------------------------------------

const GraphicsData& GraphicsSystem::GraphicsObjectSettings::getGraphicsDataFor(
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
	m_showObject1(gameexe("INIT_OBJECT1_ONOFF_MOD").to_int(0)),
	m_showObject2(gameexe("INIT_OBJECT2_ONOFF_MOD").to_int(0)),
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

int GraphicsSystem::objectIsPartOfWhichShowObject(const int objNum)
{
  return m_graphicsObjectSettings->getGraphicsDataFor(objNum).objOnOff;
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

  system["graphics"] = graphics;
}

// -----------------------------------------------------------------------

void GraphicsSystem::loadGlobals(const Json::Value& system)
{
  // Right now, noop
  Json::Value textSys = system["graphics"];
  setShowObject1(textSys["showObject1"].asInt());
  setShowObject2(textSys["showObject2"].asInt());
}
