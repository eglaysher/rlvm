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

#include "Platforms/osx/CocoaRLVMInstance.h"

#include <AppKit/AppKit.h>
#include <Cocoa/Cocoa.h>

#include "Platforms/osx/CocoaPlatform.h"

CocoaRLVMInstance::CocoaRLVMInstance() : RLVMInstance() {}

CocoaRLVMInstance::~CocoaRLVMInstance() {}

boost::filesystem::path CocoaRLVMInstance::SelectGameDirectory() {
  NSOpenPanel *oPanel = [NSOpenPanel openPanel];
  [oPanel setAllowsMultipleSelection:NO];
  [oPanel setCanChooseFiles:NO];
  [oPanel setCanChooseDirectories:YES];
  [oPanel setTitle:@"Select game folder..."];
  [oPanel setPrompt:@"Play"];

  int status = [oPanel runModal];
  boost::filesystem::path out_path;
  if (status == NSOKButton) {
    NSArray* filenames = [oPanel filenames];
    NSString* filename = [filenames objectAtIndex:0];
    out_path = boost::filesystem::path([filename UTF8String]);
  }

  return out_path;
}

void CocoaRLVMInstance::ReportFatalError(const std::string& message_text,
                                         const std::string& informative_text) {
  RLVMInstance::ReportFatalError(message_text, informative_text);

  NSString* message = UTF8ToNSString(message_text);
  NSString* information = UTF8ToNSString(informative_text);

  NSAlert* alert = [NSAlert alertWithMessageText:message
                                   defaultButton:nil
                                 alternateButton:nil
                                     otherButton:nil
                       informativeTextWithFormat:information];
  [alert runModal];
}

void CocoaRLVMInstance::DoNativeWork() {}

Platform* CocoaRLVMInstance::BuildNativePlatform(System& system) {
  return new CocoaPlatform(system);
}

NSString* UTF8ToNSString(const std::string& in) {
  return [[NSString alloc] initWithUTF8String:in.c_str()];
}
