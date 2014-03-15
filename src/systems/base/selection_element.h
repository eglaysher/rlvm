// -*- Mode: C++; tab-width:2; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi:tw=80:et:ts=2:sts=2
//
// -----------------------------------------------------------------------
//
// This file is part of RLVM, a RealLive virtual machine clone.
//
// -----------------------------------------------------------------------
//
// Copyright (C) 2007 Elliot Glaysher
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
//
// -----------------------------------------------------------------------

#ifndef SRC_SYSTEMS_BASE_SELECTION_ELEMENT_H_
#define SRC_SYSTEMS_BASE_SELECTION_ELEMENT_H_

#include <boost/shared_ptr.hpp>

#include <functional>

#include "systems/base/rect.h"

class Surface;
class System;

// Represents a clickable element inside TextWindows.
class SelectionElement {
 public:
  SelectionElement(System& system,
                   const boost::shared_ptr<Surface>& normal_image,
                   const boost::shared_ptr<Surface>& highlighted_image,
                   const std::function<void(int)>& selection_callback,
                   int id,
                   const Point& pos);
  ~SelectionElement();

  void setSelectionCallback(const std::function<void(int)>& func);

  void setMousePosition(const Point& pos);
  bool handleMouseClick(const Point& pos, bool pressed);

  void render();

 private:
  bool is_highlighted_;

  int id_;

  // Upper right location of the button
  Point pos_;

  boost::shared_ptr<Surface> normal_image_;
  boost::shared_ptr<Surface> highlighted_image_;

  // Callback function for when item is selected.
  std::function<void(int)> selection_callback_;

  System& system_;

  bool isHighlighted(const Point& p);
};

#endif  // SRC_SYSTEMS_BASE_SELECTION_ELEMENT_H_
