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

#include "Modules/Module_Jmp.hpp"

#include <cmath>
#include <sstream>
#include <string>
#include <vector>

#include "MachineBase/RLMachine.hpp"
#include "MachineBase/RLOperation.hpp"
#include "MachineBase/RLOperation/Argc_T.hpp"
#include "MachineBase/RLOperation/Special_T.hpp"
#include "Utilities/Exception.hpp"
#include "libReallive/bytecode.h"
#include "libReallive/intmemref.h"

using namespace std;
using namespace boost;
using namespace libReallive;

// The Flow Control (Jump) Module (mod<0:1>).
//
// Module that describes flow control opcodes in the RealLive virtual
// machine. This module implements commands such as goto, gosub, ret,
// jump, et cetera.
//
// Most of these opcodes derive from RLOp_SpecialCase and thus this
// module should not be used as a template for how to implement a
// RLModule.

namespace {

// Finds which case should be used in the *_case functions.
int evaluateCase(RLMachine& machine, const CommandElement& gotoElement) {
  string tmpval = gotoElement.get_param(0);
  const char* location = tmpval.c_str();

  auto_ptr<ExpressionPiece> condition(get_expression(location));
  int value = condition->integerValue(machine);

  // Walk linearly through the output cases, executing the first
  // match against value.
  int cases = gotoElement.case_count();
  for (int i = 0; i < cases; ++i) {
    string caseUnparsed = gotoElement.get_case(i);

    // Check for bytecode wellformedness. All cases should be
    // surrounded by parens
    if (caseUnparsed[0] != '(' ||
       caseUnparsed[caseUnparsed.size() - 1] != ')')
      throw rlvm::Exception("Malformed bytecode in goto_case statment");

    // In the case of an empty set of parens, always accept. It is
    // the bytecode representation for the default case.
    if (caseUnparsed == "()")
      return i;

    // Strip the parens for parsing
    caseUnparsed = caseUnparsed.substr(1, caseUnparsed.size() - 2);

    // Parse this expression, and goto the corresponding label if
    // it's equal to the value we're searching for
    const char* e = (const char*)caseUnparsed.c_str();
    auto_ptr<ExpressionPiece> output(get_expression(e));
    if (output->integerValue(machine) == value)
      return i;
  }

  throw rlvm::Exception("Malformed bytecode: no default case");
}

// -----------------------------------------------------------------------

/// Type of the parameter data in the _with functions
typedef Argc_T< Special_T< DefaultSpecialMapper,
                           IntConstant_T,
                           StrConstant_T> >::type ParamVector;

// Stores the incoming parameter format into the local variables used for
// parameters in gosub_with and farcall_with calls, and return them to the
// caller. We read the data before pushing the stack frame because intL[] and
// strK[]'s values change after the gosub_with/farcall_with.
void readWithData(RLMachine& machine, const ParamVector& f,
                  std::vector<int>& integers,
                  std::vector<std::string>& strings) {
  for (ParamVector::const_iterator it = f.begin(); it != f.end();
      ++it) {
    switch (it->type) {
    case 0:
      integers.push_back(it->first);
      break;
    case 1:
      strings.push_back(it->second);
      break;
    default: {
      ostringstream ss;
      ss << "Unknown type tag " << it->type
         << " during a *_with function call";
      throw rlvm::Exception(ss.str());
    }
    }
  }
}

// Write the data after we've changed the stack frame.
void writeWithData(RLMachine& machine,
                   const std::vector<int>& integers,
                   const std::vector<std::string>& strings) {
  for (int i = 0; i < 40 && i < integers.size(); ++i) {
    machine.setIntValue(IntMemRef(INTL_LOCATION, 0, i), integers[i]);
  }

  for (int i = 0; i < 3 && i < strings.size(); ++i) {
    machine.setStringValue(STRK_LOCATION, i, strings[i]);
  }
}

// Implements op<0:Jmp:00000, 0>, fun goto().
//
// Jumps to the supplied label in the current scenario.
struct Jmp_goto : public RLOp_SpecialCase {
  void operator()(RLMachine& machine, const CommandElement& gotoElement) {
    // Goto the first pointer
    machine.gotoLocation(gotoElement.get_pointer(0));
  }
};

// Implements op<0:Jmp:00001, 0>, fun goto_if (<'condition').
//
// Conditional equivalents of goto; goto_if () jumps to @label if the value of
// condition is non-zero
struct goto_if : public RLOp_SpecialCase {
  void operator()(RLMachine& machine, const CommandElement& gotoElement) {
    const ptr_vector<ExpressionPiece>& conditions = gotoElement.getParameters();

    if (conditions.at(0).integerValue(machine)) {
      machine.gotoLocation(gotoElement.get_pointer(0));
    } else {
      machine.advanceInstructionPointer();
    }
  }
};

// Implements op<0:Jmp:00002, 0>, fun goto_unless (<'condition').
struct goto_unless : public RLOp_SpecialCase {
  void operator()(RLMachine& machine, const CommandElement& gotoElement) {
    const ptr_vector<ExpressionPiece>& conditions = gotoElement.getParameters();

    if (!conditions.at(0).integerValue(machine)) {
      machine.gotoLocation(gotoElement.get_pointer(0));
    } else {
      machine.advanceInstructionPointer();
    }
  }
};

// Implements op<0:Jmp:00003, 0>, fun goto_on(special case).
//
// Table jumps. expr is evaluated, and control passed to the
// corresponding label in the list, counting from 0. If expr falls
// outside the valid range, no jump takes place, and execution
// continues from the next statement instead.
//
// TODO(erg): Figure out why I couldn't use cached expressions here.
struct goto_on : public RLOp_SpecialCase {
  void operator()(RLMachine& machine, const CommandElement& gotoElement) {
    // use a temporary object so that it is only destroyed at end of scope! --RT
    string tmpval = gotoElement.get_param(0);
    const char* location = tmpval.c_str();
    auto_ptr<ExpressionPiece> condition(get_expression(location));
    int value = condition->integerValue(machine);

    if (value >= 0 && value < int(gotoElement.pointers_count())) {
      machine.gotoLocation(gotoElement.get_pointer(value));
    } else {
      // If the value is not a valid pointer, simply increment.
      machine.advanceInstructionPointer();
    }
  }
};

// Implements op<0:Jmp:00004, 0>, fun goto_case (expr) { val1: @label1... }
//
// Conditional table jumps. expr is evaluated, and
// compared to val1, val2, etc. in turn, and control passes to the
// label associated with the first matching value.
struct goto_case : public RLOp_SpecialCase {
  void operator()(RLMachine& machine, const CommandElement& gotoElement) {
    int i = evaluateCase(machine, gotoElement);
    machine.gotoLocation(gotoElement.get_pointer(i));
  }
};

// Implements op<0:Jmp:00005, 0>, fun gosub().
//
// Pushes the current location onto the call stack, then jumps to the
// label @label in the current scenario.
struct gosub : public RLOp_SpecialCase {
  void operator()(RLMachine& machine, const CommandElement& gotoElement) {
    machine.gosub(gotoElement.get_pointer(0));
  }
};

// Implements op<0:Jmp:00006, 0>, fun gosub_if ().
//
// Pushes the current location onto the call stack, then jumps to the
// label @label in the current scenario, if the passed in condition is
// true.
struct gosub_if : public RLOp_SpecialCase {
  void operator()(RLMachine& machine, const CommandElement& gotoElement) {
    string tmpval = gotoElement.get_param(0);
    const char* location = tmpval.c_str();
    auto_ptr<ExpressionPiece> condition(get_expression(location));

    if (condition->integerValue(machine)) {
      machine.gosub(gotoElement.get_pointer(0));
    } else {
      machine.advanceInstructionPointer();
    }
  }
};

// Implements op<0:Jmp:00007, 0>, fun gosub_unless().
//
// Pushes the current location onto the call stack, then jumps to the label
// @label in the current scenario, if the passed in condition is false.
struct gosub_unless : public RLOp_SpecialCase {
  void operator()(RLMachine& machine, const CommandElement& gotoElement) {
    const ptr_vector<ExpressionPiece>& conditions = gotoElement.getParameters();

    if (!conditions.at(0).integerValue(machine)) {
      machine.gosub(gotoElement.get_pointer(0));
    } else {
      machine.advanceInstructionPointer();
    }
  }
};

// Implements op<0:Jmp:00008, 0>, fun gosub_on(special case).
//
// Table gosub. expr is evaluated, and control passed to the corresponding
// label in the list, counting from 0. If expr falls outside the valid range,
// no gosub takes place, and execution continues from the next statement
// instead.
struct gosub_on : public RLOp_SpecialCase {
  void operator()(RLMachine& machine, const CommandElement& gotoElement) {
    const ptr_vector<ExpressionPiece>& conditions = gotoElement.getParameters();
    int value = conditions.at(0).integerValue(machine);

    if (value >= 0 && value < int(gotoElement.pointers_count()))
      machine.gosub(gotoElement.get_pointer(value));
    else
      // If the value is not a valid pointer, simply increment.
      machine.advanceInstructionPointer();
  }
};

// Implements op<0:Jmp:00009, 0>, fun gosub_case (expr) { val1: @label1... }
//
// Conditional table gosub. expr is evaluated, and compared to val1, val2,
// etc. in turn, and control passes to the label associated with the first
// matching value.
struct gosub_case : public RLOp_SpecialCase {
  void operator()(RLMachine& machine, const CommandElement& gotoElement) {
    int i = evaluateCase(machine, gotoElement);
    machine.gosub(gotoElement.get_pointer(i));
  }
};

// Implements op<0:Jmp:00010, 0>, fun ret().
//
// Returns from the current stack frame if it was a farcall.
//
// This functor MUST increment the instruction pointer, since the instruction
// pointer at this stack frame is still pointing to the gosub that created the
// new frame.
struct ret : public RLOp_Void_Void {
  void operator()(RLMachine& machine) {
    machine.returnFromGosub();
  }
};

// Implements op<0:Jmp:00011, 0>, fun jump(intC).
//
// Jumps the instruction pointer to the begining of the |scenario|.
struct jump_0 : public RLOp_Void_1< IntConstant_T > {
  virtual bool advanceInstructionPointer() { return false; }

