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
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
// -----------------------------------------------------------------------

#ifndef __TextWaku_hpp__
#define __TextWaku_hpp__

class Point;
class RLMachine;
class Size;
class System;
class TextWindow;

class TextWaku {
 public:
  // Factory method which automatically picks the correct subclass of TextWaku
  // for the implementation at hand.
  static TextWaku* Create(System& system, TextWindow& window, int setno, int no);

  virtual ~TextWaku();

  virtual void execute() = 0;
  virtual void render(std::ostream* tree, Point box_location) = 0;

  virtual void setMousePosition(const Point& pos) { };
  virtual bool handleMouseClick(RLMachine& machine, const Point& pos,
                                bool pressed) {
    return false;
  }

  virtual Size size() const = 0;
};

#endif
