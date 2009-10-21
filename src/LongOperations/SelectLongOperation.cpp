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

#include "Precompiled.hpp"

// -----------------------------------------------------------------------

#include "LongOperations/SelectLongOperation.hpp"

#include <vector>
#include <string>
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>

#include "MachineBase/LongOperation.hpp"
#include "MachineBase/RLMachine.hpp"
#include "Systems/Base/EventListener.hpp"
#include "Systems/Base/EventSystem.hpp"
#include "Systems/Base/GraphicsSystem.hpp"
#include "Systems/Base/Renderable.hpp"
#include "Systems/Base/System.hpp"
#include "Systems/Base/TextSystem.hpp"
#include "Systems/Base/TextWindow.hpp"
#include "Utilities/StringUtilities.hpp"

#include "libReallive/bytecode.h"
#include "libReallive/expression.h"
#include "libReallive/gameexe.h"

using boost::bind;
using boost::shared_ptr;
using std::string;
using std::vector;
using std::distance;
using libReallive::SelectElement;
using libReallive::CommandElement;

// -----------------------------------------------------------------------
// SelectLongOperation
// -----------------------------------------------------------------------
SelectLongOperation::SelectLongOperation(RLMachine& machine,
                                         const SelectElement& commandElement)
    : return_value_(-1) {
  const vector<SelectElement::Param>& params = commandElement.getRawParams();
  for (unsigned int i = 0; i < params.size(); ++i) {
    std::string evaluated_native =
        libReallive::evaluatePRINT(machine, params[i].text);
    std::string utf8str = cp932toUTF8(evaluated_native,
                                      machine.getTextEncoding());

    options_.push_back(utf8str);
  }
}

// -----------------------------------------------------------------------

void SelectLongOperation::selected(int num) {
  return_value_ = num;
}

// -----------------------------------------------------------------------

bool SelectLongOperation::selectOption(const std::string& str) {
  std::vector<std::string>::iterator it =
    find(options_.begin(), options_.end(), str);

  if (it != options_.end()) {
    selected(distance(options_.begin(), it));
    return true;
  }

  return false;
}

// -----------------------------------------------------------------------

bool SelectLongOperation::operator()(RLMachine& machine) {
  if (return_value_ != -1) {
    machine.setStoreRegister(return_value_);
    return true;
  } else {
    return false;
  }
}

// -----------------------------------------------------------------------
// NormalSelectLongOperation
// -----------------------------------------------------------------------
NormalSelectLongOperation::NormalSelectLongOperation(
    RLMachine& machine,
    const libReallive::SelectElement& commandElement)
    : SelectLongOperation(machine, commandElement),
      machine_(machine),
      text_window_(machine.system().text().currentWindow()) {
  machine.system().text().setInSelectionMode(true);
  text_window_->setVisible(true);
  text_window_->startSelectionMode();
  text_window_->setSelectionCallback(
    bind(&NormalSelectLongOperation::selected, this, _1));

  for (size_t i = 0; i < options_.size(); ++i) {
    text_window_->addSelectionItem(options_[i]);
  }

  machine.system().graphics().markScreenAsDirty(GUT_TEXTSYS);
}

// -----------------------------------------------------------------------

NormalSelectLongOperation::~NormalSelectLongOperation() {
  text_window_->endSelectionMode();
  machine_.system().text().setInSelectionMode(false);
}

// -----------------------------------------------------------------------

void NormalSelectLongOperation::mouseMotion(const Point& pos) {
  // Tell the text system about the move
  machine_.system().text().setMousePosition(pos);
}

// -----------------------------------------------------------------------

bool NormalSelectLongOperation::mouseButtonStateChanged(MouseButton mouseButton,
                                                        bool pressed) {
  EventSystem& es = machine_.system().event();

  switch (mouseButton) {
    case MOUSE_LEFT: {
      Point pos = es.getCursorPos();
      machine_.system().text().handleMouseClick(machine_, pos, pressed);
      return true;
      break;
    }
    case MOUSE_RIGHT: {
      if (pressed) {
        machine_.system().showSyscomMenu(machine_);
        return true;
      }
      break;
    }
    default:
      break;
  }

  return false;
}

