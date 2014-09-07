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

#ifndef SRC_MACHINE_STACK_FRAME_H_
#define SRC_MACHINE_STACK_FRAME_H_

#include <boost/serialization/split_member.hpp>
#include <boost/serialization/version.hpp>

#include <memory>
#include <string>

#include "libreallive/scenario.h"

class LongOperation;

// -----------------------------------------------------------------------
// Stack Frame
// -----------------------------------------------------------------------

// Internally used type that represents a stack frame in RLMachine's
// call stack.
//
// StackFrames are added by two bytecode mechanisms: gosubs and
// farcalls. gosubs move the instruction pointer within one Scenario,
// while farcalls move the instruction pointer between Scenarios.
//
// StackFrames can also be added to represent LongOperations.
struct StackFrame {
  // The scenario in the SEEN file for this stack frame.
  libreallive::Scenario const* scenario;

  // The instruction pointer in the stack frame.
  libreallive::Scenario::const_iterator ip;

  // Pointer to the owned LongOperation if this is of TYPE_LONGOP.
  std::shared_ptr<LongOperation> long_op;

  // Parameter passing integer bank
  int intL[40];

  // Parameter passing string bank. I have no idea how large this should
  // actually be, so accept any number and grow to fit.
  std::vector<std::string> strK;

  // The function that pushed the current frame onto the
  // stack. Used in error checking.
  enum FrameType {
    TYPE_ROOT,     // Added by the Machine's constructor
    TYPE_GOSUB,    // Added by a call by gosub
    TYPE_FARCALL,  // Added by a call by farcall
    TYPE_LONGOP    // Added by pushLongOperation()
  } frame_type;

  // Default constructor. Only used during serialization.
  StackFrame();

  // Constructor for normal stack frames added by RealLive code.
  StackFrame(libreallive::Scenario const* s,
             const libreallive::Scenario::const_iterator& i,
             FrameType t);

  // Constructor for frames that are just LongOperations.
  StackFrame(libreallive::Scenario const* s,
             const libreallive::Scenario::const_iterator& i,
             LongOperation* op);

  ~StackFrame();

  template <class Archive>
  void save(Archive& ar, const unsigned int file_version) const;

  template <class Archive>
  void load(Archive& ar, const unsigned int file_version);

  BOOST_SERIALIZATION_SPLIT_MEMBER()
};

BOOST_CLASS_VERSION(StackFrame, 2)

std::ostream& operator<<(std::ostream& os, const StackFrame& frame);

#endif  // SRC_MACHINE_STACK_FRAME_H_
