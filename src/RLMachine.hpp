#ifndef __RLMachine_hpp__
#define __RLMachine_hpp__

#include "defs.h"
#include <boost/ptr_container/ptr_map.hpp>
#include "scenario.h"
#include "bytecode.h"
#include "bytecodeConstants.hpp"

#include <stack>

namespace  LIBRL_NAMESPACE {
class Archive;
class FunctionElement;
};

class RLModule;

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

  /// The RealLive machine's single result register
  int storeRegister;

  /// Mapping between the module_type:module pair and the module implementation
  typedef boost::ptr_map<unsigned int, RLModule> ModuleMap;
  /// Mapping between the module_type:module pair and the module implementation
  ModuleMap modules;

  /// States whether the RLMachine is in the halted state (and thus won't
  /// execute more instructions)
  bool m_halted;

  /// States whether the machine should halt if an unhandled exception is thrown
  bool m_haltOnException;

  /// The SEEN.TXT the machine is currently executing.
  Reallive::Archive& archive;

  // Describes a stack frame 
  struct StackFrame {
    StackFrame(Reallive::Scenario* s, const Reallive::Scenario::const_iterator& i) 
      : scenario(s), ip(i) {}

    /// The scenario in the SEEN file for this stack frame.
    Reallive::Scenario* scenario;
    
    /// The instruction pointer in the stack frame.
    Reallive::Scenario::const_iterator ip;
  };

  /// The call stack.
  std::stack<StackFrame> callStack;

  unsigned int packModuleNumber(int modtype, int module);
  void unpackModuleNumber(unsigned int packedModuleNumber, int& modtype, int& module);

  void executeCommand(const LIBRL_NAMESPACE::CommandElement& f);
  void executeExpression(const LIBRL_NAMESPACE::ExpressionElement& e);

public:
  RLMachine(Reallive::Archive& inArchive);
  ~RLMachine();

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
   */
  // @{

  /**
   * Permanently modifies the current stack frame to point to the new
   * location.
   *
   * @param scenario SEEN number to jump to
   * @param entrypoint Entrypoint number in that SEEN to jump to
   */
//  void jump(int scenario, int entrypoint = 0);

  /** 
   * Push a new stack frame onto the call stack 
   * 
   * @param scenario SEEN number to jump to
   * @param entrypoint Entrypoint number to that SEEN to jump to
   */
//  void farcall(int scenario, int entrypoint = 0);

  /** 
   * Return from the most recent farcall().
   * 
   * @throw Error Throws an error when there's a mismatch in the
   * script between farcall()/rtl() gosub()/ret() pairs
   */
//  void rtl();

  /**
   * Permanently moves the instruction pointer to the passed in
   * iterator in the current stack frame.
   *
   * @param newLocation New location of the instruction pointer.
   */
  void gotoLocation(Reallive::BytecodeList::iterator newLocation);
  
  /** 
   * Pushes a new stack frame onto the call stack, saving the current
   * location. The new frame contains the current SEEN with
   * newLocation as the instruction pointer.
   * 
   * @param newLocation New location of the instruction pointer.
   */
//  void gosub(BytecodeList::iterator newLocation);

  /** 
   * Returns from the most recent gosub call. 
   * 
   * @throw Error Throws an error when there's a mismatch in the
   * script between farcall()/rtl() gosub()/ret() pairs.
   */
//  void ret();

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
};

#endif
