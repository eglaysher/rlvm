// -*- Mode: C++; tab-width:2; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi:tw=80:et:ts=2:sts=2
//

#include "Systems/Base/ObjectSettings.hpp"

// -----------------------------------------------------------------------
// ObjectSettings
// -----------------------------------------------------------------------

ObjectSettings::ObjectSettings()
  : layer(0), spaceKey(0), objOnOff(0), timeMod(0), dispSort(0), initMod(0),
	weatherOnOff(0)
{}

// -----------------------------------------------------------------------

ObjectSettings::ObjectSettings(const std::vector<int>& data)
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
