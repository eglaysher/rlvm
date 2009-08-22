// -*- Mode: C++; tab-width:2; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi:tw=80:et:ts=2:sts=2
//
// -----------------------------------------------------------------------
//
// This file is part of RLVM, a RealLive virtual machine clone.
//
// -----------------------------------------------------------------------
//
// Copyright (C) 2006 Elliot Glaysher
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
//
// -----------------------------------------------------------------------

#include "Precompiled.hpp"

// -----------------------------------------------------------------------

#include "MachineBase/RLMachine.hpp"

#include "Modules/Module_Jmp.hpp"
#include "Modules/Module_EventLoop.hpp"
#include "Modules/Module_Mov.hpp"
#include "Modules/Module_Sys.hpp"
#include "Modules/Module_Str.hpp"
#include "Modules/Module_Mem.hpp"
#include "Modules/Module_Grp.hpp"
#include "Modules/Module_Msg.hpp"
#include "Modules/Module_Bgm.hpp"
#include "Modules/Module_Bgr.hpp"
#include "Modules/Module_Pcm.hpp"
#include "Modules/Module_Se.hpp"
#include "Modules/Module_Koe.hpp"
#include "Modules/Module_ObjPosDims.hpp"
#include "Modules/Module_ObjCreation.hpp"
#include "Modules/Module_ObjFgBg.hpp"
#include "Modules/Module_ObjManagement.hpp"
#include "Modules/Module_Os.hpp"
#include "Modules/Module_Refresh.hpp"
#include "Modules/Module_Scr.hpp"
#include "Modules/Module_Debug.hpp"
#include "Modules/Module_Gan.hpp"
#include "Modules/Module_Sel.hpp"
#include "Modules/Module_Shl.hpp"
#include "Modules/Module_Shk.hpp"
#include "Modules/Module_DLL.hpp"

#include "Systems/Base/System.hpp"
#include "Systems/Base/GraphicsSystem.hpp"

void addAllModules(RLMachine& rlmachine) {
  // Attatch the modules for some commands
  rlmachine.attachModule(new JmpModule);
  rlmachine.attachModule(new DLLModule);
  rlmachine.attachModule(new EventLoopModule);
  rlmachine.attachModule(new SelModule);
  rlmachine.attachModule(new SysModule);
  rlmachine.attachModule(new MovModule);
  rlmachine.attachModule(new StrModule);
  rlmachine.attachModule(new MemModule);
  rlmachine.attachModule(new MsgModule);
  rlmachine.attachModule(new GrpModule);
  rlmachine.attachModule(new BgmModule);
  rlmachine.attachModule(new BgrModule);
  rlmachine.attachModule(new PcmModule);
  rlmachine.attachModule(new SeModule);
  rlmachine.attachModule(new KoeModule);
  rlmachine.attachModule(new ObjCopyFgToBg);

  rlmachine.attachModule(new ObjFgPosDimsModule);
  rlmachine.attachModule(new ObjBgPosDimsModule);
  rlmachine.attachModule(new ChildObjFgPosDimsModule);
  rlmachine.attachModule(new ChildObjBgPosDimsModule);

  rlmachine.attachModule(new ObjFgModule);
  rlmachine.attachModule(new ObjBgModule);
  rlmachine.attachModule(new ChildObjFgModule);
  rlmachine.attachModule(new ChildObjBgModule);
  rlmachine.attachModule(new ObjRangeFgModule);
  rlmachine.attachModule(new ObjRangeBgModule);

  rlmachine.attachModule(new ObjFgCreationModule);
  rlmachine.attachModule(new ObjBgCreationModule);
  rlmachine.attachModule(new ChildObjFgCreationModule);
  rlmachine.attachModule(new ChildObjBgCreationModule);

  rlmachine.attachModule(new ObjFgManagement);
  rlmachine.attachModule(new ObjBgManagement);
  rlmachine.attachModule(new ChildObjFgManagement);
  rlmachine.attachModule(new ChildObjBgManagement);

  rlmachine.attachModule(new OsModule);
  rlmachine.attachModule(new RefreshModule);
  rlmachine.attachModule(new ScrModule);
  rlmachine.attachModule(new DebugModule);
  rlmachine.attachModule(new GanFgModule);
  rlmachine.attachModule(new GanBgModule);
  rlmachine.attachModule(new ChildGanFgModule);
  rlmachine.attachModule(new ChildGanBgModule);
  rlmachine.attachModule(new LayeredShakingModule);
  rlmachine.attachModule(new ShakingModule);
}
