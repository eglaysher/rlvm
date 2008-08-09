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

#include "MachineBase/RLMachine.hpp"
#include "MachineBase/StackFrame.hpp"
#include "MachineBase/Serialization.hpp"
#include "MachineBase/LongOperation.hpp"

#include "libReallive/archive.h"

#include "Utilities.h"

#include <iostream>

using namespace std;

// -----------------------------------------------------------------------
// StackFrame
// -----------------------------------------------------------------------
StackFrame::StackFrame()
  : scenario_(NULL), ip(), frameType()
{}

// -----------------------------------------------------------------------

StackFrame::StackFrame(libReallive::Scenario const* s,
                       const libReallive::Scenario::const_iterator& i,
                       FrameType t)
  : scenario_(s), ip(i), frameType(t)
{}

// -----------------------------------------------------------------------

StackFrame::StackFrame(libReallive::Scenario const* s,
                       const libReallive::Scenario::const_iterator& i,
                       LongOperation* op)
  : scenario_(s), ip(i), longOp(op), frameType(TYPE_LONGOP)
{}

// -----------------------------------------------------------------------

StackFrame::~StackFrame()
{
}

// -----------------------------------------------------------------------

void StackFrame::setScenario(libReallive::Scenario const* s)
{
  scenario_ = s;
}

// -----------------------------------------------------------------------

std::ostream& operator<<(std::ostream& os, const StackFrame& frame)
{
  os << "{seen=" << frame.scenario()->sceneNumber() << ", offset="
     << distance(frame.scenario()->begin(), frame.ip) << "}";

  return os;
}

// -----------------------------------------------------------------------

template<class Archive>
void StackFrame::save(Archive & ar, unsigned int version) const
{
  int sceneNumber = scenario()->sceneNumber();
  int position = distance(scenario()->begin(), ip);
  ar & sceneNumber & position & frameType;
}

// -----------------------------------------------------------------------

template<class Archive>
void StackFrame::load(Archive & ar, unsigned int version)
{
  int sceneNumber, offset;
  FrameType type;
  ar & sceneNumber & offset & type;

  libReallive::Scenario const* scenario =
    Serialization::g_currentMachine->archive().scenario(sceneNumber);
  if(scenario == NULL)
  {
    ostringstream oss;
    oss << "Unknown SEEN #" << sceneNumber << " in save file!";
    throw rlvm::Exception(oss.str());
  }

  if(offset > distance(scenario->begin(), scenario->end()) || offset < 0)
  {
    ostringstream oss;
    oss << offset << " is an illegal bytecode offset for SEEN #"
        << sceneNumber << " in save file!";
    throw rlvm::Exception(oss.str());
  }

  libReallive::Scenario::const_iterator positionIt = scenario->begin();
  advance(positionIt, offset);

  *this = StackFrame(scenario, positionIt, type);
}

// -----------------------------------------------------------------------

// Explicit instantiations for text archives (since we hide the
// implementation)

template void StackFrame::save<boost::archive::text_oarchive>(
  boost::archive::text_oarchive & ar, unsigned int version) const;

template void StackFrame::load<boost::archive::text_iarchive>(
  boost::archive::text_iarchive & ar, unsigned int version);

