// -*- Mode: C++; tab-width:2; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi:tw=80:et:ts=2:sts=2
//
// -----------------------------------------------------------------------
//
// This file is part of RLVM, a RealLive virtual machine clone.
//
// -----------------------------------------------------------------------
//
// Copyright (c) 2004-2007  Kazunori "jagarl" Ueno
// Copyright (C) 2007 Elliot Glaysher
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions
// are met:
// 1. Redistributions of source code must retain the above copyright
//    notice, this list of conditions and the following disclaimer.
// 2. Redistributions in binary form must reproduce the above copyright
//    notice, this list of conditions and the following disclaimer in the
//    documentation and/or other materials provided with the distribution.
// 3. The name of the author may not be used to endorse or promote products
//    derived from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
// IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
// OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
// INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
// NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
// THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// -----------------------------------------------------------------------
//
// This file contains memory functions that were taken from xclannad,
// and are thus under the 3-clause BSD instead of the GPL used in the
// rest of rlvm.
//
// -----------------------------------------------------------------------

#include <sstream>
#include <string>

#include "machine/memory.h"
#include "machine/rlmachine.h"
#include "utilities/exception.h"
#include "libreallive/intmemref.h"

using libreallive::IntMemRef;

namespace {

// Helper function that throws errors for illegal memory access
void throwIllegalIndex(const IntMemRef& ref, const std::string& function) {
  std::ostringstream ss;
  ss << "Invalid memory access " << ref << " in " << function;
  throw rlvm::Exception(ss.str());
}

void saveOriginalValue(int* bank,
                       std::map<int, int>* original_bank,
                       int location) {
  if (bank && original_bank) {
    std::map<int, int>::iterator it = original_bank->find(location);
    if (it == original_bank->end()) {
      original_bank->emplace(location, bank[location]);
    }
  }
}

}  // namespace

int Memory::GetIntValue(const IntMemRef& ref) {
  int type = ref.type();
  int index = ref.bank();
  int location = ref.location();

  int* bank = NULL;
  if (index == 8) {
    bank = machine_.CurrentIntLBank();
  } else if (index < 0 || index > NUMBER_OF_INT_LOCATIONS) {
    throwIllegalIndex(ref, "RLMachine::GetIntValue()");
  } else {
    bank = int_var[index];
  }

  if (type == 0) {
    // A[]..G[], Z[] を直に読む
    if ((unsigned int)(location) >= 2000)
      throwIllegalIndex(ref, "RLMachine::GetIntValue()");

    return bank[location];
  } else {
    // Ab[]..G4b[], Z8b[] などを読む
    int factor = 1 << (type - 1);
    int eltsize = 32 / factor;
    if ((unsigned int)(location) >= (64000u / factor))
      throwIllegalIndex(ref, "RLMachine::GetIntValue()");

    return (bank[location / eltsize] >> ((location % eltsize) * factor)) &
           ((1 << factor) - 1);
  }
}

void Memory::SetIntValue(const IntMemRef& ref, int value) {
  int type = ref.type();
  int index = ref.bank();
  int location = ref.location();

  int* bank = NULL;
  std::map<int, int>* original_bank = NULL;
  if (index == 8) {
    bank = machine_.CurrentIntLBank();
  } else if (index < 0 || index > NUMBER_OF_INT_LOCATIONS) {
    throwIllegalIndex(ref, "RLMachine::SetIntValue()");
  } else {
    bank = int_var[index];
    original_bank = original_int_var[index];
  }

  if (type == 0) {
    // A[]..G[], Z[] を直に書く
    if ((unsigned int)(location) >= 2000)
      throwIllegalIndex(ref, "RLMachine::SetIntValue()");
    saveOriginalValue(bank, original_bank, location);
    bank[location] = value;
  } else {
    // Ab[]..G4b[], Z8b[] などを書く
    int factor = 1 << (type - 1);
    int eltsize = 32 / factor;
    int eltmask = (1 << factor) - 1;
    int shift = (location % eltsize) * factor;
    if ((unsigned int)(location) >= (64000u / factor))
      throwIllegalIndex(ref, "RLMachine::SetIntValue()");

    saveOriginalValue(bank, original_bank, location / eltsize);
    bank[location / eltsize] =
        (bank[location / eltsize] & ~(eltmask << shift)) | (value & eltmask)
                                                               << shift;
  }
}
