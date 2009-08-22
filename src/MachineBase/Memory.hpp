// -*- Mode: C++; tab-width:2; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi:tw=80:et:ts=2:sts=2
//
// -----------------------------------------------------------------------
//
// This file is part of RLVM, a RealLive virtual machine clone.
//
// -----------------------------------------------------------------------
//
// Copyright (C) 2007 Elliot Glaysher
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

#ifndef SRC_MACHINEBASE_MEMORY_HPP_
#define SRC_MACHINEBASE_MEMORY_HPP_

/**
 * @file   Memory.hpp
 * @author Elliot Glaysher
 * @date   Wed Dec 19 19:34:33 2007
 *
 * @brief  Classes that contain local and global memory
 *
 * Classes that contain local and global memory, with abstractions for
 * serializing them (and doing other cool things)
 */

// -----------------------------------------------------------------------

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include "libReallive/intmemref.h"
#include <boost/shared_ptr.hpp>
#include <boost/dynamic_bitset.hpp>
#include <boost/serialization/version.hpp>

// -----------------------------------------------------------------------

const int NUMBER_OF_INT_LOCATIONS = 9;
const int SIZE_OF_MEM_BANK = 2000;
const int SIZE_OF_INT_PASSING_MEM = 40;
const int SIZE_OF_NAME_BANK = 702;

typedef std::vector<std::pair<int, char> > IntegerBank_t;
extern const IntegerBank_t LOCAL_INTEGER_BANKS;
extern const IntegerBank_t GLOBAL_INTEGER_BANKS;

// -----------------------------------------------------------------------

class RLMachine;
class Gameexe;

// -----------------------------------------------------------------------

/**
 * Struct that represents Global Memory. In any one rlvm process, there
 * should only be one GlobalMemory struct existing, as it will be
 * shared over all the Memory objects in the process.
 *
 * @see Memory
 */
struct GlobalMemory {
  GlobalMemory();

  int intG[SIZE_OF_MEM_BANK];
  int intZ[SIZE_OF_MEM_BANK];

  std::string strM[SIZE_OF_MEM_BANK];

  std::string global_names[SIZE_OF_NAME_BANK];

  /**
   * A mapping from a scenario number to a dynamic bitset, where each bit
   * represents a specific kidoku bit.
   */
  std::map<int, boost::dynamic_bitset<> > kidoku_data;

  /// boost::serialization
  template<class Archive>
  void serialize(Archive & ar, unsigned int version) {
    ar & intG & intZ & strM;

    // Starting in version 1, \#NAME variable storage were added.
    if (version > 0) {
      ar & global_names;
      ar & kidoku_data;
    }
  }
};

BOOST_CLASS_VERSION(GlobalMemory, 1)

// -----------------------------------------------------------------------

struct dont_initialize { };

/**
 * Struct that represents Local Memory. In any one rlvm process, lots
 * of these things will be created, because there are commands
 *
 * @see Sys_GetSaveFlag
 * @see Memory
 */
struct LocalMemory {
  LocalMemory();

  /**
   * Constructor that prevents the memory banks from being memset
   * (since they'll be overwritten entirely by the thawing process.
   */
  LocalMemory(dont_initialize);

  /// Zeros and clears all of local memory.
  void reset();

  int intA[SIZE_OF_MEM_BANK];
  int intB[SIZE_OF_MEM_BANK];
  int intC[SIZE_OF_MEM_BANK];
  int intD[SIZE_OF_MEM_BANK];
  int intE[SIZE_OF_MEM_BANK];
  int intF[SIZE_OF_MEM_BANK];

  /// Local string bank
  std::string strS[SIZE_OF_MEM_BANK];

  /// Parameter passing integer bank
  int intL[SIZE_OF_INT_PASSING_MEM];

  /// Parameter passing string bank
  std::string strK[3];

  std::string local_names[SIZE_OF_NAME_BANK];

  /// boost::serialization support
  template<class Archive>
  void serialize(Archive & ar, unsigned int version) {
    ar & intA & intB & intC & intD & intE & intF & strS & intL & strK;

    // Starting in version 1, \#LOCALNAME variable storage were added.
    if (version > 0)
      ar & local_names;
  }
};

BOOST_CLASS_VERSION(LocalMemory, 1)

