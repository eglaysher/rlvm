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
#include "Systems/Base/GraphicsSystem.hpp"
#include "Systems/Base/System.hpp"

void addAllModules(RLMachine& rlmachine) {
  // Attatch the modules for some commands
  rlmachine.attachModule(new BgmModule);
  rlmachine.attachModule(new BgrModule);
  rlmachine.attachModule(new ChildGanBgModule);
  rlmachine.attachModule(new ChildGanFgModule);
  rlmachine.attachModule(new ChildObjBgCreationModule);
  rlmachine.attachModule(new ChildObjBgManagement);
  rlmachine.attachModule(new ChildObjBgModule);
  rlmachine.attachModule(new ChildObjBgGettersModule);
  rlmachine.attachModule(new ChildObjFgCreationModule);
  rlmachine.attachModule(new ChildObjFgManagement);
  rlmachine.attachModule(new ChildObjFgModule);
  rlmachine.attachModule(new ChildObjFgGettersModule);
  rlmachine.attachModule(new ChildObjRangeFgModule);
  rlmachine.attachModule(new DLLModule);
  rlmachine.attachModule(new DebugModule);
  rlmachine.attachModule(new EventLoopModule);
  rlmachine.attachModule(new G00Module);
  rlmachine.attachModule(new GanBgModule);
  rlmachine.attachModule(new GanFgModule);
  rlmachine.attachModule(new GrpModule);
  rlmachine.attachModule(new JmpModule);
  rlmachine.attachModule(new KoeModule);
  rlmachine.attachModule(new LayeredShakingModule);
  rlmachine.attachModule(new MemModule);
  rlmachine.attachModule(new MovModule);
  rlmachine.attachModule(new MsgModule);
  rlmachine.attachModule(new ObjBgCreationModule);
  rlmachine.attachModule(new ObjBgManagement);
  rlmachine.attachModule(new ObjBgModule);
  rlmachine.attachModule(new ObjBgGettersModule);
  rlmachine.attachModule(new ObjCopyFgToBg);
  rlmachine.attachModule(new ObjFgCreationModule);
  rlmachine.attachModule(new ObjFgManagement);
  rlmachine.attachModule(new ObjFgModule);
  rlmachine.attachModule(new ObjFgGettersModule);
  rlmachine.attachModule(new ObjRangeBgModule);
  rlmachine.attachModule(new ObjRangeFgModule);
  rlmachine.attachModule(new OsModule);
  rlmachine.attachModule(new PcmModule);
  rlmachine.attachModule(new RefreshModule);
  rlmachine.attachModule(new ScrModule);
  rlmachine.attachModule(new SeModule);
  rlmachine.attachModule(new SelModule);
  rlmachine.attachModule(new ShakingModule);
  rlmachine.attachModule(new StrModule);
  rlmachine.attachModule(new SysModule);
}
