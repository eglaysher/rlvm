// -*- Mode: C++; tab-width:2; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi:tw=80:et:ts=2:sts=2
//
// -----------------------------------------------------------------------
//
// This file is part of RLVM, a RealLive virtual machine clone.
//
// -----------------------------------------------------------------------
//
// Copyright (C) 2008 Elliot Glaysher
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

#ifndef SRC_PLATFORMS_GCN_GCN_SAVE_LOAD_WINDOW_H_
#define SRC_PLATFORMS_GCN_GCN_SAVE_LOAD_WINDOW_H_

#include <guichan/actionlistener.hpp>
#include <guichan/widgets/listbox.hpp>
#include <guichan/selectionlistener.hpp>
#include <guichan/listmodel.hpp>
#include <guichan/widgets/button.hpp>

#include <vector>

#include "platforms/gcn/gcn_window.h"

class RLMachine;
class SaveGameListModel;

// -----------------------------------------------------------------------

class GCNSaveLoadWindow : public GCNWindow,
                          public gcn::ActionListener,
                          public gcn::SelectionListener {
 public:
  enum WindowType { DO_SAVE, DO_LOAD };

 public:
  GCNSaveLoadWindow(RLMachine& machine,
                    WindowType type,
                    GCNPlatform* platform_);
  ~GCNSaveLoadWindow();

  // Overriden from gcn::ActionListener:
  virtual void action(const gcn::ActionEvent& actionEvent);

  // Overriden from gcn::SelectionListener:
  virtual void valueChanged(const gcn::SelectionEvent& event);

 private:
  // Provides titles and whether a save exists in said slot.
  std::unique_ptr<SaveGameListModel> model_;

  // The kind of dialog we're presenting.
  WindowType type_;

  // Either "Save" or "Load"
  gcn::Button* action_button_;

  gcn::ListBox* listbox_;

  std::vector<std::unique_ptr<gcn::Widget>> owned_widgets_;
};  // end of class GCNSaveLoadWindow

#endif  // SRC_PLATFORMS_GCN_GCN_SAVE_LOAD_WINDOW_H_