  void operator()(RLMachine& machine, int scenario) {
    machine.jump(scenario, 0);
  }
};

// Implements op<0:Jmp:00011, 1>, fun jump(intC, intC).
//
// Jumps the instruction pointer to |entrypoint| of |scenario|.
struct jump_1 : public RLOp_Void_2< IntConstant_T, IntConstant_T > {
  virtual bool advanceInstructionPointer() { return false; }

  void operator()(RLMachine& machine, int scenario, int entrypoint) {
    machine.jump(scenario, entrypoint);
  }
};

// Implements op<0:Jmp:00012, 0>, fun farcall(intC).
//
// Farcalls the instruction pointer to the begining of the |scenario|.
struct farcall_0 : public RLOp_Void_1< IntConstant_T > {
  virtual bool advanceInstructionPointer() { return false; }

  void operator()(RLMachine& machine, int scenario) {
    machine.farcall(scenario, 0);
  }
};

// Implements op<0:Jmp:00012, 1>, fun farcall(intC, intC).
//
// Farcalls the instruction pointer to |entrypoint| of |scenario|.
struct farcall_1 : public RLOp_Void_2< IntConstant_T, IntConstant_T > {
  virtual bool advanceInstructionPointer() { return false; }

  void operator()(RLMachine& machine, int scenario, int entrypoint) {
    machine.farcall(scenario, entrypoint);
  }
};

// Implements op<0:Jmp:00013, 0>, fun rtl().
//
// Returns from the current stack frame if it was a farcall.
//
// This functor MUST increment the instruction pointer, since the instruction
// pointer at this stack frame is still pointing to the gosub that created the
// new frame.
struct rtl : public RLOp_Void_Void {
  void operator()(RLMachine& machine) {
    machine.returnFromFarcall();
  }
};

// Implements op<0:Jmp:16,0>, fun gosub_with(params...) @label.
//
// Sets the passing variables intL[] and strK[] to the incoming parameters and
// gosub to the given label.
//
// This operation is implemented in a hacky way, since gosub_with is really
// unlike most other functions, including other RLOp_SpecialCase implemented
// functions. It requires both casting from the CommandElement itself to get
// special data, as well as doing normal parameter parsing. Therefore, I invoke
// the static methods on the type checking structs directly to reuse code.
struct gosub_with : public RLOp_SpecialCase {
  void operator()(RLMachine& machine, const CommandElement& gotoElement) {
    typedef Argc_T<Special_T<DefaultSpecialMapper,
                             IntConstant_T, StrConstant_T> > ParamFormat;

    const ptr_vector<ExpressionPiece>& parameterPieces =
        gotoElement.getParameters();
    unsigned int position = 0;
    ParamFormat::type data =
        ParamFormat::getData(machine, parameterPieces, position);

    std::vector<int> integers;
    std::vector<std::string> strings;
    readWithData(machine, data, integers, strings);

    machine.gosub(gotoElement.get_pointer(0));

    writeWithData(machine, integers, strings);
  }
};

// Implements op<0:Jmp:00017, 0>, fun ret_with('value').
//
// Returns from a goto_with call, storing the value in the store register.
struct ret_with_0 : public RLOp_Void_1< IntConstant_T > {
  void operator()(RLMachine& machine, int retVal) {
    machine.setStoreRegister(retVal);
    machine.returnFromGosub();
  }
};

// Implements op<0:Jmp:00017, 1>, fun ret_with().
//
// Returns from a goto_with call. But it doesn't return a value. What gets
// dumped in the store register?
struct ret_with_1 : public RLOp_Void_Void {
  void operator()(RLMachine& machine) {
    machine.returnFromGosub();
  }
};

// Implements op<0:Jmp:00018, 0>, fun farcall_with().
//
// Performs a call into a target scenario/entrypoint pair, passing along all
// values passed in to the first avaiable intL[] and strK[] memory blocks.
struct farcall_with
  : public RLOp_Void_3< IntConstant_T, IntConstant_T,
                        Argc_T< Special_T< DefaultSpecialMapper,
                                           IntConstant_T,
                                           StrConstant_T > > > {
  virtual bool advanceInstructionPointer() { return false; }

  void operator()(RLMachine& machine, int scenario, int entrypoint,
                  ParamVector withStuff) {
    std::vector<int> integers;
    std::vector<std::string> strings;
    readWithData(machine, withStuff, integers, strings);

    machine.farcall(scenario, entrypoint);

    writeWithData(machine, integers, strings);
  }
};

// Implements op<0:Jmp:00019, 0>, fun rtl_with(intC).
//
// Returns from the current stack frame if it was a farcall_with.
//
// This functor MUST increment the instruction pointer, since the instruction
// pointer at this stack frame is still pointing to the gosub that created the
// new frame.
struct rtl_with_0 : public RLOp_Void_1< IntConstant_T > {
  void operator()(RLMachine& machine, int retVal) {
    machine.setStoreRegister(retVal);
    machine.returnFromFarcall();
  }
};

// Implements op<0:Jmp:00019, 1>, fun rtl_with().
//
// Returns from the current stack frame if it was a farcall_with.
//
// This functor MUST increment the instruction pointer, since the instruction
// pointer at this stack frame is still pointing to the gosub that created the
// new frame.
struct rtl_with_1 : public RLOp_Void_Void {
  void operator()(RLMachine& machine) {
    machine.returnFromFarcall();
  }
};

// Pushes a string value into strK[index] one stack frame above the current
// one. Used in the Little Busters battle system to return string values that
// refer to people's faces. (See SEEN8700).
struct push_string_value_up : public RLOp_Void_2<IntConstant_T, StrConstant_T> {
  void operator()(RLMachine& machine, int index, std::string val) {
    machine.pushStringValueUp(index, val);
  }
};

}  // namespace

