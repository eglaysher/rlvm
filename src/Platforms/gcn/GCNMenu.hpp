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

#ifndef __GCNMenu_hpp__
#define __GCNMenu_hpp__

#include "Platforms/gcn/GCNWindow.hpp"

#include <guichan/actionlistener.hpp>

class GCNMenu;

// -----------------------------------------------------------------------

/**
 * Input struct for a GCNMenu. GCNPlatform creates a vector of this struct.
 */
struct GCNMenuButton {
  GCNMenuButton() : enabled(false), separator(false) {}

  std::string label;
  std::string action;
  bool enabled;
  bool separator;
};

/**
 * A GCNMenu is a window that just auto-lays out a set of buttons.
 */
class GCNMenu : public GCNWindow,
                public gcn::ActionListener
{
public:
  /**
   * Creates a menu out of a set of buttons
   *
   * @param title The title for an optional label. If this is empty, no label
   *              is presented to the user.
   * @param buttons A set of button definitions.
   * @param platform Our parent.
   */
  GCNMenu(const std::string& title,
          const std::vector<GCNMenuButton>& buttons,
          GCNPlatform* platform);
  ~GCNMenu();

  // Overriden from gcn::ActionListener:
  virtual void action(const gcn::ActionEvent& actionEvent);

};  // end of class GCNMenu


#endif
