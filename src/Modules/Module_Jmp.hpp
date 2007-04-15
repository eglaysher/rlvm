// This file is part of RLVM, a RealLive virtual machine clone.
//
// -----------------------------------------------------------------------
//
// Copyright (C) 2006 Elliot Glaysher
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

#ifndef __Module_Jmp_hpp__
#define __Module_Jmp_hpp__

/**
 * @file   Module_Jmp.cpp
 * @author Elliot Glaysher
 * @date   Fri Sep 22 20:10:16 2006
 * @ingroup ModulesOpcodes
 * @brief  Declaration of JmpModule
 */

#include "MachineBase/RLModule.hpp"

/**
 * Contains functions for mod<0:1>, Jmp.
 * 
 * @ingroup ModuleJmp
 */
class JmpModule : public RLModule {
public:
  JmpModule();
};

#endif
