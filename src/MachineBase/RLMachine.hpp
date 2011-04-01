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
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
//
// -----------------------------------------------------------------------

#ifndef SRC_MACHINEBASE_RLMACHINE_HPP_
#define SRC_MACHINEBASE_RLMACHINE_HPP_

#include <boost/function.hpp>
#include <boost/ptr_container/ptr_map.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/serialization/split_member.hpp>

#include "libReallive/bytecode_fwd.h"
#include "libReallive/scenario.h"

#include <string>
#include <vector>
#include <boost/shared_ptr.hpp>

namespace  libReallive {
class Archive;
class IntMemRef;
};

class LongOperation;
class Memory;
class OpcodeLog;
class RLModule;
class RealLiveDLL;
class System;
struct StackFrame;

// The RealLive virtual machine implementation. This class is the main user
// facing class which contains all state regarding integer/string memory, flow
// control, and other execution issues.
class RLMachine {
 public:
  RLMachine(System& in_system, libReallive::Archive& in_archive);
  virtual ~RLMachine();

  // Registers a given module with this RLMachine instance. A module is a set
  // of different functions registered as one unit. Takes ownership of
  // |module|.
  virtual void attachModule(RLModule* module);

  // ------------------------------------- [ Implicit savepoint management ]
  // RealLive will save the latest savepoint for the topmost stack
  // frame. Savepoints can be manually set (with the "Savepoint" command), but
  // are usually implicit.

  // Mark a savepoint on the top of the stack. Used by both the
  // explicit Savepoint command, and most actions that would trigger
  // an implicit savepoint.
  void markSavepoint();

  // Checks to see if we should set a savepoint on the start of a
  // textout when all text windows are empty (aka, when a message starts)
  bool shouldSetMessageSavepoint() const;

  // Checks to see if we should set a savepoint on the start of a
  // user selection choice.
  bool shouldSetSelcomSavepoint() const;

  // Do we set a savepoint when we enter the top of a seen. (This may
  // be on every farcall, or it may mean \#entrypoint 0. We're not sure.)
  bool shouldSetSeentopSavepoint() const;

  typedef int(libReallive::Scenario::*AttributeFunction)() const;

  // Implementation function for should_set*Savepoint().
  //
  // - If automatic savepoints have been explicitly disabled with
  //   DisableAutoSavepoints, return false. Otherwise...
  // - The current Scenario is checked; in the bytecode header, there
  //   is a value for each of these properties. If it is 1 (always
  //   create this class of savepoint) or 2 (never), then we
  //   return. On any other value, we fall through to...
  // - Check a Gameexe key, which has the final say.
  bool savepointDecide(AttributeFunction func,
                       const std::string& gameexe_key) const;

  // Whether the DisableAutoSavepoints override is on. This is
  // triggered purely from bytecode.
  void setMarkSavepoints(const int in);

  // ------------------------------------- [ Memory manipulation functions ]

  // Returns the integer value of a certain memory location
  int getIntValue(const libReallive::IntMemRef& ref);

  // Sets the value of a certain memory location
  void setIntValue(const libReallive::IntMemRef& ref, int value);

  // Returns the string value of a string memory bank
  const std::string& getStringValue(int type, int location);

  // Sets the string value of one of the string banks
  void setStringValue(int type, int number, const std::string& value);

  // Sets the store register
  void setStoreRegister(int new_value) { store_register_ = new_value; }

  // Returns the current value of the store register
  int getStoreRegisterValue() const { return store_register_; }

  // Returns the integer that is the store register. (This is exposed for the
  // memory iterators to work properly.)
  int* storeRegisterAddress() { return &store_register_; }

  // Returns the internal memory object for raw access to the machine object's
  // memory. This should only be used during serialization or complex memory
  // operations involving overlays.
  Memory& memory() { return *memory_; }
  const Memory& memory() const { return *memory_; }

  // --------------------------------- [ Call stack manipulation functions ]

  // Permanently modifies the current stack frame to point to the new
  // location.
  void jump(int scenario, int entrypoint);

  // Push a new stack frame onto the call stack
  void farcall(int scenario, int entrypoint);

  // Return from the most recent farcall().
  void returnFromFarcall();

  // Permanently moves the instruction pointer to the passed in
  // iterator in the current stack frame.
  void gotoLocation(libReallive::BytecodeList::iterator new_location);

  // Pushes a new stack frame onto the call stack, saving the current
  // location. The new frame contains the current SEEN with
  // new_location as the instruction pointer.
  void gosub(libReallive::BytecodeList::iterator new_location);

