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

#include "platforms/gcn/gcn_menu.h"

#include <guichan/widgets/button.hpp>

#include <algorithm>
#include <string>
#include <vector>

#include "platforms/gcn/gcn_button.h"
#include "platforms/gcn/gcn_platform.h"

using std::vector;
using gcn::Button;

const int MENU_PADDING = 10;

// -----------------------------------------------------------------------
// GCNMenu
// -----------------------------------------------------------------------

GCNMenu::GCNMenu(const std::string& title,
                 const vector<GCNMenuButton>& buttons_to_display,
                 GCNPlatform* platform)
    : GCNWindow(platform) {
  int top_offset = MENU_PADDING;
  int max_button_size = 0;

  // Create the optional title if needed.
  gcn::Label* menu_title = NULL;
  if (!title.empty()) {
    menu_title = new gcn::Label(title);
    Container::add(menu_title, MENU_PADDING, top_offset);
    owned_widgets_.emplace_back(menu_title);
    top_offset += menu_title->getHeight() + MENU_PADDING;
    max_button_size = menu_title->getWidth();
  }

  vector<gcn::Button*> buttons;
  for (const GCNMenuButton& data : buttons_to_display) {
    if (data.separator) {
      top_offset += MENU_PADDING;
    } else {
      gcn::Button* button = new GCNButton(data.label);
      button->setActionEventId(data.action);
      button->addActionListener(this);
      button->setEnabled(data.enabled);

      if (!data.enabled)
        button->setForegroundColor(gcn::Color(100, 100, 100));

      Container::add(button, MENU_PADDING, top_offset);
      buttons.push_back(button);
      owned_widgets_.emplace_back(button);
      max_button_size = std::max(max_button_size, button->getWidth());
      top_offset += button->getHeight() + MENU_PADDING;
    }
  }

  // Now gothrough all the controls and make them the same width as the
  // largest button, centering the text.
  if (menu_title) {
    menu_title->setWidth(max_button_size);
    menu_title->setAlignment(gcn::Graphics::CENTER);
  }

  for (gcn::Button* button : buttons) {
    button->setWidth(max_button_size);
    button->setAlignment(gcn::Graphics::CENTER);
  }

  setSize((MENU_PADDING * 2) + max_button_size, top_offset);
}

// -----------------------------------------------------------------------

GCNMenu::~GCNMenu() {}

// -----------------------------------------------------------------------

void GCNMenu::action(const gcn::ActionEvent& actionEvent) {
  if (actionEvent.getId() == EVENT_CANCEL)
    platform_->windowCanceled(this);
  else
    platform_->receiveGCNMenuEvent(this, actionEvent.getId());
}
