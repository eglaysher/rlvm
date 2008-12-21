// -*- Mode: C++; tab-width:2; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi:tw=80:et:ts=2:sts=2
//
// -----------------------------------------------------------------------
//
// This file is part of RLVM, a RealLive virtual machine clone.
//
// -----------------------------------------------------------------------
//
// Copyright (C) 2008 Elliot Glaysher
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
// -----------------------------------------------------------------------

#include "Precompiled.hpp"

// -----------------------------------------------------------------------

#include "Systems/Base/RlBabelDLL.hpp"

#include "Systems/Base/System.hpp"
#include "Systems/Base/TextSystem.hpp"
#include "Systems/Base/TextWindow.hpp"
#include "MachineBase/RLMachine.hpp"
#include "libReallive/gameexe.h"
#include "libReallive/intmemref.h"

#include <iostream>
using std::cerr;
using std::endl;

// -----------------------------------------------------------------------
// RlBabelDLL
// -----------------------------------------------------------------------
int RlBabelDLL::callDLL(RLMachine& machine, int func, int arg1, int arg2,
                        int arg3, int arg4) {
  switch (func) {
    case dllInitialise:
      return initialize(arg1, arg2);
    case dllTextoutStart:
      if (arg1 == -1) {
        //        TextoutClear();
        return 1;
      }
      else {
        //        TextoutClear();
        return 0; //TextoutAdd(get_svar(arg1));
      }
    case dllTextoutAppend:
      //      return TextoutAdd(get_svar(arg1));
      return 0;
    case dllTextoutGetChar:
      //      return TextoutGetChar(arg1, arg2);
      return 0;
    case dllTextoutNewScreen:
      //      return StartNewScreen((char*) get_svar(arg1));
      return 0;
    case dllSetNameMod: {
      boost::shared_ptr<TextWindow> textWindow = getWindow(machine, arg1);
      int original_mod = textWindow->nameMod();
      textWindow->setNameMod(arg2);
      return original_mod;
    }
    case dllGetNameMod:
      return getWindow(machine, arg1)->nameMod();
    case dllGetTextWindow:
      return getWindow(machine, -1)->windowNumber();
    case dllSetWindowName:
      return 0;
//       return SetCurrentWindowName(get_svar(arg1), endSetWindowName,
//                                   NULL, NULL, NULL);
    case endSetWindowName:
      return 0;
//      return EndCurrentWindowName();
    case endGetCharWinNam:
//       EndCurrentWindowName();
//       return TextoutGetChar(arg1, arg2);
      return 0;
    case dllClearGlosses:
//      return ClearGlosses();
      return 0;
    case dllNewGloss:
//      return NewGloss();
      return 0;
    case dllAddGloss:
//       return AddGloss(get_svar(arg1));
      return 0;
    case dllTestGlosses:
//       return TestGlosses(arg1, arg2, get_svar(arg3), arg4);
      return 0;
    case dllGetRCommandMod: {
      // TODO(erg): This needs formatting badly! Needs to make window 3 digits?
      // int window = arg1 >= 0 ? arg1 : interpreter->getCurrentWindow();
      int window = arg1;
      return machine.system().gameexe()("WINDOW")(window)("R_COMMAND_MOD");
    }
    default:
      return 0;
  }
}

// -----------------------------------------------------------------------

int RlBabelDLL::initialize(int dllno, int windname) {
  // rlBabel hangs onto the dll index and uses it for something in his
  // SetCurrentWindowName implementation.
  return 1;
}

// -----------------------------------------------------------------------

IntReferenceIterator RlBabelDLL::getIvar(RLMachine& machine, int addr) {
  int bank = (addr >> 16) % 26;
  int location = addr & 0xffff;
  return IntReferenceIterator(&(machine.memory()), bank, location);
}

// -----------------------------------------------------------------------

StringReferenceIterator RlBabelDLL::getSvar(RLMachine& machine, int addr) {
  Memory* m = &(machine.memory());
  int bank = addr >> 16;
  int locaiton = addr & 0xfff;

  switch (bank) {
    case libReallive::STRS_LOCATION:
    case libReallive::STRM_LOCATION:
      return StringReferenceIterator(m, bank, addr & 0xfff);
    case libReallive::STRK_LOCATION:
      // To be bug for bug compatible with the real rlBabel so people don't
      // start targetting rlvm.
      cerr << "strK[] support is not implemented: falling back on strS[0]"
           << endl;
      break;
    default: {
      cerr << "Invalid string address; falling back on strS[0]" << endl;
      break;
    }
  }

  // Error.
  return StringReferenceIterator(m, libReallive::STRS_LOCATION, 0);
}

// -----------------------------------------------------------------------

boost::shared_ptr<TextWindow> RlBabelDLL::getWindow(RLMachine& machine,
                                                    int id) {
  TextSystem& text_system = machine.system().text();
  if (id >= 0) {
    return text_system.textWindow(machine, id);
  } else {
    return text_system.currentWindow(machine);
  }
}
