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

#ifndef __RLModule_hpp__
#define __RLModule_hpp__

/**
 * @file   RLModule.hpp
 * @author Elliot Glaysher
 * @date   Sat Oct  7 10:57:36 2006
 * 
 * @brief  Declares the base class RLModule 
 */

#include <boost/utility.hpp>
#include <boost/ptr_container/ptr_map.hpp>

//#include "libReallive/expression.h"

#include <map>

namespace libReallive {
class FunctionElement;
class CommandElement;
};      

class RLOperation;
class RLMachine;

/**
 * @defgroup ModulesOpcodes Modules and Opcode Definition.
 * 
 * Contains definitions for each operation that RLVM executes based
 * off of an instruction in the SEEN.TXT file.
 *
 * Each opcode/overload that RLVM recognizes has a concrete instance
 * of RLOperation represented with it's opcode number. There are a set
 * of GeneralOperations that take parameters and are used as function
 * binders to call various functions, but most opcodes are defined by
 * having a concrete subclass of RLOp_Store<> or RLOp_Void<>.
 */

/** Describes a Module, a grouping of functions. Modules are added to
 *  an RLMachine before the machine starts.
 *
 * @ingroup ModulesOpcodes
 */
class RLModule : public boost::noncopyable {
private:
  int m_moduleType;
  int m_moduleNumber;
  std::string m_moduleName;

  // Store functions. 
  typedef boost::ptr_map<int, RLOperation> OpcodeMap;
  OpcodeMap storedOperations;

  int packOpcodeNumber(int opcode, unsigned char overload);
  void unpackOpcodeNumber(int packedOpcode, int& opcode, unsigned char& overload);

protected:
  RLModule(const std::string& inModuleName, int inModuleType, int inModuleNumber);

  /** Used in derived Module constructors to declare all the
   * operations the module handles.
   *
   * @note The RLModule class takes ownership of any RLOperation
   * objects passed in this way.
   *
   * @param opcode The opcode number of this operation
   * @param overload The overload number of this operation
   * @param op An RLOperation functor which represents the
   *           implementation of this operation.
   */
  void addOpcode(int opcode, unsigned char overload, RLOperation* op);

public:
  /// Accessor that returns this module's type number
  int moduleType() const { return m_moduleType; }

  /// Accessor that returns this modules's identification number
  int moduleNumber() const { return m_moduleNumber; }

  /// Accessor that returns this module's mnemonic nmae
  const std::string& moduleName() const { return m_moduleName; }

  /** 
   * Using the bytecode element CommandElement f, try to find an
   * RLOperation implementation of the instruction in this module, and
   * execute it.
   * 
   * @param machine The RLMachine we are operating with
   * @param f The bytecode element that we are trying to execute
   */
  void dispatchFunction(RLMachine& machine, const libReallive::CommandElement& f);
};

#endif
