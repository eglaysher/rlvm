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

#include "gtest/gtest.h"

#include <iostream>
#include <utility>
#include <string>
#include <vector>

#include "machine/memory.h"
#include "machine/rlmachine.h"
#include "machine/serialization.h"
#include "modules/module_str.h"
#include "utilities/exception.h"
#include "libreallive/intmemref.h"
#include "test_utils.h"

using namespace std;
using namespace libreallive;

class RLMachineTest : public FullSystemTest {
 protected:
  void setIntMemoryCountingFrom(RLMachine& saveMachine,
                                const vector<pair<int, char>>& banks,
                                int count) {
    for (vector<pair<int, char>>::const_iterator it = banks.begin();
         it != banks.end();
         ++it) {
      for (int i = 0; i < SIZE_OF_MEM_BANK; ++i) {
        saveMachine.SetIntValue(IntMemRef(it->second, i), count);
        count++;
      }
    }
  }

  void setStrMemoryCountingFrom(RLMachine& saveMachine, int type, int count) {
    for (int i = 0; i < SIZE_OF_MEM_BANK; ++i) {
      saveMachine.SetStringValue(type, i, std::to_string(count));
      count++;
    }
  }

  void verifyIntMemoryCountingFrom(RLMachine& loadMachine,
                                   const vector<pair<int, char>>& banks,
                                   int count) {
    for (vector<pair<int, char>>::const_iterator it = banks.begin();
         it != banks.end();
         ++it) {
      for (int i = 0; i < SIZE_OF_MEM_BANK; ++i) {
        EXPECT_EQ(count, loadMachine.GetIntValue(IntMemRef(it->second, i)));
        count++;
      }
    }
  }

  void verifyStrMemoryCountingFrom(RLMachine& loadMachine,
                                   int type,
                                   int count) {
    for (int i = 0; i < SIZE_OF_MEM_BANK; ++i) {
      EXPECT_EQ(std::to_string(count), loadMachine.GetStringValue(type, i));
      count++;
    }
  }
};

TEST_F(RLMachineTest, RejectsDoubleAttachs) {
  rlmachine.AttachModule(new StrModule);
  EXPECT_THROW({ rlmachine.AttachModule(new StrModule); }, rlvm::Exception);
}

TEST_F(RLMachineTest, ReturnFromFarcallMismatch) {
  EXPECT_THROW({ rlmachine.ReturnFromFarcall(); }, rlvm::Exception);
}

TEST_F(RLMachineTest, ReturnFromGosubMismatch) {
  EXPECT_THROW({ rlmachine.ReturnFromGosub(); }, rlvm::Exception);
}

TEST_F(RLMachineTest, Halts) {
  EXPECT_TRUE(!rlmachine.halted()) << "Machine does not start halted.";
  rlmachine.Halt();
  EXPECT_TRUE(rlmachine.halted()) << "Machine is halted.";
}

TEST_F(RLMachineTest, RegisterStore) {
  for (int i = 0; i < 10; ++i) {
    rlmachine.set_store_register(i);
    EXPECT_EQ(i, rlmachine.store_register());
  }
}

// Test valid string memory access.
TEST_F(RLMachineTest, StringMemory) {
  vector<int> types = {STRK_LOCATION, STRM_LOCATION, STRS_LOCATION};

  for (vector<int>::const_iterator it = types.begin(); it != types.end();
       ++it) {
    const string str = "Stored at " + std::to_string(*it);
    rlmachine.SetStringValue(*it, 0, str);
    EXPECT_EQ(str, rlmachine.GetStringValue(*it, 0));
  }
}

// Test error-inducing, string memory access.
TEST_F(RLMachineTest, StringMemoryErrors) {
  EXPECT_THROW({ rlmachine.SetStringValue(STRM_LOCATION, 2000, "Blah"); },
               rlvm::Exception);
  EXPECT_THROW({ rlmachine.GetStringValue(STRM_LOCATION, 2000); },
               rlvm::Exception);
}

// Test valid integer access of all types.
//
// For reference to understand the following
// signed 32-bit integer: 10281 (0010 1000 0010 1001b)
//        8-bit integers: 0,0,40,41
//
// For    8-bit integers: 38,39,40,41
TEST_F(RLMachineTest, IntegerMemory) {
  vector<char> banks = {'A', 'B', 'C', 'D', 'E', 'F', 'G', 'L', 'Z'};

  const int in8b[] = {38, 39, 40, 41};
  const int base = (in8b[0] << 24) | (in8b[1] << 16) | (in8b[2] << 8) | in8b[3];

  const int rc = 8;
  const int final = (rc << 24) | (rc << 16) | (rc << 8) | rc;

  for (vector<char>::const_iterator it = banks.begin(); it != banks.end();
       ++it) {
    IntMemRef wordRef(*it, 0);
    rlmachine.SetIntValue(wordRef, base);
    EXPECT_EQ(base, rlmachine.GetIntValue(wordRef))
        << "Didn't record full value";

    for (int i = 0; i < 4; ++i) {
      IntMemRef comp(*it, "8b", i);
      EXPECT_EQ(in8b[3 - i], rlmachine.GetIntValue(comp))
          << "Could get partial value";

      rlmachine.SetIntValue(comp, rc);
    }

    EXPECT_EQ(final, rlmachine.GetIntValue(wordRef))
        << "Changing the components didn't change the full value!";
  }
}

