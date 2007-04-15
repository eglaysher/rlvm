// This file is part of RLVM, a RealLive virtual machine clone.
//
// -----------------------------------------------------------------------
//
// Copyright (C) 2006 Elliot Glaysher
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

#include "Precompiled.hpp"

// -----------------------------------------------------------------------

/**
 * @file   Module_Refresh.cpp
 * @author Elliot Glaysher
 * @date   Sun Jan 21 13:16:13 2007
 * 
 * @brief  Contains module<1:31>, which contains a single command: refresh()
 */

#include "Modules/Module_Refresh.hpp"

#include "MachineBase/RLOperation.hpp"
#include "MachineBase/RLModule.hpp"
#include "Systems/Base/System.hpp"
#include "Systems/Base/GraphicsSystem.hpp"
#include "Systems/Base/GraphicsObject.hpp"

// -----------------------------------------------------------------------

struct Refresh : public RLOp_Void_Void
{
  void operator()(RLMachine& machine)
  {
    machine.system().graphics().markScreenForRefresh();
  }
};

// -----------------------------------------------------------------------

RefreshModule::RefreshModule()
  : RLModule("Refresh", 1, 31)
{
  addOpcode(0, 0, new Refresh);
}
