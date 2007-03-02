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

#ifndef __RLMachine_hpp__
#define __RLMachine_hpp__

/**
 * @file   RLMachine.hpp
 * @author Elliot Glaysher
 * @date   Sat Oct  7 10:54:55 2006
 * 
 * @brief Declaration of the main RLMachine class, with supporting
 * constants.
 */

#include <boost/scoped_ptr.hpp>
#include <boost/ptr_container/ptr_map.hpp>

#include "libReallive/bytecode.h"
#include "MachineBase/bytecodeConstants.hpp"

#include <stack>

namespace  libReallive {
class Archive;
class FunctionElement;
};

class RLModule;
class LongOperation;
class System;

  const int INTA_LOCATION = 0   ;
  const int INTB_LOCATION = 1;
  const int INTC_LOCATION = 2;
  const int INTD_LOCATION = 3;
  const int INTE_LOCATION = 4;
  const int INTF_LOCATION = 5;
  const int INTG_LOCATION = 6;

  const int INTZ_LOCATION = 7;

  const int INTL_LOCATION = 8;
  const int NUMBER_OF_INT_LOCATIONS = 9;

/**
 * The RealLive virtual machine implementation.
 */
class RLMachine {
private:
  /// Integer variables. There is a 9 x 2000 integer memory bank. 
  int intVar[NUMBER_OF_INT_LOCATIONS][2000];

  /// First string bank. 
  std::string strS[1999];

  /// Second string bank
  std::string strM[1999];

  std::string strK[3];

  /// The RealLive machine's single result register
  int storeRegister;

  /// Mapping between the module_type:module pair and the module implementation
  typedef boost::ptr_map<unsigned int, RLModule> ModuleMap;
  /// Mapping between the module_type:module pair and the module implementation
  ModuleMap modules;

  /// States whether the RLMachine is in the halted state (and thus
  /// won't execute more instructions)
  bool m_halted;

  /// States whether the machine should halt if an unhandled exception
  /// is thrown
  bool m_haltOnException;

  /// The SEEN.TXT the machine is currently executing.
  libReallive::Archive& archive;

  /** Describes a stack frame. Stack frames are added by two
   * mechanisms: gosubs and farcalls. gosubs move the instruction
   * pointer within one Scenario, while farcalls move the instruction
   * pointer between Scenarios.
   */
  struct StackFrame;

  /// The actual call stack.
  std::stack<StackFrame> callStack;

  /// A pointer to a LongOperation
  boost::scoped_ptr<LongOperation> currentLongOperation;

  /// There are some cases where we need to create our own system,
  /// since one isn't provided for us. This variable is for those
  /// times; m_system will simply point to this object
  boost::scoped_ptr<System> m_ownedSystem;

  /// The most recent line marker we've come across
  int m_line;

  /// The RLMachine carried around a reference to the local system, to
  /// keep it from being a Singleton so we can do proper unit testing.
  System& m_system;

  unsigned int packModuleNumber(int modtype, int module);
  void unpackModuleNumber(unsigned int packedModuleNumber, int& modtype, int& module);

  void executeCommand(const libReallive::CommandElement& f);
  void executeExpression(const libReallive::ExpressionElement& e);

public:
  RLMachine(libReallive::Archive& inArchive);
  RLMachine(System& inSystem, libReallive::Archive& inArchive);
  virtual ~RLMachine();

  /** Registers a given module with this RLMachine instance. A module is a set of
   * different functions registered as one unit.
   *
   * @param module Module to attatch to the RLMachine
   * @note RLMachine takes ownership of any RLModule object passed in through 
   * attatchModule.
   */
  void attatchModule(RLModule* module);

  // -----------------------------------------------------------------------
  /**
   * @name MemoryManip 
   * Memory Manipulation Functions
   */
  // @{

  /** 
   * Returns the integer value of a certain memory location
   * 
   * @param type The memory bank/access method to access from
   * @param location The offset into that memory bank
   * @return The integer value
   */
  int getIntValue(int type, int location);    

  /** 
   * Sets the value of a certain memory location
   * 
   * @param type The memory bank/access method to access from
   * @param number The offset into that memory bank
   * @param value The new value
   */
  void setIntValue(int type, int number, int value);

