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

#ifndef SRC_PLATFORMS_GTK_GTKPLATFORM_HPP_
#define SRC_PLATFORMS_GTK_GTKPLATFORM_HPP_

#include "Systems/Base/Platform.hpp"

typedef struct _GtkBuilder GtkBuilder;
typedef struct _GtkWidget GtkWidget;

class Rect;
class RLMachine;
class RlvmInfo;
class System;

// Displays GTK+ versions of game controlled dialogs.
class GtkPlatform : public Platform {
 public:
  GtkPlatform(System& system);
  virtual ~GtkPlatform();

  // Platform:
  virtual void showNativeSyscomMenu(RLMachine& machine);
  virtual void invokeSyscomStandardUI(RLMachine& machine, int syscom);
  virtual void raiseSyscomUI(RLMachine& machine);
  virtual void showSystemInfo(RLMachine& machine, const RlvmInfo& info);

 private:
  // Builds the GtkMenu representation of the cross platform MenuSpec
  // representation.
  void RecursivelyBuildMenu(RLMachine& machine,
                            const std::vector<MenuSpec>& menu,
                            GtkWidget* out_menu);

  // Initializes |builder_|.
  void InitializeBuilder(RLMachine& machine);

  // Fetches and initializes the state of the save/load window based on
  // |syscom|.
  GtkWidget* SetupSaveLoadWindow(RLMachine& machine, int syscom);

  // Centers |window| over the main SDL window.
  void CenterGtkWindowOverSDLWindow(GtkWidget* window);

  // Returns a rect with the SDL window location. Can return an empty rect on
  // error.
  Rect GetSDLWindowPosition();

  // While most "hidden" signals are connected in the ui file and specified in
  // the callbacks file, every dialog is connected through code to this
  // platform object to manage the transient state.
  static void RemoveDialogFromTransitentList(GtkWidget* widget,
                                             GtkPlatform* platform);

  System& system_;

  // Our toplevel menu. We clear this at the beginning of every
  // showNativeSyscomMenu() call, but we keep the same object around since
  // gtk_menu_popup() doesn't take ownership.
  GtkWidget* menu_;

  // Builds our dialogs from an XML file.
  GtkBuilder* builder_;

  // Stack of currently invoked windows. Implementation detail of making gtk
  // windows "transient-for" the SDL window.
  std::vector<GtkWidget*> windows_;
};

#endif  // SRC_PLATFORMS_GTK_GTKPLATFORM_HPP_
