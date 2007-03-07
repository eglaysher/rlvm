// This file is part of RLVM, a RealLive virtual machine clone.
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

#ifndef __Module_Msg_hpp__
#define __Module_Msg_hpp__

/**
 * @file   Module_Msg.hpp
 * @author Elliot Glaysher
 * @date   Tue Nov 14 21:03:18 2006
 * 
 * @brief  Module that deals with textout
 */

#include "MachineBase/RLModule.hpp"

/**
 * Contains functions for mod<0:3>, Msg.
 * 
 * @ingroup ModuleMessage
 */
class MsgModule : public RLModule {
public:
  MsgModule();
};

#endif
