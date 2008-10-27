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

#include "Precompiled.hpp"

// -----------------------------------------------------------------------

#include "Platforms/gcn/GCNSaveLoadWindow.hpp"

#include "Platforms/gcn/GCNPlatform.hpp"
#include "MachineBase/Serialization.hpp"
#include "MachineBase/RLMachine.hpp"
#include "Modules/cp932toUnicode.hpp"

#include <iomanip>
#include <sstream>
#include <boost/filesystem.hpp>
#include <boost/date_time/posix_time/time_formatters_limited.hpp>
namespace fs = boost::filesystem;

using namespace std;

const int PADDING = 5;

const std::string EVENT_SAVE = "SAVE";
const std::string EVENT_LOAD = "LOAD";
const std::string VALUE_CHANGE = "VALUECHANGE";

// -----------------------------------------------------------------------
// SaveGameListModel
// -----------------------------------------------------------------------
/**
 * Creates a list of save games from the save game files on disk.
 */
class SaveGameListModel : public gcn::ListModel {
public:
  SaveGameListModel(const std::string& no_data, RLMachine& machine);
  virtual ~SaveGameListModel();

  // Overridden from gcn::ListModel:
  virtual int getNumberOfElements();
  virtual std::string getElementAt(int i);

private:
  std::vector<std::string> titles_;
};

// -----------------------------------------------------------------------

SaveGameListModel::SaveGameListModel(const std::string& no_data,
                                     RLMachine& machine)
{
  using namespace boost::posix_time;

  for (int slot = 0; slot < 100; ++slot) {
    fs::path saveFile = Serialization::buildSaveGameFilename(machine, slot);

    ostringstream oss;
    oss << "[" << setw(3) << setfill('0') << slot << "] ";

    if (fs::exists(saveFile)) {
      SaveGameHeader header = Serialization::loadHeaderForSlot(machine, slot);
      oss << to_simple_string(header.save_time) << " - "
          << cp932toUTF8(header.title, machine.getTextEncoding());
    } else {
      oss << no_data;
    }

    titles_.push_back(oss.str());
  }

  // To fix a bug in GUIChan. For some reason, it crashes if I don't have a
  // buffer extra item (which I exclude with getNumberOfElements()).
  titles_.push_back("never displayed");
}

// -----------------------------------------------------------------------

SaveGameListModel::~SaveGameListModel()
{
}

// -----------------------------------------------------------------------

int SaveGameListModel::getNumberOfElements()
{
  // I suspect a bug in gcn::ListBox. :(
  return titles_.size() - 1;
}

// -----------------------------------------------------------------------

std::string SaveGameListModel::getElementAt(int i)
{
  return titles_.at(i);
}

// -----------------------------------------------------------------------
// GCNSaveLoadWindow
// -----------------------------------------------------------------------
GCNSaveLoadWindow::GCNSaveLoadWindow(RLMachine& machine, WindowType type,
                                     GCNPlatform* platform)
  : GCNWindow(platform), model_(new SaveGameListModel("NO DATA", machine))
{
  setSize(540, 400);

  // Either the save/load button
  action_button_ = new gcn::Button();
  if (type == DO_SAVE) {
    action_button_->setCaption(
      platform->syscomString("DLGSAVEMESSAGE_OK_BUTTON_STR"));
    action_button_->setActionEventId(EVENT_SAVE);
  } else {
    action_button_->setCaption(
      platform->syscomString("DLGLOADMESSAGE_OK_BUTTON_STR"));
    action_button_->setActionEventId(EVENT_LOAD);
  }
  action_button_->addActionListener(this);
  action_button_->adjustSize();
  action_button_->setForegroundColor(gcn::Color(100, 100, 100));
  action_button_->setEnabled(false);

  int button_left = getWidth() - PADDING - action_button_->getWidth();
  int button_top = getHeight() - PADDING - action_button_->getHeight();
  Container::add(action_button_, button_left, button_top);

  // 030 == CANCEL
  gcn::Button* button = new gcn::Button(platform->syscomString("030"));
  button->setActionEventId(EVENT_CANCEL);
  button->addActionListener(this);
  button->setEnabled(true);
  Container::add(button, button_left - PADDING - button->getWidth(),
                 button_top);

  listbox_ = new gcn::ListBox(model_.get());
  listbox_->setActionEventId(VALUE_CHANGE);
  listbox_->addActionListener(this);
  listbox_->addSelectionListener(this);
  listbox_->adjustSize();
  listbox_->setWidth(500);

  gcn::ScrollArea* area = new gcn::ScrollArea(
    listbox_, gcn::ScrollArea::SHOW_NEVER, gcn::ScrollArea::SHOW_ALWAYS);
  area->setSize(getWidth() - (2*PADDING), button_top - (2*PADDING));

  Container::add(area, PADDING, PADDING);
}

// -----------------------------------------------------------------------

GCNSaveLoadWindow::~GCNSaveLoadWindow()
{
}

// -----------------------------------------------------------------------

void GCNSaveLoadWindow::action(const gcn::ActionEvent& actionEvent)
{
  if (actionEvent.getId() == EVENT_CANCEL) {
    platform_->windowCanceled(this);
  } else if (actionEvent.getId() == EVENT_SAVE) {
    // TODO: Use selection listener?
    platform_->saveEvent(listbox_->getSelected());
  } else if (actionEvent.getId() == EVENT_LOAD) {
    platform_->loadEvent(listbox_->getSelected());
  }
}

// -----------------------------------------------------------------------

void GCNSaveLoadWindow::valueChanged(const gcn::SelectionEvent& event)
{
  // When we get a value from the list box, enable the action button.
  action_button_->setEnabled(true);
  action_button_->setForegroundColor(gcn::Color(0, 0, 0));
}
