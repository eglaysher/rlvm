// -*- Mode: C++; tab-width:2; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi:tw=80:et:ts=2:sts=2
//
// -----------------------------------------------------------------------
//
// This file is part of RLVM, a RealLive virtual machine clone.
//
// -----------------------------------------------------------------------
//
// Copyright (C) 2009 Elliot Glaysher
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
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
// -----------------------------------------------------------------------

#ifndef SRC_SYSTEMS_BASE_TEXT_WAKU_TYPE4_H_
#define SRC_SYSTEMS_BASE_TEXT_WAKU_TYPE4_H_

#include <string>

#include "systems/base/text_waku.h"
#include "systems/base/surface.h"

// Waku which is a modified Ninebox. Instead of a ninebox, it's really a 12-box
// where four of the entries aren't used and the center is never defined. This
// box expands to fill whatever size is passed into render().
class TextWakuType4 : public TextWaku {
 public:
  TextWakuType4(System& system, TextWindow& window, int setno, int no);
  virtual ~TextWakuType4();

  virtual void Execute() override;
  virtual void Render(std::ostream* tree,
                      Point box_location,
                      Size namebox_size) override;

  // We have no size other than what is passed to |namebox_size|. Always
  // returns false and resets |out|.
  virtual Size GetSize(const Size& text_surface) const override;
  virtual Point InsertionPoint(const Rect& waku_rect,
                               const Size& padding,
                               const Size& surface_size,
                               bool center) const override;

  // TODO(erg): These two methods shouldn't really exist; I need to redo
  // plumbing of events so that these aren't routed through TextWindow, but are
  // instead some sort of listener. I'm currently thinking that the individual
  // buttons that need to handle events should be listeners.
  virtual void SetMousePosition(const Point& pos) override;
  virtual bool HandleMouseClick(RLMachine& machine,
                                const Point& pos,
                                bool pressed) override;

 private:
  void SetWakuMain(const std::string& name);

  // Returns |cached_backing_|, shrinking or enlarging it to |size|.
  const std::shared_ptr<Surface>& GetWakuBackingOfSize(Size size);

  // The system we are a part of.
  System& system_;

  // The text window we decorate. TODO: Figure out how wrong this is when we
  // are a name box.
  TextWindow& window_;

  int setno_, no_;

  // Additional area that adds to the filter backing in four directions.
  int area_top_, area_bottom_, area_left_, area_right_;

  // The surface that we pick pieces of our textbox against.
  std::shared_ptr<const Surface> waku_main_;

  // A cached backing regenerated whenever the namebox size changes
  std::shared_ptr<Surface> cached_backing_;

  // G00 regions in |waku_main_|.
  Surface::GrpRect top_left;
  Surface::GrpRect top_center;
  Surface::GrpRect top_right;

  Surface::GrpRect left_side;
  Surface::GrpRect right_side;

  Surface::GrpRect bottom_left;
  Surface::GrpRect bottom_center;
  Surface::GrpRect bottom_right;
};

#endif  // SRC_SYSTEMS_BASE_TEXT_WAKU_TYPE4_H_
