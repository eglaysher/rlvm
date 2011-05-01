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

#include "Platforms/gtk/GtkPlatform.hpp"

#include <boost/date_time/posix_time/time_formatters_limited.hpp>
#include <boost/filesystem/operations.hpp>
#include <gdk/gdkkeysyms.h>
#include <gtk/gtk.h>
#include <iomanip>
#include <ostream>

#include "MachineBase/RLMachine.hpp"
#include "MachineBase/Serialization.hpp"
#include "Platforms/gtk/gtk_callbacks.hpp"
#include "Platforms/gtk/x11_sdl_util.h"
#include "Systems/Base/EventSystem.hpp"
#include "Systems/Base/System.hpp"
#include "Utilities/StringUtilities.hpp"
#include "libReallive/gameexe.h"

namespace fs = boost::filesystem;

#include <iostream>
using namespace std;

namespace {

// We cheat; we need to always unpause execution before we activate a menu item
// because the activated menu item might spawn a window that pauses execution
// and we don't want to unpause once the new pause has started.
bool g_menu_displayed = false;

// Possible paths to the "rlvm-dialogs.ui" file. We first search locally and
// then assume we're a release build.
const char* ui_paths[] = {
  "./",
  "./build/",
  "/usr/share/rlvm/",
  NULL
};

void remove_widget(GtkWidget* widget, gpointer container) {
  gtk_container_remove(GTK_CONTAINER(container), widget);
}

void unpause_execution(GtkWidget* widget) {
  if (g_menu_displayed) {
    g_menu_displayed = false;
    gpointer raw_machine = g_object_get_data(G_OBJECT(widget), "rlmachine");
    if (RLMachine* machine = static_cast<RLMachine*>(raw_machine)) {
      machine->unpauseExecution();

      // When a menu item is activated (compare: the menu is dismissed because
      // we clicked outside the menu area), the SDL window is activated
      // immediately and we don't want to do the normal ignoring the next full
      // click behaviour that we usually do.
      machine->system().event().set_ignore_next_mouseup_event(true);
    }
  }
}

void on_menu_item_activated(GtkWidget* widget) {
  int id = GPOINTER_TO_INT(g_object_get_data(G_OBJECT(widget), "id"));
  gpointer raw_machine = g_object_get_data(G_OBJECT(widget), "rlmachine");
  if (RLMachine* machine = static_cast<RLMachine*>(raw_machine)) {

    unpause_execution(widget);

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

void BuildSaveStoreBasedOnFilesystem(RLMachine& machine,
                                     GtkListStore* store) {
  // TODO: Can I make this faster instead of trying to see if every game
  // exists?
  int latest_slot = -1;
  time_t latest_time = numeric_limits<time_t>::min();

  std::string no_data =
      cp932toUTF8(
          machine.system().gameexe()("SAVE_NODATA").to_string("No Data"),
          machine.getTextEncoding());

  gtk_list_store_clear(store);
  for (int slot = 0; slot < 100; ++slot) {
    fs::path saveFile = Serialization::buildSaveGameFilename(machine, slot);

    bool file_exists = fs::exists(saveFile);
    std::string title;
    if (file_exists) {
      SaveGameHeader header = Serialization::loadHeaderForSlot(machine, slot);
      ostringstream oss;
      oss << to_simple_string(header.save_time) << " - "
          << cp932toUTF8(header.title, machine.getTextEncoding());
      title = oss.str();

      time_t mtime = fs::last_write_time(saveFile);
      if (mtime > latest_time) {
        latest_time = mtime;
        latest_slot = slot;
      }
    } else {
      title = no_data;
    }

    GtkTreeIter iter;
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store,
                       &iter,
                       0, "",
                       1, slot,
                       2, title.c_str(),
                       3, file_exists,
                       -1);
  }

  if (latest_slot != -1) {
    GtkTreeIter iter;
    bool rv = gtk_tree_model_iter_nth_child(GTK_TREE_MODEL(store),
                                            &iter,
                                            NULL,
                                            latest_slot);
    if (rv) {
      gtk_list_store_set(store, &iter,
                         0, "NEW",
                         -1);
    }
  }
}

}  // namespace

GtkPlatform::GtkPlatform(System& system)
    : Platform(system.gameexe()),
      system_(system),
      menu_(gtk_menu_new()),
      builder_(NULL) {
  g_object_ref_sink(menu_);
  g_signal_connect(menu_, "key-press-event",
                   G_CALLBACK(on_menu_key_press), NULL);
  g_signal_connect(menu_, "hide", G_CALLBACK(unpause_execution), NULL);

  gtk_window_set_default_icon_name("rlvm");
}

GtkPlatform::~GtkPlatform() {
  if (builder_)
    g_object_unref(builder_);

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
  g_menu_displayed = true;
  gtk_menu_popup(GTK_MENU(menu_),
                 NULL,
                 NULL,
                 NULL,
                 NULL,
                 3,
                 gtk_get_current_event_time());
}

void GtkPlatform::raiseSyscomUI(RLMachine& machine) {
  if (!windows_.empty()) {
    gtk_window_present(GTK_WINDOW(windows_.back()));
  }
}

void GtkPlatform::invokeSyscomStandardUI(RLMachine& machine, int syscom) {
  if (!builder_)
    InitializeBuilder(machine);

  GtkWidget* window = NULL;
  if (syscom == SYSCOM_SAVE || syscom == SYSCOM_LOAD) {
    window = SetupSaveLoadWindow(machine, syscom);
  }
  // TODO(erg): Every other syscom UI window.

  if (window) {
    // Hook up the transient removal methods only once.
    if (g_signal_handler_find(
            window, G_SIGNAL_MATCH_FUNC, 0, 0, NULL,
            reinterpret_cast<gpointer>(RemoveDialogFromTransitentList),
            NULL) == 0) {
      g_signal_connect(window, "hide",
                       G_CALLBACK(RemoveDialogFromTransitentList), this);
    }

    CenterGtkWindowOverSDLWindow(window);

    windows_.push_back(window);
    gtk_widget_show_all(window);
  }
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
      item = gtk_menu_item_new_with_label(GetMenuLabel(*it).c_str());

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

void GtkPlatform::InitializeBuilder(RLMachine& machine) {
  builder_ = gtk_builder_new();

  for (const char** cur_path = ui_paths; *cur_path; cur_path++) {
    fs::path path = fs::path(*cur_path) / "rlvm-dialogs.ui";
    if (fs::exists(path)) {
      GError* error = NULL;
      if (gtk_builder_add_from_file(builder_,
                                    path.string().c_str(), &error)) {
        break;
      } else {
        cerr << "Error: " << error->message << endl;
        g_object_unref(builder_);
        g_error_free(error);
        return;
      }
    }
  }

  gtk_builder_connect_signals_full(builder_, GtkCallbackBuilder, &machine);

  // Glade/GtkBuilder does not allow us to set signals on internal objects. We
  // want to disable the open button when an invalid row is selected and that
  // means opening up GtkTreeView and connecting to a signal on its
  // GtkTreeSelection.
  GtkTreeSelection* selection = gtk_tree_view_get_selection(
      GTK_TREE_VIEW(gtk_builder_get_object(builder_, "saveload_treeview")));
  g_signal_connect(selection, "changed",
                   G_CALLBACK(on_treeview_row_changed), builder_);

  // While we're at it, mark the tree selection on the save/load dialog.
  g_object_set_data(gtk_builder_get_object(builder_, "saveload_dialog"),
                    "tree-selection", selection);
}

GtkWidget* GtkPlatform::SetupSaveLoadWindow(RLMachine& machine, int syscom) {
  // Save/Load share a single dialog; we just act slightly differently
  // depending on whether we're saving or loading.
  GtkWidget* window = GTK_WIDGET(gtk_builder_get_object(builder_,
                                                        "saveload_dialog"));

  GtkListStore* store = GTK_LIST_STORE(gtk_builder_get_object(
      builder_, "saveload_store"));
  BuildSaveStoreBasedOnFilesystem(machine, store);

  // Set the window title depending on which dialog we are.
  std::string title;
  if (syscom == SYSCOM_SAVE) {
    title = machine.system().gameexe()("DLGSAVEMESSAGE_TITLE_STR").
            to_string("SAVE");
  } else {
    title = machine.system().gameexe()("DLGLOADMESSAGE_TITLE_STR").
            to_string("LOAD");
  }
  gtk_window_set_title(GTK_WINDOW(window),
                       cp932toUTF8(title, machine.getTextEncoding()).c_str());

  // Only show the correct action for this dialog.
  gtk_widget_set_visible(
      GTK_WIDGET(gtk_builder_get_object(builder_, "save_button")),
      syscom == SYSCOM_SAVE);
  gtk_widget_set_visible(
      GTK_WIDGET(gtk_builder_get_object(builder_, "load_button")),
      syscom == SYSCOM_LOAD);

  return window;
}

void GtkPlatform::CenterGtkWindowOverSDLWindow(GtkWidget* window) {
  // It'd be so much easier to just use gtk_window_set_type_hint() to center
  // the window, but that requires gtk collaboration between the two windows.
  Rect r = GetSDLWindowPosition();

  if (!r.isEmpty()) {
    // Get the size of the gtk window.
    gint width, height;
    gtk_window_get_size(GTK_WINDOW(window), &width, &height);
    Size s(width, height);

    Rect centered = s.centeredIn(r);
    gtk_window_move(GTK_WINDOW(window), centered.x(), centered.y());
  }
}

// static
void GtkPlatform::RemoveDialogFromTransitentList(GtkWidget* widget,
                                                 GtkPlatform* platform) {
  std::vector<GtkWidget*>::iterator it = find(platform->windows_.begin(),
                                              platform->windows_.end(),
                                              widget);
  if (it != platform->windows_.end())
    platform->windows_.erase(it);
}
