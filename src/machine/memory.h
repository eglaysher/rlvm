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
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
//
// -----------------------------------------------------------------------

#ifndef SRC_MACHINE_MEMORY_H_
#define SRC_MACHINE_MEMORY_H_

#include <boost/dynamic_bitset.hpp>
#include <boost/serialization/split_member.hpp>
#include <boost/serialization/version.hpp>

#include <algorithm>
#include <map>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "libreallive/intmemref.h"

const int NUMBER_OF_INT_LOCATIONS = 8;
const int SIZE_OF_MEM_BANK = 2000;
const int SIZE_OF_INT_PASSING_MEM = 40;
const int SIZE_OF_NAME_BANK = 702;

typedef std::vector<std::pair<int, char>> IntegerBank_t;
extern const IntegerBank_t LOCAL_INTEGER_BANKS;
extern const IntegerBank_t GLOBAL_INTEGER_BANKS;

class RLMachine;
class Gameexe;

// Struct that represents Global Memory. In any one rlvm process, there
// should only be one GlobalMemory struct existing, as it will be
// shared over all the Memory objects in the process.
struct GlobalMemory {
  GlobalMemory();

  int intG[SIZE_OF_MEM_BANK];
  int intZ[SIZE_OF_MEM_BANK];

  std::string strM[SIZE_OF_MEM_BANK];

  std::string global_names[SIZE_OF_NAME_BANK];

  // A mapping from a scenario number to a dynamic bitset, where each bit
  // represents a specific kidoku bit.
  std::map<int, boost::dynamic_bitset<>> kidoku_data;

  // boost::serialization
  template <class Archive>
  void serialize(Archive& ar, unsigned int version) {
    ar& intG& intZ& strM;

    // Starting in version 1, \#NAME variable storage were added.
    if (version > 0) {
      ar& global_names;
      ar& kidoku_data;
    }
  }
};

BOOST_CLASS_VERSION(GlobalMemory, 1)

struct dont_initialize {};

// Struct that represents Local Memory. In any one rlvm process, lots
// of these things will be created, because there are commands
struct LocalMemory {
  LocalMemory();

  // Constructor that prevents the memory banks from being memset
  // (since they'll be overwritten entirely by the thawing process.
  explicit LocalMemory(dont_initialize);

  // Zeros and clears all of local memory.
  void reset();

  int intA[SIZE_OF_MEM_BANK];
  int intB[SIZE_OF_MEM_BANK];
  int intC[SIZE_OF_MEM_BANK];
  int intD[SIZE_OF_MEM_BANK];
  int intE[SIZE_OF_MEM_BANK];
  int intF[SIZE_OF_MEM_BANK];

  // Local string bank
  std::string strS[SIZE_OF_MEM_BANK];

  // When one of our values is changed, we put the original value in here. Why?
  // So that we can save the state of string memory at the time of the last
  // Savepoint(). Instead of doing some sort of copying entire memory banks
  // whenever we hit a Savepoint() call, only reconstruct the original memory
  // when we save.
  std::map<int, int> original_intA;
  std::map<int, int> original_intB;
  std::map<int, int> original_intC;
  std::map<int, int> original_intD;
  std::map<int, int> original_intE;
  std::map<int, int> original_intF;
  std::map<int, std::string> original_strS;

  std::string local_names[SIZE_OF_NAME_BANK];

  // Combines an array with a log of original values and writes the de-modified
  // array to |ar|.
  template <class Archive, typename T>
  void saveArrayRevertingChanges(Archive& ar,
                                 const T (&a)[SIZE_OF_MEM_BANK],
                                 const std::map<int, T>& original) const {
    T merged[SIZE_OF_MEM_BANK];
    std::copy(a, a + SIZE_OF_MEM_BANK, merged);
    for (auto it = original.cbegin(); it != original.cend(); ++it) {
      merged[it->first] = it->second;
    }
    ar& merged;
  }

  // boost::serialization support
  template <class Archive>
  void save(Archive& ar, unsigned int version) const {
    saveArrayRevertingChanges(ar, intA, original_intA);
    saveArrayRevertingChanges(ar, intB, original_intB);
    saveArrayRevertingChanges(ar, intC, original_intC);
    saveArrayRevertingChanges(ar, intD, original_intD);
    saveArrayRevertingChanges(ar, intE, original_intE);
    saveArrayRevertingChanges(ar, intF, original_intF);

    saveArrayRevertingChanges(ar, strS, original_strS);

    ar& local_names;
  }

  template <class Archive>
  void load(Archive& ar, unsigned int version) {
    ar& intA& intB& intC& intD& intE& intF& strS;

    // Starting in version 2, we no longer have the intL and strK in
    // LocalMemory. They were moved to StackFrame because they're stack
    // local. Throw away old data.
    if (version < 2) {
      int intL[SIZE_OF_INT_PASSING_MEM];
      ar& intL;

      std::string strK[3];
      ar& strK;
    }

    // Starting in version 1, \#LOCALNAME variable storage were added.
    if (version > 0)
      ar& local_names;
  }