// -----------------------------------------------------------------------

JmpModule::JmpModule()
    : RLModule("Jmp", 0, 1) {
  addOpcode(0, 0, "goto", new Jmp_goto);
  addOpcode(1, 0, "goto_if", new goto_if);
  addOpcode(2, 0, "goto_unless", new goto_unless);
  addOpcode(3, 0, "goto_on", new goto_on);
  addOpcode(4, 0, "goto_case", new goto_case);
  addOpcode(5, 0, "gosub", new gosub);
  addOpcode(6, 0, "gosub_if", new gosub_if);
  addOpcode(7, 0, "gosub_unless", new gosub_unless);
  addOpcode(8, 0, "gosub_on", new gosub_on);
  addOpcode(9, 0, "gosub_case", new gosub_case);
  addOpcode(10, 0, "ret", new ret);
  addOpcode(11, 0, "jump", new jump_0);
  addOpcode(11, 1, "jump", new jump_1);
  addOpcode(12, 0, "farcall", new farcall_0);
  addOpcode(12, 1, "farcall", new farcall_1);
  addOpcode(13, 0, "rtl", new rtl);

  addOpcode(16, 0, "gosub_with", new gosub_with);
  addOpcode(17, 0, "ret_with", new ret_with_0);
  addOpcode(17, 1, "ret_with", new ret_with_1);
  addOpcode(18, 0, "farcall_with", new farcall_with);
  addOpcode(19, 0, "rtl_with", new rtl_with_0);
  addOpcode(19, 1, "rtl_with", new rtl_with_1);

  addOpcode(101, 0, "pushStringValueUp", new push_string_value_up);
}