// -----------------------------------------------------------------------
// ButtonSelectLongOperation
// -----------------------------------------------------------------------
ButtonSelectLongOperation::ButtonSelectLongOperation(
    RLMachine& machine,
    const libReallive::SelectElement& commandElement,
    int selbtn_set)
    : SelectLongOperation(machine, commandElement),
      machine_(machine),
      highlighted_item_(-1) {
  machine.system().graphics().addRenderable(this);

  // Load all the data about this #SELBTN from the Gameexe.ini file.
  Gameexe& gexe = machine.system().gameexe();
  GameexeInterpretObject selbtn(gexe("SELBTN", selbtn_set));

  vector<int> vec = selbtn("BASEPOS");
  basepos_x_ = vec.at(0);
  basepos_y_ = vec.at(1);

  vec = selbtn("REPPOS");
  reppos_x_ = vec.at(0);
  reppos_y_ = vec.at(1);

  vec = selbtn("CENTERING");
  int center_x = vec.at(0);
  int center_y = vec.at(1);

  moji_size_ = selbtn("MOJISIZE");

  // Retrieve the parameters needed to render as a color mask.
  shared_ptr<TextWindow> window = machine.system().text().currentWindow();
  window_bg_colour_ = window->colour();
  window_filter_ = window->filter();

  int default_colour_num_ = selbtn("MOJIDEFAULTCOL");
  int select_colour_num_ = selbtn("MOJISELECTCOL");
  if (default_colour_num_ == select_colour_num_)
    select_colour_num_ = 1;  // For CLANNAD

  GraphicsSystem& gs = machine.system().graphics();
  name_surface_ = gs.loadNonCGSurfaceFromFile(selbtn("NAME"));
  back_surface_ = gs.loadNonCGSurfaceFromFile(selbtn("BACK"));

  // Pick the correct font colour
  vec = gexe("COLOR_TABLE", default_colour_num_);
  RGBColour default_colour(vec.at(0), vec.at(1), vec.at(2));
  vec = gexe("COLOR_TABLE", select_colour_num_);
  RGBColour select_colour(vec.at(0), vec.at(1), vec.at(2));
  vec = gexe("COLOR_TABLE", 255);
  RGBColour shadow_colour(vec.at(0), vec.at(1), vec.at(2));

  // Build graphic representations of the choices to display to the user.
  TextSystem& ts = machine.system().text();
  for (size_t i = 0; i < options_.size(); ++i) {
    const std::string& text = options_[i];

    default_text_surfaces_.push_back(ts.renderText(
        text, moji_size_, 0, 0, default_colour, &shadow_colour));
    select_text_surfaces_.push_back(ts.renderText(
        text, moji_size_, 0, 0, select_colour, &shadow_colour));
  }

  // Calculate out the bounding rectangles for all the options.
  Size screen_size = machine.system().graphics().screenSize();
  int baseposx = 0;
  if (center_x) {
    int totalwidth = ((options_.size() - 1) * reppos_x_) +
                     back_surface_->size().width();
    baseposx = (screen_size.width() / 2) - (totalwidth / 2);
  } else {
    baseposx = basepos_x_;
  }

  int baseposy = 0;
  if (center_y) {
    int totalheight = ((options_.size() - 1) * reppos_y_) +
                      back_surface_->size().height();
    baseposy = (screen_size.height() / 2) - (totalheight / 2);
  } else {
    baseposy = basepos_y_;
  }

  for (int i = 0; i < options_.size(); i++) {
    bounding_rects_.push_back(Rect(baseposx, baseposy, back_surface_->size()));

    baseposx += reppos_x_;
    baseposy += reppos_y_;
  }

  machine.system().graphics().markScreenAsDirty(GUT_TEXTSYS);
}

// -----------------------------------------------------------------------

ButtonSelectLongOperation::~ButtonSelectLongOperation() {
  machine_.system().graphics().removeRenderable(this);
}

// -----------------------------------------------------------------------

void ButtonSelectLongOperation::mouseMotion(const Point& p) {
  for (int i = 0; i < options_.size(); i++) {
    Rect bounding_rect = bounding_rects_[i];
    if (bounding_rect.contains(p)) {
      highlighted_item_ = i;
      return;
    }
  }

  highlighted_item_ = -1;
}

// -----------------------------------------------------------------------

bool ButtonSelectLongOperation::mouseButtonStateChanged(
    MouseButton mouseButton, bool pressed) {
  EventSystem& es = machine_.system().event();

  switch (mouseButton) {
    case MOUSE_LEFT: {
      Point pos = es.getCursorPos();
      for (int i = 0; i < options_.size(); i++) {
        Rect bounding_rect = bounding_rects_[i];
        if (bounding_rect.contains(pos)) {
          selected(i);
          break;
        }
      }

      return true;
      break;
    }
    case MOUSE_RIGHT: {
      if (pressed) {
        machine_.system().showSyscomMenu(machine_);
        return true;
      }
      break;
    }
    default:
      break;
  }

  return false;
}

// -----------------------------------------------------------------------

void ButtonSelectLongOperation::render(std::ostream* tree) {
  for (int i = 0; i < options_.size(); i++) {
    Rect bounding_rect = bounding_rects_[i];

    back_surface_->renderToScreenAsColorMask(
        back_surface_->rect(), bounding_rect, window_bg_colour_,
        window_filter_);
    name_surface_->renderToScreen(name_surface_->rect(), bounding_rect);

    if (i == highlighted_item_) {
      renderTextSurface(select_text_surfaces_[i], bounding_rect);
    } else {
      renderTextSurface(default_text_surfaces_[i], bounding_rect);
    }
  }
}

// -----------------------------------------------------------------------

void ButtonSelectLongOperation::renderTextSurface(
    const boost::shared_ptr<Surface>& text_surface, const Rect& bounding_rect) {
  // Render the correct text in the correct place.
  Rect text_bounding_rect = text_surface->size().centeredIn(bounding_rect);
  text_surface->renderToScreen(text_surface->rect(), text_bounding_rect);
}
