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

/**
 * @file   Module_Jmp.cpp
 * @author Elliot Glaysher
 * @date   Fri Sep 22 20:10:16 2006
 * 
 * @brief  Definitions for flow control opcodes.
 * 
 * @todo Consider cleaning up the code here; generalize the repeated
 * code and casting into a set of namespace local functions. We do a
 * little bit of this with the *_case functors and evaluateCase
 * function, but we could be doing so much more.
 */

#include "Module_Jmp.hpp"
#include "RLOperation.hpp"

#include <cmath>
#include <iostream>
#include <sstream>
//#include <typeinfo>

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

namespace {

/** 
 * Finds which case should be used in the *_case functions.
 * 
 * @param machine RLMachine reference
 * @param f       Bytecode element being evalutated
 * @return Which pointer we should follow.
 * @exception Error Throws on malformed bytecode
 */
int evaluateCase(RLMachine& machine, const CommandElement& f) {
  const GotoCaseElement& gotoElement = 
    dynamic_cast<const GotoCaseElement&>(f);
  const char* location = gotoElement.get_param(0).c_str();
      
  auto_ptr<ExpressionPiece> condition(get_expression(location));
  int value = condition->getIntegerValue(machine);
  const Pointers& pointers = gotoElement.get_pointersRef();

  // Walk linearly through the output cases, executing the first
  // match against value.
  int cases = gotoElement.case_count();
  for(int i = 0; i < cases; ++i) 
  {
    string caseUnparsed = gotoElement.get_case(i);
      
    // Check for bytecode wellformedness. All cases should be
    // surrounded by parens
    if(caseUnparsed[0] != '(' || 
       caseUnparsed[caseUnparsed.size() - 1] != ')')
      throw Error("Malformed bytecode in goto_case statment");

    // In the case of an empty set of parens, always accept. It is
    // the bytecode representation for the default case.
    if(caseUnparsed == "()") {
      return i;
    }

    // Strip the parens for parsing
    caseUnparsed = caseUnparsed.substr(1, caseUnparsed.size() - 2);

    // Parse this expression, and goto the corresponding label if
    // it's equal to the value we're searching for
    const char* e = (const char*)caseUnparsed.c_str();
    auto_ptr<ExpressionPiece> output(get_expression(e));
    if(output->getIntegerValue(machine) == value)
    {
      return i;
    }
  }

  throw Error("Malformed bytecode: no default case");
}

}

// -----------------------------------------------------------------------

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
      
    if(condition->getIntegerValue(machine)) 
    {
      const Pointers& pointers = gotoElement.get_pointersRef();
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
      
    if(!condition->getIntegerValue(machine)) 
    {
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
    const GotoOnElement& gotoElement = dynamic_cast<const GotoOnElement&>(f);
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
    const GotoCaseElement& gotoElement = 
      dynamic_cast<const GotoCaseElement&>(f);
    const Pointers& pointers = gotoElement.get_pointersRef();
    
    int i = evaluateCase(machine, f);
    machine.gotoLocation(pointers[i]);
  }
};

// -----------------------------------------------------------------------

/** 
 * Implement op<0:Jmp:00005, 0>, fun gosub().
 * 
 * Pushes the current location onto the call stack, then jumps to the
 * label @label in the current scenario.
 */
struct Jmp_gosub : public RLOp_SpecialCase {
  void operator()(RLMachine& machine, const CommandElement& f) {
    const GotoElement& gotoElement = dynamic_cast<const GotoElement&>(f);
    const Pointers& pointers = gotoElement.get_pointersRef();
    machine.gosub(pointers[0]);
  } 
};

// -----------------------------------------------------------------------

/** 
 * Implement op<0:Jmp:00006, 0>, fun gosub_if().
 * 
 * Pushes the current location onto the call stack, then jumps to the
 * label @label in the current scenario, if the passed in condition is
 * true.
 */
