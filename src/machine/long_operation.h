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
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
//
// -----------------------------------------------------------------------

#ifndef SRC_MACHINE_LONG_OPERATION_H_
#define SRC_MACHINE_LONG_OPERATION_H_

#include <memory>

#include "systems/base/event_listener.h"

class RLMachine;

// A LongOperation is a non-trivial command that requires multiple
// passes through the game loop to complete.
//
// For example, pause(). The pause() RLOperation sets the pause()
// LongOperation, which is executed instead of normal
// interpretation. The pause() LongOperation checks for any input from
// the user (ctrl or mouse click), returning true when it detects it,
// telling the RLMachine to delete the current LongOperation and
// resume normal operations.
//
// Since we are taking over normal event flow, we implement
// EventListener. LongOperations on the top of the callstack receive key/mouse
// events.
class LongOperation : public EventListener {
 public:
  LongOperation();
  virtual ~LongOperation();

  // Executes the current LongOperation. Returns true if the command has
  // completed, and normal interpretation should be resumed, false otherwise.
  virtual bool operator()(RLMachine& machine) = 0;
};

// LongOperator decorator that simply invokes the included
// LongOperation and when that LongOperation finishes, performs an
// arbitrary action.
class PerformAfterLongOperationDecorator : public LongOperation {
 public:
  explicit PerformAfterLongOperationDecorator(LongOperation* in_op);
  virtual ~PerformAfterLongOperationDecorator();

  // Overridden from EventListener:
  // Forward all messages to our held operation
  virtual void MouseMotion(const Point& new_location) override;
  virtual bool MouseButtonStateChanged(MouseButton mouse_button,
                                       bool pressed) override;
  virtual bool KeyStateChanged(KeyCode key_code, bool pressed) override;

  // Overridden from LongOperation:
  virtual bool operator()(RLMachine& machine);

 private:
  // Payload of decorator implemented by subclasses
  virtual void PerformAfterLongOperation(RLMachine& machine) = 0;

  std::unique_ptr<LongOperation> operation_;
};

#endif  // SRC_MACHINE_LONG_OPERATION_H_
