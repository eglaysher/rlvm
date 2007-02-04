// This file is part of RLVM, a RealLive virutal machine clone.
//
// -----------------------------------------------------------------------
//
// Copyright (C) 2006 El Riot
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

/**
 * @file   Module_Msg.cpp
 * @author Elliot Glaysher
 * @date   Sun Oct  1 22:18:39 2006
 * 
 * @brief  Implements many textout related operations.
 */

#include "Modules/Module_Msg.hpp"
#include "MachineBase/RLOperation.hpp"
#include "MachineBase/LongOperation.hpp"
#include "MachineBase/RLMachine.hpp"
#include "MachineBase/RLModule.hpp"
//#include "GeneralOperations.hpp"

#include "Systems/Base/System.hpp"
#include "Systems/Base/EventSystem.hpp"

#include <iostream>

using namespace std;

/**
 * @defgroup ModuleMessage The Message and Textout module (mod<0:3>).
 * @ingroup ModulesOpcodes
 *
 * @{
 */


/** 
 * Implements op<0:Msg:17, 0>, fun pause().
 * 
 * @todo This still isn't a real implementation of pause(), needs to
 * handle the window events when we have text windows.
 * @bug Does this work with ctrl()?
 */
struct Msg_pause : public RLOp_Void_Void {
  /// Long operation
  struct Longop_pause : public LongOperation {
    bool operator()(RLMachine& machine) {
      // Check the status of the window.
      return machine.system().event().ctrlPressed();
    }
  };

  void operator()(RLMachine& machine) {
    machine.setLongOperation(new Longop_pause());
  }
};

// -----------------------------------------------------------------------

MsgModule::MsgModule()
  : RLModule("Msg", 0, 003)
{
//  addOpcode(3, 0, /* par */);
//  addOpcode(15, 0, /* spause3 */ );
  addOpcode(17, 0, new Msg_pause);
//  addOpcode(100, 0, );

//   addOpcode(101, 0, new Op_SetToIncoming(textSystem.fontSizeInPixels(),
//                                          textSystem));            
//   addOpcode(101, 1, new Op_ReturnValue(textSystem.fontSizeInPixels()));
//  addOpcode(102 ...)
//  addOpcode(103, 0, new Op_SetToIntConstant(textSystem, 
//                                            &TextSystem::setFastTextMode, 1));
//  addOpcode(104, 0, new Op_SetToIntConstant(textSystem,
//                                            &TextSystem::setFastTextMode, 0));

//   addOpcode(104, 0, new Op_SetToFalse(text.fastTextMode(),
//                                       textSystem));
//  addOpcode(105, 0
//  addOpcode(106
//  addOpcode
}

// @}