  // Returns from the most recent gosub call. Throws if there's a mismatch
  // between farcall()/rtl() gosub()/ret() pairs.
  void returnFromGosub();

  // Writes |val| to strK[index] in the stack frame above the current one. Used
  // to return strings from {farcall,gosub}_with constructs (since rtl_with
  // only accepts integers).
  void pushStringValueUp(int index, const std::string& val);

  // Pushes a long operation onto the function stack. Control will be passed to
  // this LongOperation instead of normal bytecode passing until the
  // LongOperation gives control up.
  virtual void pushLongOperation(LongOperation* long_operation);

  // Returns a pointer to the currently running LongOperation when the top of
  // the call stack is a LongOperation. NULL otherwise.
  boost::shared_ptr<LongOperation> currentLongOperation() const;

  // Clears the callstack, properly freeing any LongOperations.
  void clearCallstack();

  // Returns the current scene number for the Scenario on the top of
  // the call stack.
  int sceneNumber() const;

  // Returns the actual Scenario on the top top of the call stack.
  const libReallive::Scenario& scenario() const;

  // Returns the value of the most recent line MetadataElement, which
  // should correspond with the line in the source file.
  int lineNumber() const { return line_; }

  // Returns the current Archive we are attached to.
  libReallive::Archive& archive() { return archive_; }

  void set_replaying_graphics_stack(bool in) { replaying_graphics_stack_ = in; }
  bool replaying_graphics_stack() { return replaying_graphics_stack_; }

  // ------------------------------------------------ [ Execuion interface ]
  // Normally, execute_next_instruction will call runOnMachine() on
  // whatever BytecodeElement is currently pointed to by the
  // instruction pointer.

  // Sets the current line number
  void setLineNumber(const int i);

  // Where the current scenario was compiled with RLdev, returns the text
  // encoding used:
  //   0 -> CP932
  //   1 -> CP936 within CP932 codespace
  //   2 -> CP1252 within CP932 codespace
  //   3 -> CP949 within CP932 codespace
  // Where a scenario was not compiled with RLdev, always returns 0.
  int getTextEncoding() const;

  void executeCommand(const libReallive::CommandElement& f);
  void executeExpression(const libReallive::ExpressionElement& e);
  void performTextout(const libReallive::TextoutElement& e);
  void performTextout(const std::string& cp932str);

  // Marks a kidoku marker as visited.
  void setKidokuMarker(int kidoku_number);

  // ---------------------------------------------------- [ DLL Management ]
  // RealLive has an extension system where a DLL can be loaded, and can be
  // called from bytecode through a ridiculously underpowered interface. We
  // couldn't support this even if we linked in winelib because the only way to
  // get things done is for the DLL to poke around in the memory of the
  // RealLive process. Haeleth (and insani?) have abused this for their own
  // purposes.
  //
  // So instead, we present the interface, and have our own versions of popular
  // DLLs compiled in. For now, that's just rlBabel.

  // Returns true if a DLL with |name| loaded.
  bool dllLoaded(const std::string& name);

  // Loads a "DLL" into the specified slot.
  void loadDLL(int slot, const std::string& name);

  // Unloads the "DLL"
  void unloadDLL(int slot);

  // Calls a DLL through the only interface we're given. (seriously, this is
  // it.)
  int callDLL(int slot, int one, int two, int three, int four, int five);

  // -----------------------------------------------------------------------

  // Executes the next instruction in the bytecode in
  void executeNextInstruction();

  // Call executeNextInstruction() repeatedly until the RLMachine is
  // halted. This function is used in unit testing, and would never be
  // called during real usage of an RLMachine instance since other
  // subsytems (graphics, sound, etc) would need to have a chance to
  // fire between RLMachine instructions.
  void executeUntilHalted();

  // Increments the stack pointer in the current frame. If we have run
  // off the end of the current scenario, set the halted bit.
  void advanceInstructionPointer();

  // Returns whether the machine is halted. When the machine is
  // halted, no more instruction may be executed, either because it
  // ran off the end of a scenario, or because the end() or halt()
  // instruction was called explicitly in the code.
  bool halted() const { return halted_; }

  // Pauses execution and notifies the System. Every call to
  // executeNextInstruction() will return immediately and the System's internal
  // timer will stop ticking.
  void pauseExecution();

  // Resume execution.
  void unpauseExecution();

  // ---------------------------------------------------------------------

  // Whether we report to stderr when we hit an undefined opcode.
  void setPrintUndefinedOpcodes(bool in);

