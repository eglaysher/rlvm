// This file is part of RLVM, a RealLive virutal machine clone.
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

#include "MachineBase/RLMachine.hpp"

#include "Modules/Module_Jmp.hpp"
#include "Modules/Module_Sys.hpp"
#include "Modules/Module_Str.hpp"
#include "Modules/Module_Mem.hpp"
#include "Modules/Module_Grp.hpp"
#include "Modules/Module_Msg.hpp"
#include "Modules/Module_ObjPosDims.hpp"
#include "Modules/Module_ObjCreation.hpp"
#include "Modules/Module_ObjFgBg.hpp"
#include "Modules/Module_ObjManagement.hpp"
#include "Modules/Module_Refresh.hpp"
#include "Modules/Module_Scr.hpp"

#include "Systems/Base/System.hpp"
#include "Systems/Base/GraphicsSystem.hpp"
#include "Systems/Base/EventSystem.hpp"

void addAllModules(RLMachine& rlmachine)
{
  // Attatch the modules for some commands
  rlmachine.attatchModule(new JmpModule);
  rlmachine.attatchModule(new SysModule(rlmachine.system().graphics()));
  rlmachine.attatchModule(new StrModule);
  rlmachine.attatchModule(new MemModule);
  rlmachine.attatchModule(new MsgModule);
  rlmachine.attatchModule(new GrpModule);
  rlmachine.attatchModule(new ObjCopyFgToBg);
  rlmachine.attatchModule(new ObjPosDimsModule);
  rlmachine.attatchModule(new ObjFgModule);
  rlmachine.attatchModule(new ObjBgModule);
  rlmachine.attatchModule(new ObjRangeFgModule);
  rlmachine.attatchModule(new ObjRangeBgModule);
  rlmachine.attatchModule(new ObjFgCreationModule);
  rlmachine.attatchModule(new ObjBgCreationModule);
  rlmachine.attatchModule(new ObjFgManagement);
  rlmachine.attatchModule(new ObjBgManagement);
  rlmachine.attatchModule(new RefreshModule);
  rlmachine.attatchModule(new ScrModule);
}
