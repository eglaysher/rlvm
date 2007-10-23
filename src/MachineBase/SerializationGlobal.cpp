// -*- Mode: C++; tab-width:2; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi:tw=80:et:ts=2:sts=2
//
// -----------------------------------------------------------------------
//
// This file is part of RLVM, a RealLive virtual machine clone.
//
// -----------------------------------------------------------------------
//
// Copyright (C) 2007 Elliot Glaysher
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

// include headers that implement a archive in simple text format
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/serialization/split_free.hpp>
#include <boost/serialization/vector.hpp>

#include "Utilities.h"
#include "MachineBase/RLMachine.hpp"
#include "MachineBase/Memory.hpp"
#include "Systems/Base/System.hpp"
#include "Systems/Base/GraphicsSystem.hpp"
#include "Systems/Base/EventSystem.hpp"
#include "Systems/Base/TextSystem.hpp"
#include "libReallive/intmemref.h"
#include <fstream>
#include <sstream>
#include <iostream>

using namespace std;
using namespace libReallive;
using namespace boost::archive;
namespace fs = boost::filesystem;

// -----------------------------------------------------------------------

namespace {

/**
 * Pointer to the machine that is having its data
 * serialized. boost::serialization doesn't allow passing something
 * like a closure around, which is frustrating because many pieces of
 * data rely on looking things up on the machine.
 *
 * @warning We're using what is essentially a piece of static data
 *          here; this is a likely location for errors
 */
RLMachine* currentMachine = NULL;

}  // close annonymous namespace

// -----------------------------------------------------------------------

namespace boost {
namespace serialization {

// -----------------------------------------------------------------------
// SystemGlobals
// -----------------------------------------------------------------------
template<class Archive>
void serialize(Archive & ar, SystemGlobals & g, const unsigned int version)
{
  ar & g.m_confirmSaveLoad;
}

// -----------------------------------------------------------------------
// GraphicsSystemGlobals
// -----------------------------------------------------------------------
template<class Archive>
void serialize(Archive& ar, GraphicsSystemGlobals& g, const unsigned int version)
{
  ar & g.showObject1 & g.showObject2 & g.showWeather;
}

// -----------------------------------------------------------------------
// EventSytemGlobals
// -----------------------------------------------------------------------
template<class Archive>
void serialize(Archive& ar, EventSystemGlobals& g, const unsigned int version)
{
  ar & g.generic1 & g.generic2;
}

// -----------------------------------------------------------------------
// TextSystemGlobals
// -----------------------------------------------------------------------
template<class Archive>
void serialize(Archive& ar, TextSystemGlobals& g, const unsigned int version)
{
  ar & g.autoModeBaseTime & g.autoModeCharTime & g.messageSpeed
    & g.windowAttr;
}

// -----------------------------------------------------------------------
// GlobalMemory
// -----------------------------------------------------------------------
template<class Archive>
inline void serialize(Archive & ar, GlobalMemory& memory, unsigned int version)
{
  ar & memory.intG & memory.intZ & memory.strM;
}

// -----------------------------------------------------------------------
// RLMachine
// -----------------------------------------------------------------------
template<class Archive>
inline void serialize(Archive & ar, RLMachine& machine, unsigned int version)
{
  ar & machine.memory().global();
}

} // namespace serialization
} // namespace boost

// -----------------------------------------------------------------------

namespace Serialization {

// -----------------------------------------------------------------------

void saveGlobalMemoryTo(std::ostream& oss, RLMachine& machine)
{
  currentMachine = &machine;

  text_oarchive oa(oss);
  System& sys = machine.system();
  oa << const_cast<const RLMachine&>(machine)
     << const_cast<const SystemGlobals&>(sys.globals())
     << const_cast<const GraphicsSystemGlobals&>(sys.graphics().globals())
     << const_cast<const EventSystemGlobals&>(sys.event().globals())
     << const_cast<const TextSystemGlobals&>(sys.text().globals());

  currentMachine = NULL;
}

// -----------------------------------------------------------------------

void loadGlobalMemoryFrom(std::istream& iss, RLMachine& machine)
{
  currentMachine = &machine;

  text_iarchive ia(iss);
  System& sys = machine.system();
  ia >> machine
     >> sys.globals()
     >> sys.graphics().globals()
     >> sys.event().globals()
     >> sys.text().globals();

  currentMachine = NULL;
}

// -----------------------------------------------------------------------

}