  // Starts logging opcodes that we don't handle. Over very long runs, this is
  // easier to deal with than set_print_undefined_opcodes. Will print the
  // results to stderr on machine destruction.
  void recordUndefinedOpcodeCounts();

  // ---------------------------------------------------------------------

  // Force the machine to halt. This should terminate the execution of
  // bytecode, and theoretically, the program.
  void halt();

  // Sets whether the RLMachine will be put into the halt state if an
  // exception is thrown while executing an instruction. By default,
  // it will.
  void setHaltOnException(bool halt_on_exception);

  // Returns the current System that this RLMachine outputs to.
  System& system() { return system_; }

  unsigned int packModuleNumber(int modtype, int module);
  void unpackModuleNumber(unsigned int packed_module_number, int& modtype,
                          int& module);

  // Pushes a stack frame onto the call stack, alerting possible
  // LongOperations of this change if needed.
  void pushStackFrame(const StackFrame& frame);

  // Pops a stack frame from the call stack, alerting possible
  // LongOperations of this change if needed.
  void popStackFrame();

  // Returns the intL bank of the current stack frame.
  int* currentIntLBank();

  // Returns the strK bank of the current stack frame.
  std::string* currentStrKBank();

  // Clears all LongOperations from the back of the stack.
  void clearLongOperationsOffBackOfStack();

  // Clears all call stacks and other data. Does not clear any local memory, as
  // this should only be called right before a load.
  void reset();

  // Resets pieces of local memory. Correspondingly does NOT clear the call
  // stack, though it does clear the shadow save stack.
  void localReset();

  // Adds a programatic action triggered by a line marker in a specific SEEN
  // file. This is used both by luaRlvm to trigger actions specified in lua to
  // drive rlvm's playing certain games, but is also used for game specific
  // hacks.
  void addLineAction(const int seen, const int line,
                     boost::function<void(void)>);

 private:
  // The Reallive VM's integer and string memory
  boost::scoped_ptr<Memory> memory_;

  // The RealLive machine's single result register
  int store_register_;

  // Mapping between the module_type:module pair and the module implementation
  typedef boost::ptr_map<unsigned int, RLModule> ModuleMap;
  // Mapping between the module_type:module pair and the module implementation
  ModuleMap modules_;

  // When true, we should ignore all execution request return immediately. This
  // is used by native dialog stuff.
  int interpreter_paused_;

  // States whether the RLMachine is in the halted state (and thus won't
  // execute more instructions)
  bool halted_;

  // Whether we should print an error to stderr when we encounter an undefined
  // opcode.
  bool print_undefined_opcodes_;

  // States whether the machine should halt if an unhandled exception is thrown
  bool halt_on_exception_;

  // The SEEN.TXT the machine is currently executing.
  libReallive::Archive& archive_;

  // The actual call stack.
  std::vector<StackFrame> call_stack_;

  // The state of the call stack the last time a savepoint was called
  std::vector<StackFrame> savepoint_call_stack_;

  // The most recent line marker we've come across
  int line_;

  // The RLMachine carried around a reference to the local system, to keep it
  // from being a Singleton so we can do proper unit testing.
  System& system_;

  // (Optional) A structure that keeps track of how many times we encountered
  // undefined opcodes.
  boost::scoped_ptr<OpcodeLog> undefined_log_;

  // Override defaults
  bool mark_savepoints_;

  // Whether the stack was modified during the running of a
  // LongOperation. Used to signal that any stack mutating functions should be
  // be placed in |delay_modifications_| for execution later.
  bool delay_stack_modifications_;

  // Whether we are currently replaying the graphics stack. While replaying the
  // graphics stack, we shouldn't advance the instruction pointer and do other
  // stuff.
  bool replaying_graphics_stack_;

  /// The actions that were delayed when |delay_stack_modifications_| is on.
  std::vector<boost::function<void(void)> > delayed_modifications_;

  // An optional set of game specific hacks that run at certain SEEN/line
  // pairs. These run during setLineNumer().
  typedef std::map<std::pair<int, int>, boost::function<void(void)> > ActionMap;
  boost::scoped_ptr<ActionMap> on_line_actions_;

  typedef boost::ptr_map<int, RealLiveDLL> DLLMap;
  // Currenlty loaded "DLLs".
  DLLMap loaded_dlls_;

  // boost::serialization support
  friend class boost::serialization::access;

  template<class Archive>
  void save(Archive & ar, const unsigned int file_version) const;

  template<class Archive>
  void load(Archive& ar, const unsigned int file_version);

  BOOST_SERIALIZATION_SPLIT_MEMBER()
};

#endif  // SRC_MACHINEBASE_RLMACHINE_HPP_
