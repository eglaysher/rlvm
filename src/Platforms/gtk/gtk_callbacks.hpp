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

#ifndef SRC_PLATFORMS_GTK_GTK_CALLBACKS_HPP_
#define SRC_PLATFORMS_GTK_GTK_CALLBACKS_HPP_

#include <gtk/gtk.h>

// Disable the open button when we're pointing to a row that contains an
// unloaded game.
//
// (This is a signal on an object internal to GtkTreeView, and thus can't be
// connected to through the ui file.)
void on_treeview_row_changed(GtkTreeSelection* selection, GtkBuilder* builder);

// Connects all signals declared in "rlvm-dialogs.ui" without using GModule.
void GtkCallbackBuilder(GtkBuilder* builder,
                        GObject* object,
                        const gchar* signal_name,
                        const gchar* handler_name,
                        GObject* connect_object,
                        GConnectFlags flags,
                        gpointer user_data);

#endif  // SRC_PLATFORMS_GTK_GTK_CALLBACKS_HPP_
