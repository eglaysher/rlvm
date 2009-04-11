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

#ifndef __SelectLongOperation_hpp__
#define __SelectLongOperation_hpp__

#include "MachineBase/LongOperation.hpp"
#include "Systems/Base/EventListener.hpp"
#include "Systems/Base/Renderable.hpp"

#include <vector>
#include <string>

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

// Selection LongOperation which waits for input
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

// Selection LongOperation for #SELBTN based selections
class ButtonSelectLongOperation : public SelectLongOperation,
                                  public Renderable {
 public:
  ButtonSelectLongOperation(RLMachine& machine,
                            const libReallive::SelectElement& commandElement);
  ~ButtonSelectLongOperation();

  // Overridden from EventListener:
  virtual void mouseMotion(const Point&);
  virtual bool mouseButtonStateChanged(MouseButton mouseButton, bool pressed);

  // Overridden from Renderable:
  virtual void render(std::ostream* tree);
};

#endif
