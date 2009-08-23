// -*- Mode: C++; tab-width:2; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi:tw=80:et:ts=2:sts=2
//
// -----------------------------------------------------------------------
//
// This file is part of RLVM, a RealLive virtual machine clone.
//
// -----------------------------------------------------------------------
//
// This file contains memory functions that were taken from xclannad,
// and are thus under the 3-clause BSD instead of the GPL used in the
// rest of rlvm.
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

#include "Precompiled.hpp"

// -----------------------------------------------------------------------

#include <sstream>
#include <string>

#include "MachineBase/Memory.hpp"
#include "Utilities/Exception.hpp"
#include "libReallive/intmemref.h"

// -----------------------------------------------------------------------

using namespace std;
using libReallive::IntMemRef;

// -----------------------------------------------------------------------

/**
 * Helper function that throws errors for illegal memory access
 *
 * @param location The illegal index that was accessed
 * @see RLMachine::get_int_value
 */
static void throwIllegalIndex(const IntMemRef& ref,
                              const std::string& function) {
  ostringstream ss;
  ss << "Invalid memory access " << ref << " in " << function;
  throw rlvm::Exception(ss.str());
}

// -----------------------------------------------------------------------

int Memory::getIntValue(const IntMemRef& ref) {
  int type = ref.type();
  int index = ref.bank();
  int location = ref.location();

  if (index > NUMBER_OF_INT_LOCATIONS)
      throwIllegalIndex(ref, "RLMachine::getIntValue()");

  if (type == 0) {
    // A[]..G[], Z[] を直に読む
    if ((unsigned int)(location) >= 2000)
      throwIllegalIndex(ref, "RLMachine::getIntValue()");

    return int_var[index][location];
  } else {
    // Ab[]..G4b[], Z8b[] などを読む
    int factor = 1 << (type - 1);
    int eltsize = 32 / factor;
    if ((unsigned int)(location) >= (64000u / factor))
      throwIllegalIndex(ref, "RLMachine::getIntValue()");

    return (int_var[index][location / eltsize] >>
            ((location % eltsize) * factor)) & ((1 << factor) - 1);
  }
}

// -----------------------------------------------------------------------

void Memory::setIntValue(const IntMemRef& ref, int value) {
  int type = ref.type();
  int index = ref.bank();
  int location = ref.location();

  if (index < 0 || index > NUMBER_OF_INT_LOCATIONS) {
    throwIllegalIndex(ref, "RLMachine::setIntValue()");
  }
  if (type == 0) {
    // A[]..G[], Z[] を直に書く
    if ((unsigned int)(location) >= 2000)
      throwIllegalIndex(ref, "RLMachine::setIntValue()");
    int_var[index][location] = value;
  } else {
    // Ab[]..G4b[], Z8b[] などを書く
    int factor = 1 << (type - 1);
    int eltsize = 32 / factor;
    int eltmask = (1 << factor) - 1;
    int shift = (location % eltsize) * factor;
    if ((unsigned int)(location) >= (64000u / factor))
      throwIllegalIndex(ref, "RLMachine::setIntValue()");

    int_var[index][location / eltsize] =
      (int_var[index][location / eltsize] & ~(eltmask << shift))
      | (value & eltmask) << shift;
  }
}