TEST_F(RLMachineTest, IntegerMemoryErrors) {
  EXPECT_THROW({ rlmachine.GetIntValue(IntMemRef(10, 0, 0)); },
               rlvm::Exception);
  EXPECT_NO_THROW({ rlmachine.GetIntValue(IntMemRef('A', 1999)); });  // NOLINT
  EXPECT_THROW({ rlmachine.GetIntValue(IntMemRef('A', 2000)); },
               rlvm::Exception);
}

TEST_F(RLMachineTest, CheckNameLetterIndex) {
  EXPECT_EQ(0, Memory::ConvertLetterIndexToInt("A"));
  EXPECT_EQ(25, Memory::ConvertLetterIndexToInt("Z"));
  EXPECT_EQ(26, Memory::ConvertLetterIndexToInt("AA"));
  EXPECT_EQ(52, Memory::ConvertLetterIndexToInt("BA"));
  EXPECT_EQ(701, Memory::ConvertLetterIndexToInt("ZZ"));
}

TEST_F(RLMachineTest, NameStorage) {
  Memory& memory = rlmachine.memory();
  EXPECT_EQ("Bob", memory.GetName(Memory::ConvertLetterIndexToInt("A")));
  EXPECT_EQ("Alice",
            memory.GetLocalName(Memory::ConvertLetterIndexToInt("AB")));
}

TEST_F(RLMachineTest, Serialization) {
  stringstream ss;
  libreallive::Archive arc(locateTestCase("Module_Str_SEEN/strcpy_0.TXT"));
  // Save data
  {
    RLMachine saveMachine(system, arc);
    setIntMemoryCountingFrom(saveMachine, GLOBAL_INTEGER_BANKS, 0);
    setStrMemoryCountingFrom(saveMachine, STRM_LOCATION, 0);

    Serialization::saveGlobalMemoryTo(ss, saveMachine);
  }

  // Load data
  {
    RLMachine loadMachine(system, arc);
    Serialization::loadGlobalMemoryFrom(ss, loadMachine);
    verifyIntMemoryCountingFrom(loadMachine, GLOBAL_INTEGER_BANKS, 0);
    verifyStrMemoryCountingFrom(loadMachine, STRM_LOCATION, 0);
  }
}

// Tests serialization of the kidoku table.
TEST_F(RLMachineTest, SerializationOfKidoku) {
  stringstream ss;
  libreallive::Archive arc(locateTestCase("Module_Str_SEEN/strcpy_0.TXT"));

  // Save data
  {
    RLMachine saveMachine(system, arc);

    for (int i = 0; i < 10; i += 2) {
      saveMachine.memory().RecordKidoku(5, i);
    }

    Serialization::saveGlobalMemoryTo(ss, saveMachine);
  }

  // Load data
  {
    RLMachine loadMachine(system, arc);
    Serialization::loadGlobalMemoryFrom(ss, loadMachine);

    for (int i = 0; i < 10; i++) {
      EXPECT_EQ(!(i % 2), loadMachine.memory().HasBeenRead(5, i))
          << "Didn't save kidoku table correctly!";
    }
  }
}

TEST_F(RLMachineTest, SerializationOfSavepointValues) {
  stringstream ss;
  libreallive::Archive arc(locateTestCase("Module_Str_SEEN/strcpy_0.TXT"));
  // Save data
  {
    RLMachine saveMachine(system, arc);

    // Write the values we're going to check for.
    setIntMemoryCountingFrom(saveMachine, LOCAL_INTEGER_BANKS, 0);
    setStrMemoryCountingFrom(saveMachine, STRS_LOCATION, 0);
    saveMachine.MarkSavepoint();

    // Verify that those values are written.
    verifyIntMemoryCountingFrom(saveMachine, LOCAL_INTEGER_BANKS, 0);
    verifyStrMemoryCountingFrom(saveMachine, STRS_LOCATION, 0);

    // Scribble different values on top, immediately check to make sure we can
    // still read them, but don't commit them.
    setIntMemoryCountingFrom(saveMachine, LOCAL_INTEGER_BANKS, 5);
    setStrMemoryCountingFrom(saveMachine, STRS_LOCATION, 5);
    verifyIntMemoryCountingFrom(saveMachine, LOCAL_INTEGER_BANKS, 5);
    verifyStrMemoryCountingFrom(saveMachine, STRS_LOCATION, 5);

    Serialization::saveGameTo(ss, saveMachine);
  }

  // Load data. Assure that we only have the committed values.
  {
    RLMachine loadMachine(system, arc);
    Serialization::loadGameFrom(ss, loadMachine);
    verifyIntMemoryCountingFrom(loadMachine, LOCAL_INTEGER_BANKS, 0);
    verifyStrMemoryCountingFrom(loadMachine, STRS_LOCATION, 0);
  }
}
