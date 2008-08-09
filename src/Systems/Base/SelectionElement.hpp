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
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
//
// -----------------------------------------------------------------------

#ifndef __SelectionElement_hpp__
#define __SelectionElement_hpp__

#include <boost/shared_ptr.hpp>

class RLMachine;
class Surface;
class Point;

/**
 * Represents a clickable
 *
 * @todo Later on, see if I can factor out common code from
 *       @c TextWindowButton.
 */
class SelectionElement
{
private:
  bool m_isHighlighted;

  int m_id;

  // Upper right location of the button
  Point m_pos;

  boost::shared_ptr<Surface> m_normalImage;
  boost::shared_ptr<Surface> m_highlightedImage;

  /// Callback function for when item is selected.
  boost::function<void(int)> m_selectionCallback;

  bool isHighlighted(const Point& p);

public:
  SelectionElement(const boost::shared_ptr<Surface>& normalImage,
                   const boost::shared_ptr<Surface>& highlightedImage,
                   const boost::function<void(int)>& selectionCallback,
                   int id, const Point& pos);
  ~SelectionElement();

  void setSelectionCallback(const boost::function<void(int)>& func);

  void setMousePosition(RLMachine& machine, const Point& pos);
  bool handleMouseClick(RLMachine& machine, const Point& pos, bool pressed);

  void render();
};

#endif
