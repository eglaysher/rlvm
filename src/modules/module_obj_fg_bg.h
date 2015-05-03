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

#ifndef SRC_MODULES_MODULE_OBJ_FG_BG_H_
#define SRC_MODULES_MODULE_OBJ_FG_BG_H_

#include "machine/rlmodule.h"
#include "machine/mapped_rlmodule.h"
#include "modules/object_module.h"

// Contains functions for mod<1:10>, Obj.
class ObjFgModule : public RLModule {
 public:
  ObjFgModule();

 private:
  ObjectModule helper_;
};

class ObjBgModule : public RLModule {
 public:
  ObjBgModule();

 private:
  ObjectModule helper_;
};

class ChildObjFgModule : public MappedRLModule {
 public:
  ChildObjFgModule();

 private:
  ObjectModule helper_;
};

class ChildObjBgModule : public MappedRLModule {
 public:
  ChildObjBgModule();

 private:
  ObjectModule helper_;
};

class ObjRangeFgModule : public MappedRLModule {
 public:
  ObjRangeFgModule();

 private:
  ObjectModule helper_;
};

class ObjRangeBgModule : public MappedRLModule {
 public:
  ObjRangeBgModule();

 private:
  ObjectModule helper_;
};

class ChildObjRangeFgModule : public MappedRLModule {
 public:
  ChildObjRangeFgModule();

 private:
  ObjectModule helper_;
};

class ChildObjRangeBgModule : public MappedRLModule {
 public:
  ChildObjRangeBgModule();

 private:
  ObjectModule helper_;
};

#endif  // SRC_MODULES_MODULE_OBJ_FG_BG_H_
