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

// Implementations of all the callback methods used in the .ui file.

#include "Platforms/gtk/gtk_callbacks.hpp"

#include <cstring>

#include "MachineBase/RLMachine.hpp"
#include "MachineBase/Serialization.hpp"
#include "Modules/Module_Sys_Save.hpp"

#include <iostream>
using namespace std;

namespace {

// Every dialog will call this method to pause the machine.
void on_dialog_show(GtkWidget* widget, RLMachine* machine) {
  machine->pauseExecution();
}

// Callback for when the save dialog is dismissed.
void on_save_dialog_response(GtkWidget* widget,
                             gint response_id,
                             RLMachine* machine) {
  // Release the lock.
  machine->unpauseExecution();

  // Hide the window.
  gtk_widget_hide(widget);

  // Grab the tree selection (set in GtkPlatform::InitializeBuilder()).
  GtkTreeSelection* selection = GTK_TREE_SELECTION(g_object_get_data(
      G_OBJECT(widget), "tree-selection"));
  GtkTreeIter iter;
  GtkTreeModel* model;
  if (selection && gtk_tree_selection_get_selected(selection, &model, &iter)) {
    GValue value = { 0, };
    gtk_tree_model_get_value(model, &iter, 1, &value);
    int slot = g_value_get_int(&value);
    g_value_unset(&value);

    if (response_id == 1) {
      // The save button was clicked.
      Serialization::saveGlobalMemory(*machine);
      Serialization::saveGameForSlot(*machine, slot);
    } else if (response_id == 2) {
      // The load button was clicked.
      machine->clearLongOperationsOffBackOfStack();
      Sys_load()(*machine, slot);
    }
  }
}

#define ADD_CALLBACK(x) { #x , G_CALLBACK(x) }

struct CallbackDecl {
  const char* name;
  GCallback callback;
} callback_functions[] = {
  ADD_CALLBACK(gtk_widget_hide_on_delete),
  ADD_CALLBACK(on_dialog_show),
  ADD_CALLBACK(on_save_dialog_response),
  {NULL, NULL}
};

#undef ADD_CALLBACK

}  // namespace

void on_treeview_row_changed(GtkTreeSelection* selection, GtkBuilder* builder) {
  GtkWidget* load_button =
      GTK_WIDGET(gtk_builder_get_object(builder, "load_button"));
  if (gtk_widget_get_visible(load_button)) {
    GtkTreeIter iter;
    GtkTreeModel* model;
    if (gtk_tree_selection_get_selected(selection, &model, &iter)) {
      GValue value = { 0, };
      gtk_tree_model_get_value(model, &iter, 3, &value);
      bool ok = g_value_get_boolean(&value);
      g_value_unset(&value);

      gtk_widget_set_sensitive(load_button, ok);
    }
  }
}

// The main publicly exposed callback.
void GtkCallbackBuilder(GtkBuilder* builder,
                        GObject* object,
                        const gchar* signal_name,
                        const gchar* handler_name,
                        GObject* connect_object,
                        GConnectFlags flags,
                        gpointer user_data) {
  GCallback func = NULL;
  CallbackDecl* decl = callback_functions;
  while (decl->name) {
    if (std::strcmp(decl->name, handler_name) == 0) {
      func = decl->callback;
      break;
    }

    decl++;
  }

  if (!func) {
    cerr << "Could not find function for " << handler_name << endl;
    return;
  }

  if (connect_object)
    g_signal_connect_object(object, signal_name, func, connect_object, flags);
  else
    g_signal_connect_data(object, signal_name, func, user_data, NULL, flags);
}
