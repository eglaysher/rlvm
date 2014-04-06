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
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
// -----------------------------------------------------------------------

#include "platforms/osx/CocoaRLVMInstance.h"

#include <AppKit/AppKit.h>
#include <Cocoa/Cocoa.h>
#include <boost/filesystem.hpp>

#include "utilities/file.h"
#include "utilities/gettext.h"

namespace fs = boost::filesystem;

@interface FileValidator : NSObject
- (BOOL)panel:(id)sender isValidFilename:(NSString *)filename;
@end

@implementation FileValidator
- (BOOL)panel:(id)sender isValidFilename:(NSString *)filename {
  if (!fs::exists(CorrectPathCase(
          fs::path([filename fileSystemRepresentation]) / "Seen.txt"))) {
    NSAlert *alert = [[NSAlert alloc] init];
    [alert setMessageText:[NSString stringWithUTF8String:
      _("This directory is not a RealLive game. Games have a Seen.txt file.")]];
    [alert beginSheetModalForWindow:sender
                      modalDelegate:nil
                     didEndSelector:NULL
                        contextInfo:NULL];
    return NO;
  }

  return YES;
}
@end

CocoaRLVMInstance::CocoaRLVMInstance()
    : RLVMInstance(),
      validator_([[FileValidator alloc] init]) {
}

CocoaRLVMInstance::~CocoaRLVMInstance() {
  [validator_ release];
}

boost::filesystem::path CocoaRLVMInstance::SelectGameDirectory() {
  NSOpenPanel *oPanel = [NSOpenPanel openPanel];
  [oPanel setAllowsMultipleSelection:NO];
  [oPanel setCanChooseFiles:NO];
  [oPanel setCanChooseDirectories:YES];
  [oPanel setTitle:[NSString stringWithUTF8String:_("Select Game Directory")]];;
  [oPanel setDelegate:validator_];

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

bool CocoaRLVMInstance::AskUserPrompt(const std::string& message_text,
                                      const std::string& informative_text,
                                      const std::string& true_button,
                                      const std::string& false_button) {
  NSString* message = UTF8ToNSString(message_text);
  NSString* information = UTF8ToNSString(informative_text);

  NSString* d = UTF8ToNSString(true_button);
  NSString* other = UTF8ToNSString(false_button);

  NSAlert* alert = [NSAlert alertWithMessageText:message
                                   defaultButton:d
                                 alternateButton:nil
                                     otherButton:other
                       informativeTextWithFormat:information];
  NSInteger ret = [alert runModal];
  return ret == NSAlertDefaultReturn;
}

NSString* UTF8ToNSString(const std::string& in) {
  return [[NSString alloc] initWithUTF8String:in.c_str()];
}
