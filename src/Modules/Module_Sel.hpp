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

#ifndef __Module_Sel_hpp__
#define __Module_Sel_hpp__

#include "MachineBase/RLModule.hpp"
#include "MachineBase/LongOperation.hpp"
#include "Systems/Base/EventHandler.hpp"

#include <vector>
#include <string>

class TextWindow;

// -----------------------------------------------------------------------

namespace libReallive {
class SelectElement;
}

// -----------------------------------------------------------------------

class Sel_LongOperation : public LongOperation, public EventHandler
{
private:
  RLMachine& m_machine;

  std::vector<std::string> options;

  TextWindow& textWindow;

  int m_returnValue;

public:
  Sel_LongOperation(RLMachine& machine,
                    const libReallive::SelectElement& commandElement);
  ~Sel_LongOperation();

  void selected(int num);

  // ----------------------------------------------- [ NiceLongOperation ]
  virtual bool operator()(RLMachine& machine);

  virtual void mouseMotion(const Point&);
  virtual void mouseButtonStateChanged(MouseButton mouseButton, bool pressed);
};

// -----------------------------------------------------------------------

/**
 * Contains functions for mod<0:2>, Sel.
 */
class SelModule : public RLModule {
public:
  SelModule();
};


#endif
