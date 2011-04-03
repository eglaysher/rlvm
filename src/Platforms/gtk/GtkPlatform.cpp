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
#include <gdk/gdkkeysyms.h>
#include <iomanip>
#include <ostream>

#include "MachineBase/RLMachine.hpp"
#include "Systems/Base/EventSystem.hpp"
#include "Systems/Base/System.hpp"

#include <iostream>
using namespace std;

namespace {

void remove_widget(GtkWidget* widget, gpointer container) {
  gtk_container_remove(GTK_CONTAINER(container), widget);
}

void unpause_execution(GtkWidget* widget) {
  gpointer raw_machine = g_object_get_data(G_OBJECT(widget), "rlmachine");
  if (RLMachine* machine = static_cast<RLMachine*>(raw_machine)) {
    machine->unpauseExecution();

    // When a menu item is activated (compare: the menu is dismissed because we
    // clicked outside the menu area), the SDL window is activated immediately
    // and we don't want to do the normal ignoring the next full click
    // behaviour that we usually do.
    machine->system().event().set_ignore_next_mouseup_event(true);
  }
}

void on_menu_item_activated(GtkWidget* widget) {
  int id = GPOINTER_TO_INT(g_object_get_data(G_OBJECT(widget), "id"));
  gpointer raw_machine = g_object_get_data(G_OBJECT(widget), "rlmachine");
  if (RLMachine* machine = static_cast<RLMachine*>(raw_machine)) {
    machine->system().invokeSyscom(*machine, id);
  }
}

// Frustrating hack alert!
//
// Submenus don't always handle the "activate" signal correctly in GTK+. (See
// http://www.kadjo.net/softdev/?p=54 for another instance, along with various
// mailing lists.) The workaround is to have GtkMenuItems connect to the
// "button-press-event" signal. This breaks keyboard selection though, and
// since the user can bring up the right click menu with escape in RealLive
// games, we need ANOTHER hack on menus that recognizes the enter key, tears
// down the menus and forwards the signal to our button press handler.
gint on_menu_key_press(GtkMenuShell* shell,
                       GdkEventKey* event) {
  // TODO: gtk3 defines gtk_menu_shell_get_selected_item(). Too bad we can't
  // use that yet and need to break a GSEAL for now.
  GtkWidget* item = shell->active_menu_item;
  if (event->keyval == GDK_KEY_Return && item != NULL) {
    g_object_ref(item);

    gtk_menu_shell_cancel(shell);

    // AND NOW...We call our "activated" handler.
    on_menu_item_activated(item);

    g_object_unref(item);

    return TRUE;
  }

  return FALSE;
}

}  // namespace

GtkPlatform::GtkPlatform(System& system)
    : Platform(system.gameexe()),
      system_(system),
      menu_(gtk_menu_new()) {
  g_object_ref_sink(menu_);
  g_signal_connect(menu_, "key-press-event",
                   G_CALLBACK(on_menu_key_press), NULL);
  g_signal_connect(menu_, "hide", G_CALLBACK(unpause_execution), NULL);
}

GtkPlatform::~GtkPlatform() {
  gtk_widget_destroy(menu_);
  g_object_unref(menu_);
}

void GtkPlatform::showNativeSyscomMenu(RLMachine& machine) {
  gtk_container_foreach(GTK_CONTAINER(menu_), remove_widget, menu_);

  std::vector<MenuSpec> menu;
  GetMenuSpecification(machine, menu);
  RecursivelyBuildMenu(machine, menu, menu_);

  machine.pauseExecution();
  g_object_set_data(G_OBJECT(menu_), "rlmachine", &machine);
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

void GtkPlatform::RecursivelyBuildMenu(RLMachine& machine,
                                       const std::vector<MenuSpec>& menu,
                                       GtkWidget* out_menu) {
  System& sys = machine.system();
  for (std::vector<MenuSpec>::const_iterator it = menu.begin();
       it != menu.end(); ++it) {
    GtkWidget* item = NULL;
    if (it->syscom_id == MENU_SEPARATOR) {
      item = gtk_separator_menu_item_new();
      gtk_widget_show(item);
    } else if (it->syscom_id == MENU) {
      GtkWidget* submenu = gtk_menu_new();
      RecursivelyBuildMenu(machine, it->submenu, submenu);
      g_signal_connect(submenu, "key-press-event",
                       G_CALLBACK(on_menu_key_press), NULL);

      item = gtk_menu_item_new_with_label(syscomString(it->label).c_str());
      gtk_menu_item_set_submenu(GTK_MENU_ITEM(item), submenu);
      gtk_widget_show(item);
    } else {
      std::string label;
      if (it->label == NULL) {
        std::ostringstream labelss;
        labelss << std::setw(3) << std::setfill('0') << it->syscom_id;
        label = syscomString(labelss.str());
      } else {
        label = syscomString(it->label);
      }

      item = gtk_menu_item_new_with_label(label.c_str());

      int visibility = sys.isSyscomEnabled(it->syscom_id);
      // If 0, we just don't show it.
      if (visibility == 1) {
        gtk_widget_show(item);
      } else if (visibility == 2) {
        gtk_widget_show(item);
        gtk_widget_set_sensitive(item, FALSE);
      }

      g_object_set_data(G_OBJECT(item), "id", GINT_TO_POINTER(it->syscom_id));
      g_object_set_data(G_OBJECT(item), "rlmachine", &machine);

      g_signal_connect(item, "button-press-event",
                       G_CALLBACK(on_menu_item_activated), NULL);
    }

    gtk_menu_shell_append(GTK_MENU_SHELL(out_menu), item);
  }
}
