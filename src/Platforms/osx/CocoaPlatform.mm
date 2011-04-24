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

#include "Platforms/osx/CocoaPlatform.h"

// Must be included before AppKit due to macro collision with boost.
#include "MachineBase/RLMachine.hpp"

#include <AppKit/AppKit.h>

#include "Platforms/osx/CocoaRLVMInstance.h"
#include "Platforms/osx/SDLMain.h"
#include "Platforms/osx/mac_utils.h"
#include "Systems/Base/System.hpp"
#include "libReallive/gameexe.h"

// As a private interface detail, CocoaPlatform creates and owns an objective c
// object which exists to redirect calls back to the CocoaPlatform.
@interface MenuRedirector : NSObject {
  CocoaPlatform* platform;
}
@end

@implementation MenuRedirector
-(id)initWithPlatform:(CocoaPlatform*)p
{
  if (self = [super init]) {
    platform = p;
  }
  return self;
}

-(void)activate:(NSMenuItem*)item
{
  platform->MenuItemActivated([item tag]);
}

- (void)menuDidClose:(NSMenu *)menu
{
  platform->MenuDidClose();
}

@end

CocoaPlatform::CocoaPlatform(System& system)
    : Platform(system.gameexe()),
      machine_(NULL),
      menu_(NULL),
      redirector_([[MenuRedirector alloc] initWithPlatform:this]) {
}

CocoaPlatform::~CocoaPlatform() {}

void CocoaPlatform::MenuDidClose() {
  if (machine_) {
    machine_->unpauseExecution();

    // Cocoa is about to break our cursor after this delegate method has been
    // called. Schedule a one off LongOperation to fix that.
    mac_utils::ResetCursor(machine_);
  }
}

void CocoaPlatform::MenuItemActivated(int syscom_id) {
  if (machine_) {
    machine_->system().invokeSyscom(*machine_, syscom_id);
  }
}

void CocoaPlatform::showNativeSyscomMenu(RLMachine& machine) {
  machine_ = &machine;

  [menu_ release];
  menu_ = [[NSMenu alloc] init];
  [menu_ setAutoenablesItems:NO];
  [menu_ setDelegate:redirector_];

  std::vector<MenuSpec> menu;
  GetMenuSpecification(machine, menu);
  RecursivelyBuildMenu(machine, menu, menu_);

  mac_utils::PauseExecution(machine);

  NSView* contentView = [[NSApp mainWindow] contentView];
  NSEvent* e = GetLastRightClickEvent();
  [NSMenu popUpContextMenu:menu_
                 withEvent:e
                   forView:contentView];
}

void CocoaPlatform::invokeSyscomStandardUI(RLMachine& machine, int syscom) {
}

void CocoaPlatform::showSystemInfo(RLMachine& machine, const RlvmInfo& info) {
}

void CocoaPlatform::RecursivelyBuildMenu(RLMachine& machine,
                                         const std::vector<MenuSpec>& menu,
                                         NSMenu* out_menu) {
  System& sys = machine.system();
  for (std::vector<MenuSpec>::const_iterator it = menu.begin();
       it != menu.end(); ++it) {
    NSMenuItem* item = NULL;
    if (it->syscom_id == MENU_SEPARATOR) {
      item = [NSMenuItem separatorItem];
    } else if (it->syscom_id == MENU) {
      NSMenu* submenu = [[NSMenu alloc] init];
      [submenu setAutoenablesItems:NO];
      [submenu setDelegate:redirector_];
      RecursivelyBuildMenu(machine, it->submenu, submenu);

      NSString* label = UTF8ToNSString(GetMenuLabel(*it));
      item = [[NSMenuItem alloc] initWithTitle:label
                                        action:nil
                                 keyEquivalent:@""];
      [item setSubmenu:submenu];
    } else {
      NSString* label = UTF8ToNSString(GetMenuLabel(*it));
      item = [[NSMenuItem alloc] initWithTitle:label
                                        action:@selector(activate:)
                                 keyEquivalent:@""];
      [item setTarget:redirector_];
      [item setTag:it->syscom_id];

      int visibility = sys.isSyscomEnabled(it->syscom_id);
      if (visibility == 0)
        [item setHidden:YES];
      else
        [item setEnabled:(visibility == 1 ? YES : NO)];
    }

    [out_menu addItem:item];
  }
}
