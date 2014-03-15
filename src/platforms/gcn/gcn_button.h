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

#ifndef SRC_PLATFORMS_GCN_GCN_BUTTON_H_
#define SRC_PLATFORMS_GCN_GCN_BUTTON_H_

#include <guichan/widgets/button.hpp>

#include <string>

#include "platforms/gcn/gcn_graphics.h"

// Graphical button overriding the default GUIchan ugliness.
//
// Spiritual copy of The Mana World's Button class; the resource images are
// actual copies.
class GCNButton : public gcn::Button {
 public:
  GCNButton();
  explicit GCNButton(const std::string& label);
  ~GCNButton();

  // Override from gcn::Button:
  virtual void draw(gcn::Graphics* graphics);

 private:
  void init();

  ImageRect& currentButtonImage();

  static ImageRect s_button;
  static ImageRect s_buttonhi;
  static ImageRect s_buttonpress;
  static ImageRect s_button_disabled;
};  // end of class GCNButton

#endif  // SRC_PLATFORMS_GCN_GCN_BUTTON_H_