struct Jmp_gosub_if : public RLOp_SpecialCase {
  void operator()(RLMachine& machine, const CommandElement& f) {
    const GotoElement& gotoElement = dynamic_cast<const GotoElement&>(f);
    const char* location = gotoElement.get_param(0).c_str();
      
    auto_ptr<ExpressionPiece> condition(get_expression(location));
      
    if(condition->getIntegerValue(machine)) 
    {
      const Pointers& pointers = gotoElement.get_pointersRef();
      machine.gosub(pointers[0]);
    }
    else 
      machine.advanceInstructionPointer();
  } 
};

// -----------------------------------------------------------------------

/** 
 * Implement op<0:Jmp:00007, 0>, fun gosub_unless().
 * 
 * Pushes the current location onto the call stack, then jumps to the
 * label @label in the current scenario, if the passed in condition is
 * false.
 */
struct Jmp_gosub_unless : public RLOp_SpecialCase {
  void operator()(RLMachine& machine, const CommandElement& f) {
    const GotoElement& gotoElement = dynamic_cast<const GotoElement&>(f);
    const char* location = gotoElement.get_param(0).c_str();
      
    auto_ptr<ExpressionPiece> condition(get_expression(location));
      
    if(!condition->getIntegerValue(machine)) 
    {
      const Pointers& pointers = gotoElement.get_pointersRef();
      machine.gosub(pointers[0]);
    }
    else 
      machine.advanceInstructionPointer();
  } 
};

// -----------------------------------------------------------------------

/** 
 * Implements op<0:Jmp:00008, 0>, fun gosub_on(special case).
 * 
 * Table gosub. expr is evaluated, and control passed to the
 * corresponding label in the list, counting from 0. If expr falls
 * outside the valid range, no gosub takes place, and execution
 * continues from the next statement instead.
 */
struct Jmp_gosub_on : public RLOp_SpecialCase {
  void operator()(RLMachine& machine, const CommandElement& f) {
    const GotoOnElement& gotoElement = dynamic_cast<const GotoOnElement&>(f);
    const char* location = gotoElement.get_param(0).c_str();
      
    auto_ptr<ExpressionPiece> condition(get_expression(location));
    int value = condition->getIntegerValue(machine);
    const Pointers& pointers = gotoElement.get_pointersRef();
    if(value >= 0 && value < pointers.size()) 
      machine.gosub(pointers[value]);
    else
      // If the value is not a valid pointer, simply increment.
      machine.advanceInstructionPointer();
  }
};

// -----------------------------------------------------------------------

/** 
 * Implements op<0:Jmp:00009, 0>, fun gosub_case (expr) { val1:
 * @label1... } 
 *
 * Conditional table gosub. expr is evaluated, and
 * compared to val1, val2, etc. in turn, and control passes to the
 * label associated with the first matching value.
 */
struct Jmp_gosub_case : public RLOp_SpecialCase {
  void operator()(RLMachine& machine, const CommandElement& f) {
    const GotoCaseElement& gotoElement = 
      dynamic_cast<const GotoCaseElement&>(f);
    const Pointers& pointers = gotoElement.get_pointersRef();
    
    int i = evaluateCase(machine, f);
    machine.gosub(pointers[i]);
  }
};

// -----------------------------------------------------------------------

/** 
 * Implement op<0:Jmp:00010, 0>, fun ret().
 * 
 * Returns from the current stack frame if it was a farcall.
 *
 * @note This functor MUST increment the instruction pointer, since
 * the instruction pointer at this stack frame is still pointing to
 * the gosub that created the new frame.
 */
struct Jmp_ret : public RLOp_Void_Void {
  void operator()(RLMachine& machine) {
    machine.returnFromGosub();
  }
};

// -----------------------------------------------------------------------

/** 
 * Implement op<0:Jmp:00011, 0>, fun jump(intC).
 * 
 * Jumps the instruction pointer to the begining of the scenario
 * #scenario.
 */
struct Jmp_jump_0 : public RLOp_Void_1< IntConstant_T > {
  virtual bool advanceInstructionPointer() { return false; }

  void operator()(RLMachine& machine, int scenario) {
    machine.jump(scenario);
  }
};

// -----------------------------------------------------------------------

/** 
 * Implement op<0:Jmp:00011, 1>, fun jump(intC, intC).
 * 
 * Jumps the instruction pointer to entrypoint #entrypoint of scenario
 * #scenario.
 */
