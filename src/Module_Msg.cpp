
// -----------------------------------------------------------------------

/**
 * @file   Module_Msg.cpp
 * @author Elliot Glaysher
 * @date   Sun Oct  1 22:18:39 2006
 * 
 * @brief  Implements many textout related operations.
 */

#include "Module_Msg.hpp"
#include "RLOperation.hpp"
#include "LongOperation.hpp"
#include "RLMachine.hpp"
//#include "GeneralOperations.hpp"

#include <iostream>


using namespace std;

/** 
 * Implements op<0:Msg:17, 0>, fun pause().
 * 
 * 
 * @todo Make this a real implmentation of pause() instead of a
 * debugging tool.
 */
struct Msg_pause : public RLOp_Void_Void {
  /// Long operation
  struct Longop_pause : public LongOperation {
    bool operator()(RLMachine& machine) {
      char x;
      cin.get(x);
      return true;
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
//  addOpcode(100, 0, /* SetFontColour */);
//   addOpcode(101, 0, new Op_SetToIncoming(textSystem.fontSizeInPixels(),
//                                          textSystem));            
//   addOpcode(101, 1, new Op_ReturnValue(textSystem.fontSizeInPixels()));
//  addOpcode(102 ...)
//   addOpcode(103, 0, new Op_SetToTrue(text.fastTextMode(),
//                                      textSystem));
//   addOpcode(104, 0, new Op_SetToFalse(text.fastTextMode(),
//                                       textSystem));
//  addOpcode(105, 0
//  addOpcode(106
//  addOpcode
}