  /** 
   * Returns the string value of a string memory bank
   * 
   * @param type The memory bank to access from
   * @param location The offset into that memory bank
   * @return The string in that location
   */
  const std::string& getStringValue(int type, int location);

  /** 
   * Sets the string value of one of the string banks
   * 
   * @param type The memory bank to set to
   * @param number The offset into that memory bank
   * @param value The new string value to assign
   */
  void setStringValue(int type, int number, const std::string& value);

  /** 
   * Sets the store register
   * 
   * @param newValue New value of the store register
   */
  void setStoreRegister(int newValue) { storeRegister = newValue; }

  /** 
   * Returns the current value of the store register
   * 
   * @return The value of the store register
   */
  int getStoreRegisterValue() const { return storeRegister; }

  //@}

  // -----------------------------------------------------------------------

  /** @name StackManip
   *  Call stack manipulation functions
   *
   * @{
   */

  /**
   * Permanently modifies the current stack frame to point to the new
   * location.
   *
   * @param scenario SEEN number to jump to
   * @param entrypoint Entrypoint number in that SEEN to jump to
   */
  void jump(int scenario, int entrypoint = 0);

  /** 
   * Push a new stack frame onto the call stack 
   * 
   * @param scenario SEEN number to jump to
   * @param entrypoint Entrypoint number to that SEEN to jump to
   */
  void farcall(int scenario, int entrypoint = 0);

  /** 
   * Return from the most recent farcall().
   * 
   * @throw Error Throws an error when there's a mismatch in the
   * script between farcall()/rtl() gosub()/ret() pairs
   */
  void returnFromFarcall();

  /**
   * Permanently moves the instruction pointer to the passed in
   * iterator in the current stack frame.
   *
   * @param newLocation New location of the instruction pointer.
   */
  void gotoLocation(libReallive::BytecodeList::iterator newLocation);
  
  /** 
   * Pushes a new stack frame onto the call stack, saving the current
   * location. The new frame contains the current SEEN with
   * newLocation as the instruction pointer.
   * 
   * @param newLocation New location of the instruction pointer.
   */
  void gosub(libReallive::BytecodeList::iterator newLocation);

  /** 
   * Returns from the most recent gosub call. 
   * 
   * @throw Error Throws an error when there's a mismatch in the
   * script between farcall()/rtl() gosub()/ret() pairs.
   */
  void returnFromGosub();

  /** 
   * Sets a long operation. Control will be passed to this
   * LongOperation instead of normal bytecode passing until the
   * LongOperation gives control up.
   * 
   * @param longOperation LongOperation to take control
   * @see LongOperation
   */
  void setLongOperation(LongOperation* longOperation);

  /** 
   * Returns the current scene number for the Scenario on the top of
   * the call stack.
   */
  int sceneNumber() const;

  // @}

  // -----------------------------------------------------------------------

  /** 
   * Executes the next instruction in the bytecode in 
   * 
   * @see halted()
   */
  void executeNextInstruction();

  /** 
   * Call executeNextInstruction() repeatedly until the RLMachine is
   * halted. This function is used in unit testing, and would never be
   * called during real usage of an RLMachine instance since other
   * subsytems (graphics, sound, etc) would need to have a chance to
   * fire between RLMachine instructions.
   */
  void executeUntilHalted();

  /** 
   * Increments the stack pointer in the current frame. If we have run
   * off the end of the current scenario, set the halted bit.
   */
  void advanceInstructionPointer();

  /** 
   * Returns whether the machine is halted. When the machine is
   * halted, no more instruction may be executed, either because it
   * ran off the end of a scenario, or because the end() or halt()
   * instruction was called explicitly in the code.
   * 
   * @return Whether the machine is halted
   */
  bool halted() const { return m_halted; }

  /** 
   * Force the machine to halt. This should terminate the execution of
   * bytecode, and theoretically, the program.
   */
  void halt() { m_halted = true; }

  /** 
   * Sets whether the RLMachine will be put into the halt state if an
   * exception is thrown while executing an instruction. By default,
   * it will.
   */
  void setHaltOnException(bool haltOnException) { m_haltOnException = haltOnException; }

  /** 
   * Returns the current System that this RLMachine outputs to.
   */
  System& system() { return m_system; }
};

#endif
