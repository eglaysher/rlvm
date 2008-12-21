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

#ifndef __RlBabelDLL_hpp__
#define __RlBabelDLL_hpp__

#include "MachineBase/RealLiveDLL.hpp"

/**
 * Possible commands sent to the rlBabel DLL from the code. These will be
 * passed in as the first integer argument (func) to RlBabelDLL::callDLL().
 */
enum dllFunction {
	dllInitialise       =   0,
	dllTextoutStart     =  10,
	dllTextoutAppend    =  11,
	dllTextoutGetChar   =  12,
	dllTextoutNewScreen =  13,
	dllClearGlosses     =  20,
	dllNewGloss         =  21,
	dllAddGloss         =  22,
	dllTestGlosses      =  23,
	endSetWindowName    =  98,
	endGetCharWinNam    =  99,
	dllSetNameMod       = 100,
	dllGetNameMod       = 101,
	dllSetWindowName    = 102,
	dllGetTextWindow    = 103,
	dllGetRCommandMod   = 104,
	dllMessageBox       = 105
};

// -----------------------------------------------------------------------

/**
 * Return codes from the above functions sent back to the RealLive bytecode.
 */
enum getcReturn {
  getcError,
  getcEndOfString,
  getcPrintChar,
  getcNewLine,
  getcNewScreen,
  getcSetIndent,
  getcClearIndent,
  getcBeginGloss
};

// -----------------------------------------------------------------------

/**
 *
 */
class RlBabelDLL : public RealLiveDLL {
 public:
  // Overridden from RealLiveDLL:

  // Main entrypoint to the "DLL". It's a giant switch function that handles
  // all the commands that Haeleth added with rlBabel.
  virtual int callDLL(RLMachine& machine, int func, int arg1, int arg2,
                      int arg3, int arg4);

 private:
  /// Initializes the DLL.
  int initialize(int dllno, int windname);

  /// Transform one of rlBabel's integer addresses into an iterator to the
  /// corresponding piece of integer memory.
  IntReferenceIterator getIvar(RLMachine& machine, int addr);

  /// Transform one of rlBabel's integer addresses into an iterator to the
  /// corresponding piece of integer memory.
  StringReferenceIterator getSvar(RLMachine& machine, int addr);
};  // end of class RlBabelDll


#endif
