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

#include "Platforms/gcn/GCNMenu.hpp"

#include "Platforms/gcn/GCNPlatform.hpp"

#include <guichan/widgets/button.hpp>

using std::vector;
using gcn::Button;

const int MENU_PADDING = 10;

// -----------------------------------------------------------------------
// GCNMenu
// -----------------------------------------------------------------------

GCNMenu::GCNMenu(const vector<GCNMenuButton>& buttons_to_display,
                 const Size& screen_size,
                 GCNMenuListener* listener)
  : listener_(listener) {
  int top_offset = MENU_PADDING;
  int max_button_size = 0;

  vector<gcn::Button*> buttons;

  for (vector<GCNMenuButton>::const_iterator it = buttons_to_display.begin();
       it != buttons_to_display.end(); ++it) {
    if (it->separator) {
      top_offset += MENU_PADDING;
    } else {
      gcn::Button* button = new gcn::Button(it->label);
      button->setActionEventId(it->action);
      button->addActionListener(this);
      button->setEnabled(it->enabled);

      Container::add(button, MENU_PADDING, top_offset);
      buttons.push_back(button);

      max_button_size = std::max(max_button_size, button->getWidth());
      top_offset += button->getHeight() + MENU_PADDING;
    }
  }

  // Now iterate through all the buttons and make them the same width as the
  // largest button, centering the text.
  for (vector<gcn::Button*>::iterator it = buttons.begin();
       it != buttons.end(); ++it) {
    (*it)->setWidth(max_button_size);
    (*it)->setAlignment(gcn::Graphics::CENTER);
  }

  setSize((MENU_PADDING*2) + max_button_size, top_offset);
  centerInWindow(screen_size);
}

// -----------------------------------------------------------------------

GCNMenu::~GCNMenu() {
/*
  Do we need this for ownership reasons?

  for (vector<Button*>::iterator it = buttons_to_deallocate_.begin();
       it != buttons_to_deallocate_.end(); ++it) {
    it->removeActionListener(this);
  }
*/
}

// -----------------------------------------------------------------------

void GCNMenu::action(const gcn::ActionEvent& actionEvent) {
  if (actionEvent.getId() == EVENT_CANCEL)
    listener_->windowCanceled(this);
  else
    listener_->receiveGCNMenuEvent(this, actionEvent.getId());
}
