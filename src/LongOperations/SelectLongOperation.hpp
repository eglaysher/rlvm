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

// Base class for all selections.
class SelectLongOperation : public LongOperation {
 public:
  struct Option {
    // Whether we show this option.
    bool shown;

    // What to print as the option.
    std::string str;
  };

  SelectLongOperation(RLMachine& machine,
                      const libReallive::SelectElement& commandElement);

  // Selects by index among options.
  void selected(int num);

  // Selects by the string text. Returns true if successful.
  bool selectOption(const std::string& str);

  // Returns the underlying list of options.
  std::vector<std::string> options() const;

  // Overridden from LongOperation:
  virtual bool operator()(RLMachine& machine);

 protected:
  RLMachine& machine_;

  // A list of the string literals of the options.
  std::vector<Option> options_;

  // If positive, the option that was selected (and that this object should
  // put in the store register).
  int return_value_;
};

// Selection LongOperation which waits for input in the TextBox. Most work is
// passed off to the TextWindow which does all the drawing.
//
// TODO: Move that code into here.
class NormalSelectLongOperation : public SelectLongOperation {
 public:
  NormalSelectLongOperation(RLMachine& machine,
                            const libReallive::SelectElement& commandElement);
  virtual ~NormalSelectLongOperation();

  // Overridden from EventListener:
  virtual void mouseMotion(const Point&);
  virtual bool mouseButtonStateChanged(MouseButton mouseButton, bool pressed);

 private:
  boost::shared_ptr<TextWindow> text_window_;
};

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
  virtual ~ButtonSelectLongOperation();

  // Overridden from EventListener:
  virtual void mouseMotion(const Point&);
  virtual bool mouseButtonStateChanged(MouseButton mouseButton, bool pressed);

  // Overridden from Renderable:
  virtual void render(std::ostream* tree);

 private:
  struct ButtonOption {
    // The id to return when this button is clicked.
    int id;

    // Text representations to blit to the screen.
    boost::shared_ptr<Surface> default_surface;
    boost::shared_ptr<Surface> select_surface;

    // Where to render the above surface to.
    Rect bounding_rect;
  };

  void renderTextSurface(const boost::shared_ptr<Surface>& text_surface,
                         const Rect& bounding_rect);

  // ????
  int basepos_x_, basepos_y_;

  // ????
  int reppos_x_, reppos_y_;

  int moji_size_;

  // If positive, the currently highlighted text button. (As an index into
  // buttons_, not as the button's id.)
  int highlighted_item_;

  // Properties when rendering our back surface as a color mask.
  RGBAColour window_bg_colour_;
  int window_filter_;

  // g00 pattern numbers in name_surface_ for which pattern to display during
  // certain states.
  int normal_frame_;
  int select_frame_;

  // Surface loaded from #SELBTN.xxx.NAME.
  boost::shared_ptr<const Surface> name_surface_;

  // Surface loaded from #SELBTN.xxx.BACK.
  boost::shared_ptr<const Surface> back_surface_;

  std::vector<ButtonOption> buttons_;
};

#endif  // SRC_LONGOPERATIONS_SELECTLONGOPERATION_HPP_
