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

#ifndef SRC_SYSTEMS_BASE_SELECTIONELEMENT_HPP_
#define SRC_SYSTEMS_BASE_SELECTIONELEMENT_HPP_

#include <boost/function.hpp>
#include <boost/shared_ptr.hpp>

#include "Systems/Base/Rect.hpp"

class GraphicsSystem;
class Surface;

// Represents a clickable element inside TextWindows.
class SelectionElement {
 public:
  SelectionElement(GraphicsSystem& gs,
                   const boost::shared_ptr<Surface>& normal_image,
                   const boost::shared_ptr<Surface>& highlighted_image,
                   const boost::function<void(int)>& selection_callback,
                   int id, const Point& pos);
  ~SelectionElement();

  void setSelectionCallback(const boost::function<void(int)>& func);

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
  boost::function<void(int)> selection_callback_;

  GraphicsSystem& graphics_system_;

  bool isHighlighted(const Point& p);
};

#endif  // SRC_SYSTEMS_BASE_SELECTIONELEMENT_HPP_