struct Jmp_jump_1 : public RLOp_Void_2< IntConstant_T, IntConstant_T > {
  virtual bool advanceInstructionPointer() { return false; }

  void operator()(RLMachine& machine, int scenario, int entrypoint) {
    machine.jump(scenario, entrypoint);
  }
};

// -----------------------------------------------------------------------

/** 
 * Implement op<0:Jmp:00012, 0>, fun farcall(intC).
 * 
 * Farcalls the instruction pointer to the begining of the scenario
 * #scenario.
 */
struct Jmp_farcall_0 : public RLOp_Void_1< IntConstant_T > {
  virtual bool advanceInstructionPointer() { return false; }

  void operator()(RLMachine& machine, int scenario) {
    machine.farcall(scenario);
  }
};

// -----------------------------------------------------------------------

/** 
 * Implement op<0:Jmp:00012, 1>, fun farcall(intC, intC).
 * 
 * Farcalls the instruction pointer to entrypoint #entrypoint of scenario
 * #scenario.
 */
struct Jmp_farcall_1 : public RLOp_Void_2< IntConstant_T, IntConstant_T > {
  virtual bool advanceInstructionPointer() { return false; }

  void operator()(RLMachine& machine, int scenario, int entrypoint) {
    machine.farcall(scenario, entrypoint);
  }
};

// -----------------------------------------------------------------------

/** 
 * Implement op<0:Jmp:00013, 0>, fun rtl().
 * 
 * Returns from the current stack frame if it was a farcall.
 *
 * @note This functor MUST increment the instruction pointer, since
 * the instruction pointer at this stack frame is still pointing to
 * the gosub that created the new frame.
 */
struct Jmp_rtl : public RLOp_Void_Void {
  void operator()(RLMachine& machine) {
    machine.returnFromFarcall();
  }
};

// -----------------------------------------------------------------------


struct Jmp_gosub_with : public RLOp_SpecialCase {
  void operator()(RLMachine& machine, const CommandElement& f) {
    const GotoElement& gotoElement = dynamic_cast<const GotoElement&>(f);
    const Pointers& pointers = gotoElement.get_pointersRef();

    // Process the parameters

    machine.gosub(pointers[0]);
  }
};

// -----------------------------------------------------------------------

struct Jmp_farcall_with
  : public RLOp_Void_3< IntConstant_T, IntConstant_T,
                        Argc_T< Special_T< IntConstant_T, StrConstant_T > > >
{
  typedef Special_T< IntConstant_T, StrConstant_T>::type Param;

  void operator()(RLMachine& machine, int scenario, int entrypoint,
                  vector<Param> withStuff) {
    // First, we copy all the input parameters into 
    int intLpos = 0;
    int strKpos = 0;

    for(vector<Param>::iterator it = withStuff.begin(); it != withStuff.end(); 
        ++it)
    {
      switch(it->type) {
      case 0:
        // Make sure we aren't overflowing
        cerr << "(0, " << it->first << ")" << endl;
        break;
      case 1:
        cerr << "(1, " << it->second << ")" << endl;
        break;
      default:
      {
        stringstream ss;
        ss << "Unknown type tag " << it->type << " in Jmp_farcall_with";
        throw Error(ss.str());
      }
      }
    }
  }
};

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
  addOpcode(4, 0, new Jmp_goto_case);
  addOpcode(5, 0, new Jmp_gosub);
  addOpcode(6, 0, new Jmp_gosub_if);
  addOpcode(7, 0, new Jmp_gosub_unless);
  addOpcode(8, 0, new Jmp_gosub_on);
  addOpcode(9, 0, new Jmp_gosub_case);
  addOpcode(10, 0, new Jmp_ret);
  addOpcode(11, 0, new Jmp_jump_0);
  addOpcode(11, 1, new Jmp_jump_1);
  addOpcode(12, 0, new Jmp_farcall_0);
  addOpcode(12, 1, new Jmp_farcall_1);
  addOpcode(13, 0, new Jmp_rtl);

  addOpcode(18, 0, new Jmp_farcall_with);
}

//@}
