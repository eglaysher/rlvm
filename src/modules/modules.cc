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
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
//
// -----------------------------------------------------------------------

#include "machine/rlmachine.h"

#include "modules/module_bgm.h"
#include "modules/module_bgr.h"
#include "modules/module_dll.h"
#include "modules/module_debug.h"
#include "modules/module_event_loop.h"
#include "modules/module_g00.h"
#include "modules/module_gan.h"
#include "modules/module_grp.h"
#include "modules/module_jmp.h"
#include "modules/module_koe.h"
#include "modules/module_mem.h"
#include "modules/module_mov.h"
#include "modules/module_msg.h"
#include "modules/module_obj_creation.h"
#include "modules/module_obj_fg_bg.h"
#include "modules/module_obj_management.h"
#include "modules/module_obj_getters.h"
#include "modules/module_os.h"
#include "modules/module_pcm.h"
#include "modules/module_refresh.h"
#include "modules/module_scr.h"
#include "modules/module_se.h"
#include "modules/module_sel.h"
#include "modules/module_shk.h"
#include "modules/module_shl.h"
#include "modules/module_str.h"
#include "modules/module_sys.h"
#include "systems/base/graphics_system.h"
#include "systems/base/system.h"

void AddAllModules(RLMachine& rlmachine) {
  // Attatch the modules for some commands
  rlmachine.AttachModule(new BgmModule);
  rlmachine.AttachModule(new BgrModule);
  rlmachine.AttachModule(new BraModule);
  rlmachine.AttachModule(new ChildGanBgModule);
  rlmachine.AttachModule(new ChildGanFgModule);
  rlmachine.AttachModule(new ChildObjBgCreationModule);
  rlmachine.AttachModule(new ChildObjBgGettersModule);
  rlmachine.AttachModule(new ChildObjBgManagement);
  rlmachine.AttachModule(new ChildObjBgModule);
  rlmachine.AttachModule(new ChildObjFgCreationModule);
  rlmachine.AttachModule(new ChildObjFgGettersModule);
  rlmachine.AttachModule(new ChildObjFgManagement);
  rlmachine.AttachModule(new ChildObjFgModule);
  rlmachine.AttachModule(new ChildObjRangeBgModule);
  rlmachine.AttachModule(new ChildObjRangeFgModule);
  rlmachine.AttachModule(new DLLModule);
  rlmachine.AttachModule(new DebugModule);
  rlmachine.AttachModule(new EventLoopModule);
  rlmachine.AttachModule(new G00Module);
  rlmachine.AttachModule(new GanBgModule);
  rlmachine.AttachModule(new GanFgModule);
  rlmachine.AttachModule(new GrpModule);
  rlmachine.AttachModule(new JmpModule);
  rlmachine.AttachModule(new KoeModule);
  rlmachine.AttachModule(new LayeredShakingModule);
  rlmachine.AttachModule(new MemModule);
  rlmachine.AttachModule(new MovModule);
  rlmachine.AttachModule(new MsgModule);
  rlmachine.AttachModule(new ObjBgCreationModule);
  rlmachine.AttachModule(new ObjBgGettersModule);
  rlmachine.AttachModule(new ObjBgManagement);
  rlmachine.AttachModule(new ObjBgModule);
  rlmachine.AttachModule(new ObjFgCreationModule);
  rlmachine.AttachModule(new ObjFgGettersModule);
  rlmachine.AttachModule(new ObjFgManagement);
  rlmachine.AttachModule(new ObjFgModule);
  rlmachine.AttachModule(new ObjManagement);
  rlmachine.AttachModule(new ObjRangeBgModule);
  rlmachine.AttachModule(new ObjRangeFgModule);
  rlmachine.AttachModule(new OsModule);
  rlmachine.AttachModule(new PcmModule);
  rlmachine.AttachModule(new RefreshModule);
  rlmachine.AttachModule(new ScrModule);
  rlmachine.AttachModule(new SeModule);
  rlmachine.AttachModule(new SelModule);
  rlmachine.AttachModule(new ShakingModule);
  rlmachine.AttachModule(new StrModule);
  rlmachine.AttachModule(new SysModule);
}
