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

#ifndef SRC_PLATFORMS_SDL_SDL_RLVM_INSTANCE_H_
#define SRC_PLATFORMS_SDL_SDL_RLVM_INSTANCE_H_

#include <string>

#include "machine/rlvm_instance.h"

// A SDL subclass of RLVMInstance that displays SDL dialogs.
class SDLRLVMInstance : public RLVMInstance {
 public:
  SDLRLVMInstance(int* argc, char** argv[]);
  virtual ~SDLRLVMInstance();

  virtual boost::filesystem::path SelectGameDirectory();

 protected:
  virtual void ReportFatalError(const std::string& message_text,
                                const std::string& informative_text);

  virtual bool AskUserPrompt(const std::string& message_text,
                             const std::string& informative_text,
                             const std::string& true_button,
                             const std::string& false_button);
};

#endif  // SRC_PLATFORMS_SDL_SDL_RLVM_INSTANCE_H_
