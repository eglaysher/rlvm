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

#include "platforms/gtk/gtk_rlvm_instance.h"

#include <gtk/gtk.h>
#include <libintl.h>
#include <string>

#include "utilities/file.h"
#include "utilities/gettext.h"

namespace fs = boost::filesystem;

namespace {

void on_selection_changed(GtkFileChooser* chooser) {
  gchar* name = gtk_file_chooser_get_filename(chooser);
  if (name) {
    boost::filesystem::path path =
        CorrectPathCase(fs::path(name) / fs::path("Gameexe.ini"));
    gtk_dialog_set_response_sensitive(
        GTK_DIALOG(chooser), GTK_RESPONSE_ACCEPT, fs::exists(path));
    g_free(name);
  }
}

}  // namespace

GtkRLVMInstance::GtkRLVMInstance(int* argc, char** argv[]) : RLVMInstance() {
#if defined ENABLE_NLS
  gtk_set_locale();
#endif
  gtk_init(argc, argv);

#if defined ENABLE_NLS
  setlocale(LC_ALL, "");
  bindtextdomain("rlvm", "/usr/share/locale");
  textdomain("rlvm");
#endif
}

GtkRLVMInstance::~GtkRLVMInstance() {}

boost::filesystem::path GtkRLVMInstance::SelectGameDirectory() {
  GtkWidget* dialog = gtk_file_chooser_dialog_new(_("Select Game Directory"),
                                                  NULL,
                                                  GTK_FILE_CHOOSER_ACTION_OPEN,
                                                  GTK_STOCK_CANCEL,
                                                  GTK_RESPONSE_CANCEL,
                                                  GTK_STOCK_OPEN,
                                                  GTK_RESPONSE_ACCEPT,
                                                  NULL);

  gtk_file_chooser_set_action(GTK_FILE_CHOOSER(dialog),
                              GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER);
  gtk_file_chooser_set_create_folders(GTK_FILE_CHOOSER(dialog), false);
  g_signal_connect(
      dialog, "selection-changed", G_CALLBACK(on_selection_changed), NULL);

  boost::filesystem::path out_path;
  if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
    char* filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
    out_path = filename;
    g_free(filename);
  }
  gtk_widget_hide(dialog);
  gtk_widget_destroy(dialog);

  // Actually execute the destruction signals.
  DoNativeWork();

  return out_path;
}

void GtkRLVMInstance::DoNativeWork() {
  while (gtk_events_pending())
    gtk_main_iteration();
}

void GtkRLVMInstance::ReportFatalError(const std::string& message_text,
                                       const std::string& informative_text) {
  RLVMInstance::ReportFatalError(message_text, informative_text);

  GtkWidget* message = gtk_message_dialog_new(NULL,
                                              GTK_DIALOG_MODAL,
                                              GTK_MESSAGE_ERROR,
                                              GTK_BUTTONS_CLOSE,
                                              "%s",
                                              message_text.c_str());
  gtk_message_dialog_format_secondary_text(
      GTK_MESSAGE_DIALOG(message), "%s", informative_text.c_str());

  gtk_dialog_run(GTK_DIALOG(message));
  gtk_widget_destroy(message);

  DoNativeWork();
}

bool GtkRLVMInstance::AskUserPrompt(const std::string& message_text,
                                    const std::string& informative_text,
                                    const std::string& true_button,
                                    const std::string& false_button) {
  GtkWidget* message = gtk_message_dialog_new(NULL,
                                              GTK_DIALOG_MODAL,
                                              GTK_MESSAGE_WARNING,
                                              GTK_BUTTONS_NONE,
                                              "%s",
                                              message_text.c_str());
  gtk_message_dialog_format_secondary_text(
      GTK_MESSAGE_DIALOG(message), "%s", informative_text.c_str());

  gtk_dialog_add_button(GTK_DIALOG(message), false_button.c_str(), 0);
  gtk_dialog_add_button(GTK_DIALOG(message), true_button.c_str(), 1);
  gtk_dialog_set_default_response(GTK_DIALOG(message), 1);

  gint result = gtk_dialog_run(GTK_DIALOG(message));
  gtk_widget_destroy(message);

  DoNativeWork();

  return result;
}
