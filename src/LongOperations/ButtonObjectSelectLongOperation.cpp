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

#include "LongOperations/ButtonObjectSelectLongOperation.hpp"

#include "MachineBase/RLMachine.hpp"
#include "Systems/Base/GraphicsObject.hpp"
#include "Systems/Base/GraphicsSystem.hpp"
#include "Systems/Base/ParentGraphicsObjectData.hpp"
#include "Systems/Base/System.hpp"

ButtonObjectSelectLongOperation::ButtonObjectSelectLongOperation(
    RLMachine& machine,
    int group)
    : machine_(machine),
      group_(group),
      has_return_value_(false),
      return_value_(-1),
      gameexe_(machine.system().gameexe()),
      currently_hovering_button_(NULL),
      currently_pressed_button_(NULL) {
  GraphicsSystem& graphics = machine.system().graphics();
  AllocatedLazyArrayIterator<GraphicsObject> it =
      graphics.foregroundObjects().allocated_begin();
  AllocatedLazyArrayIterator<GraphicsObject> end =
      graphics.foregroundObjects().allocated_end();
  for (; it != end; ++it) {
    if (it->isButton() && it->buttonGroup() == group_) {
      buttons_.push_back(std::make_pair(&*it,
                                        static_cast<GraphicsObject*>(NULL)));
    } else if (it->hasObjectData()) {
      ParentGraphicsObjectData* parent =
          dynamic_cast<ParentGraphicsObjectData*>(&it->objectData());

      if (parent) {
        AllocatedLazyArrayIterator<GraphicsObject> child_it =
            parent->objects().allocated_begin();
        AllocatedLazyArrayIterator<GraphicsObject> child_end =
            parent->objects().allocated_end();
        for (; child_it != child_end; ++child_it) {
          if (child_it->isButton() && child_it->buttonGroup() == group_) {
            buttons_.push_back(std::make_pair(&*child_it, &*it));
          }
        }
      }
    }
  }

  // Initialize overrides on all buttons that we'll use.
  for (ObjVector::iterator it = buttons_.begin(); it != buttons_.end(); ++it) {
    setButtonOverride(it->first, "NORMAL");
  }
}

ButtonObjectSelectLongOperation::~ButtonObjectSelectLongOperation() {
  // Disable overrides on all graphics objects we've dealt with.
  for (ObjVector::iterator it = buttons_.begin(); it != buttons_.end(); ++it) {
    it->first->clearButtonOverrides();
  }
}

void ButtonObjectSelectLongOperation::mouseMotion(const Point& point) {
  GraphicsObject* hovering_button = NULL;

  for (ObjVector::iterator it = buttons_.begin(); it != buttons_.end(); ++it) {
    if (it->first->hasObjectData()) {
      GraphicsObjectData* data = &it->first->objectData();
      Rect screen_rect = data->dstRect(*it->first, it->second);

      if (screen_rect.contains(point))
        hovering_button = it->first;
    }
  }

  if (currently_hovering_button_ != hovering_button) {
    if (currently_hovering_button_) {
      setButtonOverride(currently_hovering_button_, "NORMAL");

      if (currently_hovering_button_ == currently_pressed_button_)
        currently_pressed_button_ = NULL;
    }

    if (hovering_button)
      setButtonOverride(hovering_button, "HIT");
  }

  currently_hovering_button_ = hovering_button;
}

bool ButtonObjectSelectLongOperation::mouseButtonStateChanged(
    MouseButton mouseButton, bool pressed) {
  if (mouseButton == MOUSE_LEFT) {
    if (pressed) {
      currently_pressed_button_ = currently_hovering_button_;
      setButtonOverride(currently_pressed_button_, "PUSH");
    } else {
      if (currently_hovering_button_ &&
          currently_hovering_button_ == currently_pressed_button_) {
        has_return_value_ = true;
        return_value_ = currently_pressed_button_->buttonNumber();
        setButtonOverride(currently_pressed_button_, "HIT");
      }
    }

    // Changes override properties doesn't automatically refresh the screen the
    // way mouse movement does.
    machine_.system().graphics().forceRefresh();

    return true;
  }

  return false;
}

bool ButtonObjectSelectLongOperation::operator()(RLMachine& machine) {
  if (has_return_value_) {
    machine.setStoreRegister(return_value_);
    return true;
  } else {
    return false;
  }
}

void ButtonObjectSelectLongOperation::setButtonOverride(
    GraphicsObject* object,
    const char* type) {
  int action = object->buttonAction();

  GameexeInterpretObject key = gameexe_("BTNOBJ.ACTION", action, type);
  if (key.exists()) {
    const std::vector<int>& ints = key.to_intVector();
    object->setButtonOverrides(ints[0], ints[2], ints[3]);
  }
}
