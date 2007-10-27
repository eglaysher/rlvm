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

#ifndef __StackFrame_hpp__
#define __StackFrame_hpp__

#include "libReallive/scenario.h"
#include <boost/shared_ptr.hpp>

class LongOperation;

// -----------------------------------------------------------------------
// Stack Frame
// -----------------------------------------------------------------------
/**
 * Internally used type that represents a stack frame in RLMachine's
 * call stack.
 *
 * StackFrames are added by two bytecode mechanisms: gosubs and
 * farcalls. gosubs move the instruction pointer within one Scenario,
 * while farcalls move the instruction pointer between Scenarios.
 *
 * StackFrames can also be added to represent LongOperations.
 */
struct StackFrame 
{
  /// The scenario in the SEEN file for this stack frame.
  libReallive::Scenario const* scenario;
    
  /// The instruction pointer in the stack frame.
  libReallive::Scenario::const_iterator ip;

  /// Pointer to the owned LongOperation if this is of TYPE_LONGOP.
  boost::shared_ptr<LongOperation> longOp;

  /// Whether this frame has save game information.
  bool saveGameFrame;

  /// The last save point hit (from this stack frame). 
  libReallive::Scenario::const_iterator savePoint;

  /**
   * The function that pushed the current frame onto the
   * stack. Used in error checking.
   */
  enum FrameType {
    TYPE_ROOT,    /**< Added by the Machine's constructor */
    TYPE_GOSUB,   /**< Added by a call by gosub */
    TYPE_FARCALL, /**< Added by a call by farcall */
    TYPE_LONGOP   /**< Added by pushLongOperation() */
  } frameType;

  /**
   * Constructor for normal stack frames added by RealLive code.
   */
  StackFrame(libReallive::Scenario const* s,
             const libReallive::Scenario::const_iterator& i,
             FrameType t);

  /**
   * Constructor for frames that are just LongOperations.
   */
  StackFrame(libReallive::Scenario const* s,
             const libReallive::Scenario::const_iterator& i,
             LongOperation* op);

  ~StackFrame();

  /** 
   * Used during serialization to move the saveGamePointer to the
   * instruction pointer.
   */
  void setSaveGameAsIP();
};

#endif
