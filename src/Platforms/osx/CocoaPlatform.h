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

#ifndef SRC_PLATFORMS_OSX_COCOAPLATFORM_H_
#define SRC_PLATFORMS_OSX_COCOAPLATFORM_H_

#include "Systems/Base/Platform.hpp"

class RLMachine;
class System;
@class MenuRedirector;
@class NSMenu;

class CocoaPlatform : public Platform {
 public:
  CocoaPlatform(System& system);
  virtual ~CocoaPlatform();

  // Called by our private delegate when various menu events occur.
  void MenuDidClose();
  void MenuItemActivated(int syscom_id);

  // Platform:
  virtual void showNativeSyscomMenu(RLMachine& machine);
  virtual void invokeSyscomStandardUI(RLMachine& machine, int syscom);
  virtual void showSystemInfo(RLMachine& machine, const RlvmInfo& info);

 private:
  // Recursively build the NSMenu from the cross platform representation.
  void RecursivelyBuildMenu(RLMachine& machine,
                            const std::vector<MenuSpec>& menu,
                            NSMenu* out_menu);

  // The last RLMachine* we saw. Weak ref.
  RLMachine* machine_;

  // The last menu constructed. Regenerated each time we show the menu.
  NSMenu* menu_;

  // An Objective-C object that forwards method calls back to CocoaPlatform.
  MenuRedirector* redirector_;
};

#endif  // SRC_PLATFORMS_OSX_COCOAPLATFORM_HPP_
