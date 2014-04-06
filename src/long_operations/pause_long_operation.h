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

#ifndef SRC_LONG_OPERATIONS_PAUSE_LONG_OPERATION_H_
#define SRC_LONG_OPERATIONS_PAUSE_LONG_OPERATION_H_

#include "machine/long_operation.h"
#include "systems/base/event_listener.h"

// Main pause function. Exported for TextoutLongOperation to abuse.
class PauseLongOperation : public LongOperation {
 public:
  explicit PauseLongOperation(RLMachine& machine);
  virtual ~PauseLongOperation();

  // Overridden from EventListener:
  virtual void MouseMotion(const Point& new_location) override;
  virtual bool MouseButtonStateChanged(MouseButton mouse_button,
                                       bool pressed) override;
  virtual bool KeyStateChanged(KeyCode key_code, bool pressed) override;

  // Overridden from LongOperation:
  virtual bool operator()(RLMachine& machine);

 private:
  // Has this pause timed out?
  bool AutomodeTimerFired();

  RLMachine& machine_;

  bool is_done_;

  // Used in automode:
  // How long after start_time_ to automatically break out of this
  // Longoperation if auto mode is enabled
  unsigned int automode_time_;

  // How long it's been since the last time we've added time to |total_time_|.
  unsigned int time_at_last_pass_;

  // When this hits |automode_time_|, we fire.
  unsigned int total_time_;
};

// -----------------------------------------------------------------------
// NewPageAfterLongop
// -----------------------------------------------------------------------
class NewPageAfterLongop : public PerformAfterLongOperationDecorator {
 public:
  explicit NewPageAfterLongop(LongOperation* inOp);
  virtual ~NewPageAfterLongop();

 private:
  virtual void PerformAfterLongOperation(RLMachine& machine) override;
};

// -----------------------------------------------------------------------
// NewPageOnAllAfterLongop
// -----------------------------------------------------------------------
class NewPageOnAllAfterLongop : public PerformAfterLongOperationDecorator {
 public:
  explicit NewPageOnAllAfterLongop(LongOperation* inOp);
  virtual ~NewPageOnAllAfterLongop();

 private:
  virtual void PerformAfterLongOperation(RLMachine& machine) override;
};

// -----------------------------------------------------------------------
// NewParagraphAfterLongop
// -----------------------------------------------------------------------
class NewParagraphAfterLongop : public PerformAfterLongOperationDecorator {
 public:
  explicit NewParagraphAfterLongop(LongOperation* inOp);
  virtual ~NewParagraphAfterLongop();

 private:
  virtual void PerformAfterLongOperation(RLMachine& machine) override;
};

#endif  // SRC_LONG_OPERATIONS_PAUSE_LONG_OPERATION_H_
