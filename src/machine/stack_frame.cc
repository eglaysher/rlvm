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
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
//
// -----------------------------------------------------------------------

#include <boost/archive/text_iarchive.hpp>  // NOLINT
#include <boost/archive/text_oarchive.hpp>  // NOLINT

#include "machine/stack_frame.h"

#include <boost/serialization/vector.hpp>
#include <typeinfo>

#include "libreallive/archive.h"
#include "machine/long_operation.h"
#include "machine/rlmachine.h"
#include "machine/serialization.h"
#include "utilities/exception.h"

// -----------------------------------------------------------------------
// StackFrame
// -----------------------------------------------------------------------
StackFrame::StackFrame() : scenario(NULL), ip(), frame_type() {
  memset(intL, 0, sizeof(intL));
}

StackFrame::StackFrame(libreallive::Scenario const* s,
                       const libreallive::Scenario::const_iterator& i,
                       FrameType t)
    : scenario(s), ip(i), frame_type(t) {
  memset(intL, 0, sizeof(intL));
}

StackFrame::StackFrame(libreallive::Scenario const* s,
                       const libreallive::Scenario::const_iterator& i,
                       LongOperation* op)
    : scenario(s), ip(i), long_op(op), frame_type(TYPE_LONGOP) {
  memset(intL, 0, sizeof(intL));
}

StackFrame::~StackFrame() {}

std::ostream& operator<<(std::ostream& os, const StackFrame& frame) {
  os << "{seen=" << frame.scenario->scene_number()
     << ", offset=" << distance(frame.scenario->begin(), frame.ip);

  if (frame.long_op)
    os << " [LONG OP=" << typeid(*frame.long_op).name() << "]";

  os << "}";

  return os;
}

template <class Archive>
void StackFrame::save(Archive& ar, unsigned int version) const {
  int scene_number = scenario->scene_number();
  int position = distance(scenario->begin(), ip);
  ar& scene_number& position& frame_type& intL& strK;
}

template <class Archive>
void StackFrame::load(Archive& ar, unsigned int version) {
  int scene_number, offset;
  FrameType type;
  ar& scene_number& offset& type;

  libreallive::Scenario const* scenario =
      Serialization::g_current_machine->archive().GetScenario(scene_number);
  if (scenario == NULL) {
    std::ostringstream oss;
    oss << "Unknown SEEN #" << scene_number << " in save file!";
    throw rlvm::Exception(oss.str());
  }

  if (offset > distance(scenario->begin(), scenario->end()) || offset < 0) {
    std::ostringstream oss;
    oss << offset << " is an illegal bytecode offset for SEEN #" << scene_number
        << " in save file!";
    throw rlvm::Exception(oss.str());
  }

  libreallive::Scenario::const_iterator position_it = scenario->begin();
  advance(position_it, offset);

  *this = StackFrame(scenario, position_it, type);

  if (version >= 1) {
    ar& intL;
  }

  if (version == 1) {
    std::string old_strk[3];
    ar & old_strk;

    strK.resize(3);
    strK[0] = old_strk[0];
    strK[1] = old_strk[1];
    strK[2] = old_strk[2];
  } else if (version > 1) {
    ar & strK;
  }
}

// -----------------------------------------------------------------------

// Explicit instantiations for text archives (since we hide the
// implementation)

template void StackFrame::save<boost::archive::text_oarchive>(
    boost::archive::text_oarchive& ar,
    unsigned int version) const;

template void StackFrame::load<boost::archive::text_iarchive>(
    boost::archive::text_iarchive& ar,
    unsigned int version);
