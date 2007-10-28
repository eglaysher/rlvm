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

#ifndef __Memory_hpp__
#define __Memory_hpp__

#include <string>
#include <vector>
#include <map>
#include "libReallive/intmemref.h"
#include <boost/shared_ptr.hpp>

// -----------------------------------------------------------------------

const int NUMBER_OF_INT_LOCATIONS = 9;
const int SIZE_OF_MEM_BANK = 2000;
const int SIZE_OF_INT_PASSING_MEM = 40;

typedef std::vector<std::pair<int, char> > IntegerBank_t;
extern const IntegerBank_t LOCAL_INTEGER_BANKS;
extern const IntegerBank_t GLOBAL_INTEGER_BANKS;

// -----------------------------------------------------------------------

class RLMachine;

// -----------------------------------------------------------------------

struct GlobalMemory
{
  GlobalMemory();

  int intG[SIZE_OF_MEM_BANK];
  int intZ[SIZE_OF_MEM_BANK];

  std::string strM[SIZE_OF_MEM_BANK];
};

// -----------------------------------------------------------------------

struct dont_initialize { };

struct LocalMemory
{
  LocalMemory();

  /**
   * Constructor that prevents the memory banks from being memset
   * (since they'll be overwritten entirely by the thawing process.
   */
  LocalMemory(dont_initialize);

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
};

// -----------------------------------------------------------------------

/**
 * Class that encapsulates access to memory.
 */
class Memory
{
private:
  /**
   * Pointer to the GlobalMemory structure. While there can (and will
   * be) multiple Memory instances (this is how we implement
   * GetSaveFlag), we don't really need to duplicate this data
   * structure and can simply pass a pointer to it.
   */
  boost::shared_ptr<GlobalMemory> m_global;

  /// Local memory to a save file
  LocalMemory m_local;
  
  /// Integer variable pointers. This redirect into Global and local
  /// memory (as the case may be) allows us to overlay new views of
  /// local memory without copying global memory.
  int* intVar[NUMBER_OF_INT_LOCATIONS];

  /** 
   * Connects the memory banks in m_local and in m_global into intVar.
   */
  void connectIntVarPointers();

public: 
  /**
   * Default constructor; creates a Memory object which owns its own
   * GlobalMemory.
   */
  Memory();

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
   */
  int getIntValue(const libReallive::IntMemRef& ref);    

  /** 
   * Sets the value of a certain memory location
   * 
   * @param type The memory bank/access method to access from
   * @param number The offset into that memory bank
   * @param value The new value
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


  GlobalMemory& global() { return *m_global; }
  const GlobalMemory& global() const { return *m_global; }
  LocalMemory& local() { return m_local; }
  const LocalMemory& local() const { return m_local; }



};	// end of class Memory


#endif
