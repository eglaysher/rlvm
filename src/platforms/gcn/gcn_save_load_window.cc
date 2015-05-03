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

#include "platforms/gcn/gcn_save_load_window.h"

#include <boost/date_time/posix_time/time_formatters_limited.hpp>
#include <boost/filesystem.hpp>

#include <algorithm>
#include <iomanip>
#include <limits>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

#include "machine/rlmachine.h"
#include "machine/serialization.h"
#include "platforms/gcn/gcn_button.h"
#include "platforms/gcn/gcn_platform.h"
#include "platforms/gcn/gcn_scroll_area.h"
#include "utilities/string_utilities.h"

namespace fs = boost::filesystem;

const int PADDING = 5;

const std::string EVENT_SAVE = "SAVE";
const std::string EVENT_LOAD = "LOAD";
const std::string VALUE_CHANGE = "VALUECHANGE";

// -----------------------------------------------------------------------
// SaveGameListModel
// -----------------------------------------------------------------------

// Creates a list of save games from the save game files on disk.
class SaveGameListModel : public gcn::ListModel {
 public:
  SaveGameListModel(const std::string& no_data, RLMachine& machine);
  virtual ~SaveGameListModel();

  // Overridden from gcn::ListModel:
  virtual int getNumberOfElements();
  virtual std::string getElementAt(int i);

  // Whether this save should enable the action button while loading.
  bool getSaveExistsAt(int i);

 private:
  std::vector<std::pair<std::string, bool>> titles_;
};

// -----------------------------------------------------------------------

SaveGameListModel::SaveGameListModel(const std::string& no_data,
                                     RLMachine& machine) {
  int latestSlot = -1;
  time_t latestTime = std::numeric_limits<time_t>::min();

  for (int slot = 0; slot < 100; ++slot) {
    fs::path saveFile = Serialization::buildSaveGameFilename(machine, slot);

    std::ostringstream oss;
    oss << "[" << std::setw(3) << std::setfill('0') << slot << "] ";

    bool file_exists = fs::exists(saveFile);
    if (file_exists) {
      SaveGameHeader header = Serialization::loadHeaderForSlot(machine, slot);
      oss << to_simple_string(header.save_time) << " - "
          << cp932toUTF8(header.title, machine.GetTextEncoding());

      time_t mtime = fs::last_write_time(saveFile);

      if (mtime > latestTime) {
        latestTime = mtime;
        latestSlot = slot;
      }
    } else {
      oss << no_data;
    }

    titles_.emplace_back(oss.str(), file_exists);
  }

  if (latestSlot != -1) {
    titles_[latestSlot].first = "[NEW] " + titles_[latestSlot].first;
  }
}

// -----------------------------------------------------------------------

SaveGameListModel::~SaveGameListModel() {}

// -----------------------------------------------------------------------

int SaveGameListModel::getNumberOfElements() { return titles_.size(); }

// -----------------------------------------------------------------------

std::string SaveGameListModel::getElementAt(int i) {
  if (size_t(i) < titles_.size())
    return titles_[i].first;
  else
    // Control sometimes asks for impossible value.
    return "";
}

// -----------------------------------------------------------------------

bool SaveGameListModel::getSaveExistsAt(int i) {
  if (size_t(i) < titles_.size())
    return titles_[i].second;
  else
    return false;
}

// -----------------------------------------------------------------------
// GCNSaveLoadWindow
// -----------------------------------------------------------------------
GCNSaveLoadWindow::GCNSaveLoadWindow(RLMachine& machine,
                                     WindowType type,
                                     GCNPlatform* platform)
    : GCNWindow(platform),
      model_(new SaveGameListModel("NO DATA", machine)),
      type_(type) {
  setSize(540, 400);

  // Either the save/load button
  action_button_ = new GCNButton();
  if (type == DO_SAVE) {
    action_button_->setCaption(
        platform->GetSyscomString("DLGSAVEMESSAGE_OK_BUTTON_STR"));
    action_button_->setActionEventId(EVENT_SAVE);
  } else {
    action_button_->setCaption(
        platform->GetSyscomString("DLGLOADMESSAGE_OK_BUTTON_STR"));
    action_button_->setActionEventId(EVENT_LOAD);
  }
  action_button_->addActionListener(this);
  action_button_->adjustSize();
  action_button_->setForegroundColor(gcn::Color(100, 100, 100));
  action_button_->setEnabled(false);

  // 030 == CANCEL
  gcn::Button* button = new GCNButton(platform->GetSyscomString("030"));
  button->setActionEventId(EVENT_CANCEL);
  button->addActionListener(this);
  button->setEnabled(true);

  int max_size = std::max(action_button_->getWidth(), button->getWidth());
  action_button_->setWidth(max_size + (2 * PADDING));
  button->setWidth(max_size);

  int button_left = getWidth() - PADDING - action_button_->getWidth();
  int button_top = getHeight() - PADDING - action_button_->getHeight();
  Container::add(action_button_, button_left, button_top);
  Container::add(
      button, button_left - PADDING - button->getWidth(), button_top);
  owned_widgets_.emplace_back(action_button_);
  owned_widgets_.emplace_back(button);

  listbox_ = new gcn::ListBox(model_.get());
  listbox_->setActionEventId(VALUE_CHANGE);
  listbox_->addActionListener(this);
  listbox_->addSelectionListener(this);
  listbox_->adjustSize();
  owned_widgets_.emplace_back(listbox_);

  gcn::ScrollArea* area = new GCNScrollArea(listbox_);
  area->setHorizontalScrollPolicy(gcn::ScrollArea::SHOW_NEVER);
  area->setSize(getWidth() - (2 * PADDING), button_top - (2 * PADDING));

  Container::add(area, PADDING, PADDING);
  owned_widgets_.emplace_back(area);
}

// -----------------------------------------------------------------------

GCNSaveLoadWindow::~GCNSaveLoadWindow() {}

// -----------------------------------------------------------------------

void GCNSaveLoadWindow::action(const gcn::ActionEvent& actionEvent) {
  if (actionEvent.getId() == EVENT_CANCEL) {
    platform_->windowCanceled(this);
  } else if (actionEvent.getId() == EVENT_SAVE) {
    platform_->saveEvent(listbox_->getSelected());
  } else if (actionEvent.getId() == EVENT_LOAD) {
    platform_->loadEvent(listbox_->getSelected());
  }
}

// -----------------------------------------------------------------------

void GCNSaveLoadWindow::valueChanged(const gcn::SelectionEvent& event) {
  // When we get a value from the list box, enable the action button.
  bool activate_button = true;
  if (type_ == DO_LOAD) {
    activate_button = model_->getSaveExistsAt(listbox_->getSelected());
  }

  action_button_->setEnabled(activate_button);

  if (activate_button) {
    action_button_->setForegroundColor(gcn::Color(0, 0, 0));
  } else {
    action_button_->setForegroundColor(gcn::Color(100, 100, 100));
  }
}
