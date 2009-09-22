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

#ifndef SRC_SYSTEMS_BASE_TEXTWAKUNORMAL_HPP_
#define SRC_SYSTEMS_BASE_TEXTWAKUNORMAL_HPP_

#include <iosfwd>
#include <string>
#include <boost/scoped_ptr.hpp>

#include "Systems/Base/TextWaku.hpp"

class Point;
class Rect;
class RLMachine;
class Size;
class Surface;
class System;
class TextWindow;
class TextWindowButton;

/**
 * Container class that owns all text window decorations.
 *
 * Window decorations are defined with \#WAKU.<setno>.<no>. Gameexe.ini keys.
 */
class TextWakuNormal : public TextWaku {
 public:
  TextWakuNormal(System& system, TextWindow& window, int setno, int no);
  ~TextWakuNormal();

  virtual void execute();
  virtual void render(std::ostream* tree, Point box_location,
                      Size namebox_size);
  virtual bool getSize(Size& out) const;

  /**
   * @todo These two methods shouldn't really exist; I need to redo plumbing of
   *       events so that these aren't routed through TextWindow, but are
   *       instead some sort of listener. I'm currently thinking that the
   *       individual buttons that need to handle events should be listeners.
   */
  virtual void setMousePosition(const Point& pos);
  virtual bool handleMouseClick(RLMachine& machine, const Point& pos,
                                bool pressed);

 private:
  /// Renders all the buttons in |button_map_|.
  void renderButtons();

  /// Loads all bitmaps and sets up all window buttons for this waku.
  void loadWindowWaku();

  void setWakuMain(const std::string& name);

  /**
   * Loads the graphics file name as the mask for represents the areas
   * of the text window that should be shaded.
   */
  void setWakuBacking(const std::string& name);

  /**
   * Loads the graphics file name as the image with all the button
   * images used when drawing
   */
  void setWakuButton(const std::string& name);

  /// The system we are a part of.
  System& system_;

  /// The text window we decorate. TODO: Figure out how wrong this is when we
  /// are a name box.
  TextWindow& window_;

  int setno_, no_;

  boost::shared_ptr<Surface> waku_main_;
  boost::shared_ptr<Surface> waku_backing_;
  boost::shared_ptr<Surface> waku_button_;

  /**
   * @name Buttons in this text box
   *
   * Attached action buttons defined in the
   * \#WAKU.index1.index2.XXX_BOX properties. These actions represent
   * things such as moving the text box, clearing the text box, moving
   * forward or backwards in message history, and farcall()-ing a
   * custom handler (EXBTN_index_BOX).
   *
   * @{
   */
  boost::scoped_ptr<TextWindowButton> button_map_[12];
  /// @}
};  // end of class TextWakuNormal

#endif  // SRC_SYSTEMS_BASE_TEXTWAKUNORMAL_HPP_
