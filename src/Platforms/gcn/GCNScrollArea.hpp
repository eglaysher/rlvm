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

#ifndef SRC_PLATFORMS_GCN_GCNSCROLLAREA_HPP_
#define SRC_PLATFORMS_GCN_GCNSCROLLAREA_HPP_

#include <guichan/widgets/scrollarea.hpp>

#include "Platforms/gcn/GCNGraphics.hpp"

#include <boost/scoped_ptr.hpp>

/**
 * Copy of TMW's ScrollArea class, adapted to my system.
 */
class GCNScrollArea : public gcn::ScrollArea {
 public:
  explicit GCNScrollArea(gcn::Widget *widget);
  ~GCNScrollArea();

  /**
   * Logic function optionally adapts width or height of contents. This
   * depends on the scrollbar settings.
   */
  void logic();

  /**
   * Draws the scroll area.
   */
  void draw(gcn::Graphics *graphics);

  /**
   * Draws the background and border of the scroll area.
   */
  void drawFrame(gcn::Graphics *graphics);

 protected:
  enum BUTTON_DIR {
    UP,
    DOWN,
    LEFT,
    RIGHT
  };

  /// Initializes object and shared data.
  void init();

  void drawButton(gcn::Graphics *graphics, BUTTON_DIR dir);
  void drawUpButton(gcn::Graphics *graphics);
  void drawDownButton(gcn::Graphics *graphics);
  void drawLeftButton(gcn::Graphics *graphics);
  void drawRightButton(gcn::Graphics *graphics);
  void drawVBar(gcn::Graphics *graphics);
  void drawHBar(gcn::Graphics *graphics);
  void drawVMarker(gcn::Graphics *graphics);
  void drawHMarker(gcn::Graphics *graphics);

  static ImageRect s_background;
  static ImageRect s_vMarker;
  static boost::scoped_ptr<gcn::Image> s_buttonImages[4][2];
};  // end of class GCNScrollArea

#endif  // SRC_PLATFORMS_GCN_GCNSCROLLAREA_HPP_
