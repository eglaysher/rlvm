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

#include "platforms/sdl/sdl_rlvm_instance.h"

#include <SDL.h>
#include <libintl.h>
#include <string>

#include "utilities/file.h"
#include "utilities/gettext.h"

namespace fs = boost::filesystem;

SDLRLVMInstance::SDLRLVMInstance(int* argc, char** argv[]) : RLVMInstance() {
}

SDLRLVMInstance::~SDLRLVMInstance() {}

boost::filesystem::path SDLRLVMInstance::SelectGameDirectory() {
  /*
  SDLWidget* dialog = SDL_file_chooser_dialog_new(_("Select Game Directory"),
                                                  NULL,
                                                  SDL_FILE_CHOOSER_ACTION_OPEN,
                                                  SDL_STOCK_CANCEL,
                                                  SDL_RESPONSE_CANCEL,
                                                  SDL_STOCK_OPEN,
                                                  SDL_RESPONSE_ACCEPT,
                                                  NULL);

  SDL_file_chooser_set_action(SDL_FILE_CHOOSER(dialog),
                              SDL_FILE_CHOOSER_ACTION_SELECT_FOLDER);
  SDL_file_chooser_set_create_folders(SDL_FILE_CHOOSER(dialog), false);
  g_signal_connect(
      dialog, "selection-changed", G_CALLBACK(on_selection_changed), NULL);

  boost::filesystem::path out_path;
  if (SDL_dialog_run(SDL_DIALOG(dialog)) == SDL_RESPONSE_ACCEPT) {
    char* filename = SDL_file_chooser_get_filename(SDL_FILE_CHOOSER(dialog));
    out_path = filename;
    g_free(filename);
  }
  SDL_widget_hide(dialog);
  SDL_widget_destroy(dialog);

  // Actually execute the destruction signals.
  DoNativeWork();

  return out_path;
  */

  // TODO:
  return fs::path("gamedir/");
}

void SDLRLVMInstance::ReportFatalError(const std::string& message_text,
                                       const std::string& informative_text) {
  RLVMInstance::ReportFatalError(message_text, informative_text);

  if(SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, message_text.c_str(),
    informative_text.c_str(), NULL)) {
     printf("%s: SDL_ShowSimpleMessageBox failed: %s\n"
       "Message Text: %s\n"
       "Informative Text: %s\n", __FUNCTION__, SDL_GetError(), message_text.c_str(), informative_text.c_str());
  }
}

bool SDLRLVMInstance::AskUserPrompt(const std::string& message_text,
                                    const std::string& informative_text,
                                    const std::string& true_button,
                                    const std::string& false_button) {
  SDL_MessageBoxButtonData yesBtn = { 0, 1, "Yes" };
  SDL_MessageBoxButtonData noBtn  = { 0, 0, "No" };
  const SDL_MessageBoxButtonData btns[2] = { noBtn, yesBtn };
  SDL_MessageBoxData msgBox = {
    SDL_MESSAGEBOX_INFORMATION,
    NULL,
    message_text.c_str(),
    informative_text.c_str(),
    sizeof(btns) / sizeof(btns[0]),
    btns,
    NULL
  };

  int btnId = 0;
  if(SDL_ShowMessageBox(&msgBox, &btnId)) {
     ReportFatalError(__FUNCTION__, std::string("Can't show prompt") + SDL_GetError());
  }
  return btnId != 0;
}
