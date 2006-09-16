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

#include "Module_Jmp.hpp"
#include "RLOperation.hpp"

#include <cmath>
#include <iostream>

using namespace std;
using namespace Reallive;

/**
 * @defgroup ModuleJmp Module 001, Jmp.
 *
 * Module that describes flow control opcodes in the RealLive virtual
 * machine. This module implements commands such as goto, gosub, ret,
 * jump, et cetera.
 *
 * Most of these opcodes derive from RLOp_SpecialCase and thus this
 * module should not be used as a template for how to implement a
 * RLModule.
 */
//@{

/** 
 * Implement op<0:Jmp:00000, 0>, fun goto().
 * 
 * Jumps to the supplied label in the current scenario.
 */
struct Jmp_goto : public RLOp_SpecialCase {
  void operator()(RLMachine& machine, const CommandElement& f) {
    const GotoElement& gotoElement = dynamic_cast<const GotoElement&>(f);
    const Pointers& pointers = gotoElement.get_pointersRef();

    // Goto the first pointer
    machine.gotoLocation(pointers[0]);
  }
};

// -----------------------------------------------------------------------

/**
 * Implements op<0:Jmp:00001, 0>, fun goto_if(<'condition').
 *
 * Conditional equivalents of goto; goto_if() jumps to @label if the
 * value of condition is non-zero
 */
struct Jmp_goto_if : public RLOp_SpecialCase {
  void operator()(RLMachine& machine, const CommandElement& f) {
    const GotoElement& gotoElement = dynamic_cast<const GotoElement&>(f);
    const char* location = gotoElement.get_param(0).c_str();
      
    auto_ptr<ExpressionPiece> condition(get_expression(location));
      
    if(condition->getIntegerValue(machine)) {
      const Pointers& pointers = gotoElement.get_pointersRef();
      
      // Goto the first pointer
      machine.gotoLocation(pointers[0]);
    }
    else 
      machine.advanceInstructionPointer();
  }
};

// -----------------------------------------------------------------------

/**
 * Implements op<0:Jmp:00002, 0>, fun goto_if(<'condition').
 *
 * Conditional equivalents of goto; goto_if() jumps to @label if the
 * value of condition is non-zero
 */
struct Jmp_goto_unless : public RLOp_SpecialCase {
  void operator()(RLMachine& machine, const CommandElement& f) {
    const GotoElement& gotoElement = dynamic_cast<const GotoElement&>(f);
    const char* location = gotoElement.get_param(0).c_str();
      
    auto_ptr<ExpressionPiece> condition(get_expression(location));
      
    if(!condition->getIntegerValue(machine)) {
      const Pointers& pointers = gotoElement.get_pointersRef();
      machine.gotoLocation(pointers[0]);
    } 
    else 
      machine.advanceInstructionPointer();
  }
};

// -----------------------------------------------------------------------

/** 
 * Implements op<0:Jmp:00003, 0>, fun goto_on(special case).
 * 
 * Table jumps. expr is evaluated, and control passed to the
 * corresponding label in the list, counting from 0. If expr falls
 * outside the valid range, no jump takes place, and execution
 * continues from the next statement instead.
 */
struct Jmp_goto_on : public RLOp_SpecialCase {
  void operator()(RLMachine& machine, const CommandElement& f) {
    const GotoElement& gotoElement = dynamic_cast<const GotoElement&>(f);
    const char* location = gotoElement.get_param(0).c_str();
      
    auto_ptr<ExpressionPiece> condition(get_expression(location));
    int value = condition->getIntegerValue(machine);
    const Pointers& pointers = gotoElement.get_pointersRef();
    if(value >= 0 && value < pointers.size()) 
      machine.gotoLocation(pointers[value]);
    else
      // If the value is not a valid pointer, simply increment.
      machine.advanceInstructionPointer();
  }
};

// -----------------------------------------------------------------------

/** 
 * Implements op<0:Jmp:00004, 0>, fun goto_case (expr) { val1:
 * @label1... } 
 *
 * Conditional table jumps. expr is evaluated, and
 * compared to val1, val2, etc. in turn, and control passes to the
 * label associated with the first matching value.
 */
struct Jmp_goto_case : public RLOp_SpecialCase {
  void operator()(RLMachine& machine, const CommandElement& f) {
    const GotoElement& gotoElement = dynamic_cast<const GotoElement&>(f);
    const char* location = gotoElement.get_param(0).c_str();
      
    auto_ptr<ExpressionPiece> condition(get_expression(location));
    int value = condition->getIntegerValue(machine);
    const Pointers& pointers = gotoElement.get_pointersRef();
    
//    cerr << "[" << pointers.size() << "/" << pointeres.idSize() << "]" << endl;
  }
};

// -----------------------------------------------------------------------

/**
 * @class JmpModule
 *
 * RLModule that contains functions from mod<0:Jmp>.
 */
JmpModule::JmpModule()
  : RLModule("Jmp", 0, 1)
{
  addOpcode(0, 0, new Jmp_goto);
  addOpcode(1, 0, new Jmp_goto_if);
  addOpcode(2, 0, new Jmp_goto_unless);
  addOpcode(3, 0, new Jmp_goto_on);
}

//@}
