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
 * @file   Module_Debug.cpp
 * @author Elliot Glaysher
 * @date   Sun Mar  4 17:41:03 2007
 * 
 * @brief  Defines debugging functions. (mod<1:255)
 */

#include "Modules/Module_Debug.hpp"
#include "MachineBase/RLOperation.hpp"
#include "Systems/Base/System.hpp"
#include "libReallive/gameexe.h"

#include <iostream>
using namespace std;

/**
 * @defgroup ModuleDebug The Debug Module (mod<1:255)
 * @ingroup ModulesOpcodes 
 *
 * Module that defines runtime debugging operations. The following
 * operations are only executed when #MEMORY is defined in the
 * incoming Gameexe file.
 * 
 * @{
 */

namespace Opcodes {
namespace Debug {

template<typename TYPE>
struct DebugMessage : public RLOp_Void_1< TYPE >
{
  void operator()(RLMachine& machine, typename TYPE::type value)
  {
    if(machine.system().gameexe()("MEMORY").exists())
       cerr << "VALUE: " << value << endl;
  }
};

// -----------------------------------------------------------------------

}
}

DebugModule::DebugModule()
  : RLModule("Debug", 1, 255)
{
  using namespace Opcodes::Debug;

  addOpcode(  10, 0, new DebugMessage<IntConstant_T>);
  addOpcode(  10, 1, new DebugMessage<StrConstant_T>);
}

/// @}
