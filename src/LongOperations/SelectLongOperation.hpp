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

#ifndef SRC_LONGOPERATIONS_SELECTLONGOPERATION_HPP_
#define SRC_LONGOPERATIONS_SELECTLONGOPERATION_HPP_

#include <vector>
#include <string>
#include <boost/shared_ptr.hpp>

#include "MachineBase/LongOperation.hpp"
#include "Systems/Base/Colour.hpp"
#include "Systems/Base/EventListener.hpp"
#include "Systems/Base/Rect.hpp"
#include "Systems/Base/Renderable.hpp"
#include "Systems/Base/Surface.hpp"

class RLMachine;
class TextWindow;

namespace libReallive {
class SelectElement;
}

// -----------------------------------------------------------------------

/// Base class for all selections.
class SelectLongOperation : public LongOperation {
 public:
  SelectLongOperation(RLMachine& machine,
                      const libReallive::SelectElement& commandElement);

  // Selects by index among options.
  void selected(int num);

  // Selects by the string text. Returns true if successful.
  bool selectOption(const std::string& str);

  // Returns the underlying list of options.
  const std::vector<std::string>& options() const {
    return options_;
  }

  // Overridden from LongOperation:
  virtual bool operator()(RLMachine& machine);

 protected:
  // A list of the string literals of the options.
  std::vector<std::string> options_;

  // If positive, the option that was selected (and that this object should
  // put in the store register).
  int return_value_;
};

// -----------------------------------------------------------------------

// Selection LongOperation which waits for input in the TextBox. Most work is
// passed off to the TextWindow which does all the drawing.
//
// TODO: Move that code into here.
class NormalSelectLongOperation : public SelectLongOperation {
 public:
  NormalSelectLongOperation(RLMachine& machine,
                            const libReallive::SelectElement& commandElement);
  ~NormalSelectLongOperation();

  // Overridden from EventListener:
  virtual void mouseMotion(const Point&);
  virtual bool mouseButtonStateChanged(MouseButton mouseButton, bool pressed);

 private:
  RLMachine& machine_;

  boost::shared_ptr<TextWindow> text_window_;
};

// -----------------------------------------------------------------------

// Selection LongOperation for #SELBTN based selections. Most of the work is
// done in this class, and is rendered through Renderable.
//
// Haeleth has no documentation on SELBTNs. What follows are guesses based on
// xclannad.
class ButtonSelectLongOperation : public SelectLongOperation,
                                  public Renderable {
 public:
  ButtonSelectLongOperation(RLMachine& machine,
                            const libReallive::SelectElement& commandElement,
                            int selbtn_set);
  ~ButtonSelectLongOperation();

  // Overridden from EventListener:
  virtual void mouseMotion(const Point&);
  virtual bool mouseButtonStateChanged(MouseButton mouseButton, bool pressed);

  // Overridden from Renderable:
  virtual void render(std::ostream* tree);


 private:
  void renderTextSurface(const boost::shared_ptr<Surface>& text_surface,
                         const Rect& bounding_rect);

  RLMachine& machine_;

  // ????
  int basepos_x_, basepos_y_;

  // ????
  int reppos_x_, reppos_y_;

  int moji_size_;

  // If positive, the currently highlighted text button.
  int highlighted_item_;

  // Surface loaded from #SELBTN.xxx.NAME.
  boost::shared_ptr<Surface> name_surface_;

  // Surface loaded from #SELBTN.xxx.BACK.
  boost::shared_ptr<Surface> back_surface_;

  // Text representations to blit to the screen.
  std::vector<boost::shared_ptr<Surface> > default_text_surfaces_;
  std::vector<boost::shared_ptr<Surface> > select_text_surfaces_;

  // A set of rects describing where to render the back_surface_ to.
  std::vector<Rect> bounding_rects_;
};

#endif  // SRC_LONGOPERATIONS_SELECTLONGOPERATION_HPP_
