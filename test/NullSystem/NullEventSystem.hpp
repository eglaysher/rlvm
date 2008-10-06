// -*- Mode: C++; tab-width:2; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi:tw=80:et:ts=2:sts=2
//
// -----------------------------------------------------------------------
//
// This file is part of RLVM, a RealLive virtual machine clone.
//
// -----------------------------------------------------------------------
//
// Copyright (C) 2008 Elliot Glaysher
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

#ifndef __NullEventSystem_hpp__
#define __NullEventSystem_hpp__

#include "Systems/Base/EventSystem.hpp"
#include "NullSystem/MockLog.hpp"

#include <boost/shared_ptr.hpp>

// -----------------------------------------------------------------------

/**
 * Provides behaviour for the NullEventSystem.
 */
class EventSystemMockHandler {
public:
  virtual bool shiftPressed() const { return false; }
  virtual bool ctrlPressed() const { return false; }
  virtual unsigned int getTicks() const { return 0; }
};

// -----------------------------------------------------------------------

/**
 * Mock enabled event system. Returned values are controlled by
 * EventSystemMockHandler.
 */
class NullEventSystem : public EventSystem
{
public:
  NullEventSystem(Gameexe& gexe);
  void setMockHandler(const boost::shared_ptr<EventSystemMockHandler>& handler);

  virtual void executeEventSystem(RLMachine& machine);
  virtual bool shiftPressed() const;
  virtual bool ctrlPressed() const;
  virtual unsigned int getTicks() const;
  virtual void wait(unsigned int milliseconds) const;

  // TODO: This needs to be done to get running luaRlvm with a
  // NullEventSystem. Among a lot of other things in this class...
  virtual void injectMouseMovement(const Point& loc) {}
  virtual void injectMouseDown() {}
  virtual void injectMouseUp() {}

private:
  /// Defines test specific behaviour for the NullEventSystem
  boost::shared_ptr<EventSystemMockHandler> event_system_mock_;

  /// Log of which methods have been called.
  mutable MockLog mock_log_;
};

#endif