// -----------------------------------------------------------------------

/**
 * Class that encapsulates access to all integer and string
 * memory. Multiple instances of this class will probably exist if
 * save games are used.
 *
 * @note Because I use BSD code from xclannad in some of the methods
 *       in this class, for licensing purposes, that code is separated
 *       into RLMachine_intmem.cpp.
 *
 * @see RLMachine
 * @see Sys_GetSaveFlag
 */
class Memory {
 private:
  /**
   * Pointer to the GlobalMemory structure. While there can (and will
   * be) multiple Memory instances (this is how we implement
   * GetSaveFlag), we don't really need to duplicate this data
   * structure and can simply pass a pointer to it.
   */
  boost::shared_ptr<GlobalMemory> global_;

  /// Local memory to a save file
  LocalMemory local_;

  /// Integer variable pointers. This redirect into Global and local
  /// memory (as the case may be) allows us to overlay new views of
  /// local memory without copying global memory.
  int* int_var[NUMBER_OF_INT_LOCATIONS];

  /**
   * Connects the memory banks in local_ and in global_ into int_var.
   */
  void connectIntVarPointers();

  /**
   * Input validating function to the {get,set}(Local)?Name set of functions.
   */
  void checkNameIndex(int index, const std::string& name) const;

  /**
   * Reads in default memory values from the passed in Gameexe, such as \#NAME
   * and \#LOCALNAME values.
   */
  void initializeDefaultValues(Gameexe& gameexe);

 public:
  /**
   * Default constructor; creates a Memory object which owns its own
   * GlobalMemory. Initial memory values are read from the passed in Gameexe
   * object.
   *
   * @note For now, we only read \#NAME and \#LOCALNAME variables, skipping any
   *       declaration of the form \#intvar[index] or \#strvar[index].
   */
  Memory(Gameexe& gamexe);

  /**
   * Creates an overlayed memory object. An overlay takes another
   * Memory's global memory.
   *
   * @param machine Machine to overlay memory from.
   * @param slot Save game slot to read local memory from.
   *
   * @warning Local integer memory isn't initialized; it isn't even
   *          memset zeroed out.
   */
  Memory(RLMachine& machine, int slot);

  ~Memory();

  /**
   * Returns the integer value of a certain memory location
   *
   * @param type The memory bank/access method to access from
   * @param location The offset into that memory bank
   * @return The integer value
   * @note This method was plagarized from xclannad.
   */
  int getIntValue(const libReallive::IntMemRef& ref);

  /**
   * Sets the value of a certain memory location
   *
   * @param type The memory bank/access method to access from
   * @param number The offset into that memory bank
   * @param value The new value
   * @note This method was plagarized from xclannad.
   */
  void setIntValue(const libReallive::IntMemRef& ref, int value);

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
   * @name Name tables
   *
   * @{
   */
  /**
   * Sets the local name slot index to name.
   */
  void setName(int index, const std::string& name);

  /**
   * Returns the local name slot index.
   */
  const std::string& getName(int index) const;

  /**
   * Sets the local name slot index to name.
   */
  void setLocalName(int index, const std::string& name);

  /**
   * Returns the local name slot index.
   */
  const std::string& getLocalName(int index) const;
  /// @}

  /**
   * @name Kidoku Access
   *
   * Methods that record whether a piece of text has been read. RealLive
   * scripts have a piece of metadata called a kidoku marker which signifies if
   * the text between that and the next kidoku marker have been previously read.
   *
   * @{
   */
  bool hasBeenRead(int scenario, int kidoku) const;
  void recordKidoku(int scenario, int kidoku);
  /// @}


  /**
   * @name Accessors for serialization.
   *
   * @{
   */
  GlobalMemory& global() { return *global_; }
  const GlobalMemory& global() const { return *global_; }
  LocalMemory& local() { return local_; }
  const LocalMemory& local() const { return local_; }
  /// @}

  /**
   * Converts a RealLive letter index (A-Z, AA-ZZ) to its numeric
   * equivalent. These letter indexies are used in \#NAME definitions.
   */
  static int ConvertLetterIndexToInt(const std::string& value);
};  // end of class Memory


#endif  // SRC_MACHINEBASE_MEMORY_HPP_