  BOOST_SERIALIZATION_SPLIT_MEMBER()
};

BOOST_CLASS_VERSION(LocalMemory, 2)

// Class that encapsulates access to all integer and string
// memory. Multiple instances of this class will probably exist if
// save games are used.
//
// @note Because I use BSD code from xclannad in some of the methods
//       in this class, for licensing purposes, that code is separated
//       into memory_intmem.cc.
class Memory {
 public:
  // Default constructor; creates a Memory object which owns its own
  // GlobalMemory. Initial memory values are read from the passed in Gameexe
  // object.
  //
  // @note For now, we only read \#NAME and \#LOCALNAME variables, skipping any
  //       declaration of the form \#intvar[index] or \#strvar[index].
  Memory(RLMachine& machine, Gameexe& gamexe);

  // Creates an overlaid memory object. An overlay takes another Memory's
  // global memory. Local integer memory isn't initialized; it isn't even
  // memset zeroed out.
  //
  // Theoretically, |slot| is the save game slot to read local memory from, but
  // this is a misnomer. |slot| isn't used and appears only for the type
  // system(?).
  Memory(RLMachine& machine, int slot);

  ~Memory();

  // Returns the integer value of a certain memory location
  int GetIntValue(const libreallive::IntMemRef& ref);

  // Sets the value of a certain memory location
  void SetIntValue(const libreallive::IntMemRef& ref, int value);

  // Returns the string value of a string memory bank
  const std::string& GetStringValue(int type, int location);

  // Sets the string value of one of the string banks
  void SetStringValue(int type, int number, const std::string& value);

  // Name table functions:

  // Sets the local name slot index to name.
  void SetName(int index, const std::string& name);

  // Returns the local name slot index.
  const std::string& GetName(int index) const;

  // Sets the local name slot index to name.
  void SetLocalName(int index, const std::string& name);

  // Returns the local name slot index.
  const std::string& GetLocalName(int index) const;

  // Methods that record whether a piece of text has been read. RealLive
  // scripts have a piece of metadata called a kidoku marker which signifies if
  // the text between that and the next kidoku marker have been previously read.
  bool HasBeenRead(int scenario, int kidoku) const;
  void RecordKidoku(int scenario, int kidoku);

  // Accessors for serialization.
  GlobalMemory& global() { return *global_; }
  const GlobalMemory& global() const { return *global_; }
  LocalMemory& local() { return local_; }
  const LocalMemory& local() const { return local_; }

  // Commit changes in local memory. Unlike the code in src/Systems/ which
  // copies current values to shadow values, Memory clears a list of changes
  // that have been made since.
  void TakeSavepointSnapshot();

  // Converts a RealLive letter index (A-Z, AA-ZZ) to its numeric
  // equivalent. These letter indexes are used in \#NAME definitions.
  static int ConvertLetterIndexToInt(const std::string& value);

 private:
  // Connects the memory banks in local_ and in global_ into int_var.
  void ConnectIntVarPointers();

  // Input validating function to the {get,set}(Local)?Name set of functions.
  void CheckNameIndex(int index, const std::string& name) const;

  // Reads in default memory values from the passed in Gameexe, such as \#NAME
  // and \#LOCALNAME values.
  void InitializeDefaultValues(Gameexe& gameexe);

  // Pointer to the GlobalMemory structure. While there can (and will
  // be) multiple Memory instances (this is how we implement
  // GetSaveFlag), we don't really need to duplicate this data
  // structure and can simply pass a pointer to it.
  std::shared_ptr<GlobalMemory> global_;

  // Local memory to a save file
  LocalMemory local_;

  // Our owning machine. We keep this reference so we can ask for the current
  // stackframe.
  RLMachine& machine_;

  // Integer variable pointers. This redirect into Global and local
  // memory (as the case may be) allows us to overlay new views of
  // local memory without copying global memory.
  int* int_var[NUMBER_OF_INT_LOCATIONS];

  // Change records for original.
  std::map<int, int>* original_int_var[NUMBER_OF_INT_LOCATIONS];
};  // end of class Memory

// Implementation of getting an integer out of an array. Global because we need
// share this implementation with RLMachine which passes in the local stack
// frames bank for intL[] access.
int GetIntValue(const libreallive::IntMemRef& ref, int* bank);

// Implementation of setting an integer out of an array. Global because we need
// share this implementation with RLMachine which passes in the local stack
// frames bank for intL[] access.
void SetIntValue(const libreallive::IntMemRef& ref, int* bank, int value);

#endif  // SRC_MACHINE_MEMORY_H_
