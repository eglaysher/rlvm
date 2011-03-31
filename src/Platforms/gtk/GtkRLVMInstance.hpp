// -*- Mode: C++; tab-width:2; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi:tw=80:et:ts=2:sts=2
//
// -----------------------------------------------------------------------
//
// This file is part of RLVM, a RealLive virtual machine clone.
//
// -----------------------------------------------------------------------
//
// Copyright (C) 2011 Elliot Glaysher
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

#ifndef SRC_PLATFORMS_GTK_GTKRLVMINSTANCE_HPP_
#define SRC_PLATFORMS_GTK_GTKRLVMINSTANCE_HPP_

#include "MachineBase/RLVMInstance.hpp"

// A GTK subclass of RLVMInstance that displays GTK dialogs.
class GtkRLVMInstance : public RLVMInstance {
 public:
  GtkRLVMInstance(int* argc, char** argv[]);
  virtual ~GtkRLVMInstance();

  virtual boost::filesystem::path SelectGameDirectory();

 protected:
  virtual void ReportFatalError(const std::string& message_text,
                                const std::string& informative_text);
  virtual void DoNativeWork();
  virtual Platform* BuildNativePlatform(System& system);
};

#endif  // SRC_PLATFORMS_GTK_GTKRLVMINSTANCE_HPP_
