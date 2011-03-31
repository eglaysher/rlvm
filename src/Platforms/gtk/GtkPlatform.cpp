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

#include "Platforms/gtk/GtkPlatform.hpp"

#include <gtk/gtk.h>

#include "Systems/Base/System.hpp"

namespace {

void remove_widget(GtkWidget* widget, gpointer container) {
  gtk_container_remove(GTK_CONTAINER(container), widget);
}

}

GtkPlatform::GtkPlatform(System& system)
    : Platform(system.gameexe()),
      system_(system),
      menu_(gtk_menu_new()) {
  g_object_ref_sink(menu_);
}

GtkPlatform::~GtkPlatform() {
  gtk_widget_destroy(menu_);
  g_object_unref(menu_);
}

void GtkPlatform::showNativeSyscomMenu(RLMachine& machine) {
  gtk_container_foreach(GTK_CONTAINER(menu_), remove_widget, menu_);

  GtkWidget* item = gtk_menu_item_new_with_label("one");
  gtk_widget_show(item);
  gtk_menu_shell_append(GTK_MENU_SHELL(menu_), item);

  gtk_menu_popup(GTK_MENU(menu_),
                 NULL,
                 NULL,
                 NULL,
                 NULL,
                 3,
                 gtk_get_current_event_time());
}

void GtkPlatform::invokeSyscomStandardUI(RLMachine& machine, int syscom) {
}

void GtkPlatform::showSystemInfo(RLMachine& machine, const RlvmInfo& info) {
}
