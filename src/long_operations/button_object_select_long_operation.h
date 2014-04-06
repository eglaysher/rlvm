// -*- Mode: C++; tab-width:2; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi:tw=80:et:ts=2:sts=2
//
// -----------------------------------------------------------------------
//
// This file is part of RLVM, a RealLive virtual machine clone.
//
// -----------------------------------------------------------------------
//
// Copyright (C) 2013 Elliot Glaysher
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

#ifndef SRC_LONG_OPERATIONS_BUTTON_OBJECT_SELECT_LONG_OPERATION_H_
#define SRC_LONG_OPERATIONS_BUTTON_OBJECT_SELECT_LONG_OPERATION_H_

#include <utility>
#include <vector>

#include "libreallive/gameexe.h"
#include "machine/long_operation.h"

class GraphicsObject;

// LongOperation that uses GraphicsObjects to present a choice to the
// user. This doesn't derive from SelectLongOperation because these commands
// are normal commands; they aren't like the select() family.
class ButtonObjectSelectLongOperation : public LongOperation {
 public:
  ButtonObjectSelectLongOperation(RLMachine& machine, int group);
  virtual ~ButtonObjectSelectLongOperation();

  void set_cancelable() { cancelable_ = true; }

  // Overridden from EventListener:
  virtual void MouseMotion(const Point& point) override;
  virtual bool MouseButtonStateChanged(MouseButton mouseButton,
                                       bool pressed) override;

  // Overridden from LongOperation:
  virtual bool operator()(RLMachine& machine);

 private:
  // Sets the override data (changes pattern number and offset) based on
  // |type|, which is a string representation of the hover state.
  void SetButtonOverride(GraphicsObject* object, const char* type);

  RLMachine& machine_;

  int group_;

  // Whether we cancel when the right mouse button is pressed.
  bool cancelable_;

  // Whether the user has accepted an answer.
  bool has_return_value_;

  // If |has_return_value_|, this contains the value we should pass back to the
  // RL script.
  int return_value_;

  // A reference to our game configuration file.
  Gameexe& gameexe_;

  // All objects that are buttons in |group_|. First is the object, Second is
  // the parent object, or NULL if first isn't a child object.
  typedef std::pair<GraphicsObject*, GraphicsObject*> ButtonPair;
  typedef std::vector<ButtonPair> ObjVector;
  ObjVector buttons_;

  // The graphics object that the mouse cursor is hovering over.
  GraphicsObject* currently_hovering_button_;

  // The graphics object of the button currently being pressed. If we have both
  // mousedown and mouseup in the same button, we consider that a mouse press.
  GraphicsObject* currently_pressed_button_;
};

#endif  // SRC_LONG_OPERATIONS_BUTTON_OBJECT_SELECT_LONG_OPERATION_H_
