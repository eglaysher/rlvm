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

#ifndef SRC_PLATFORMS_GCN_GCN_SCROLL_AREA_H_
#define SRC_PLATFORMS_GCN_GCN_SCROLL_AREA_H_

#include <guichan/widgets/scrollarea.hpp>

#include "base/notification_observer.h"
#include "base/notification_registrar.h"
#include "platforms/gcn/gcn_graphics.h"

// Copy of TMW's ScrollArea class, adapted to my system.
class GCNScrollArea : public gcn::ScrollArea, public NotificationObserver {
 public:
  explicit GCNScrollArea(gcn::Widget* widget);
  ~GCNScrollArea();

  // Logic function optionally adapts width or height of contents. This
  // depends on the scrollbar settings.
  virtual void logic() override;

  // Draws the scroll area.
  virtual void draw(gcn::Graphics* graphics) override;

  // Draws the background and border of the scroll area.
  virtual void drawFrame(gcn::Graphics* graphics) override;

 protected:
  enum BUTTON_DIR { UP, DOWN, LEFT, RIGHT };

  // Initializes object and shared data.
  void init();

  void drawButton(gcn::Graphics* graphics, BUTTON_DIR dir);
  void drawUpButton(gcn::Graphics* graphics);
  void drawDownButton(gcn::Graphics* graphics);
  void drawLeftButton(gcn::Graphics* graphics);
  void drawRightButton(gcn::Graphics* graphics);
  void drawVBar(gcn::Graphics* graphics);
  void drawHBar(gcn::Graphics* graphics);
  void drawVMarker(gcn::Graphics* graphics);
  void drawHMarker(gcn::Graphics* graphics);

  virtual void Observe(NotificationType type,
                       const NotificationSource& source,
                       const NotificationDetails& details) override;

  // Button images.
  std::unique_ptr<gcn::Image> buttonImages_[4][2];

  NotificationRegistrar registrar_;

  static ImageRect s_background;
  static ImageRect s_vMarker;
};  // end of class GCNScrollArea

#endif  // SRC_PLATFORMS_GCN_GCN_SCROLL_AREA_H_
