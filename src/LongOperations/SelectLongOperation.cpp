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

#include "LongOperations/SelectLongOperation.hpp"

#include <iostream>
#include <vector>
#include <string>
#include <boost/bind.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/shared_ptr.hpp>

#include "MachineBase/LongOperation.hpp"
#include "MachineBase/RLMachine.hpp"
#include "Systems/Base/EventListener.hpp"
#include "Systems/Base/EventSystem.hpp"
#include "Systems/Base/GraphicsSystem.hpp"
#include "Systems/Base/Renderable.hpp"
#include "Systems/Base/SoundSystem.hpp"
#include "Systems/Base/System.hpp"
#include "Systems/Base/TextSystem.hpp"
#include "Systems/Base/TextWindow.hpp"
#include "Utilities/StringUtilities.hpp"
#include "libReallive/bytecode.h"
#include "libReallive/expression.h"
#include "libReallive/gameexe.h"

using boost::bind;
using boost::scoped_ptr;
using boost::shared_ptr;
using std::cerr;
using std::endl;
using std::string;
using std::vector;
using std::distance;
using libReallive::SelectElement;
using libReallive::ExpressionPiece;
using libReallive::CommandElement;

// -----------------------------------------------------------------------
// SelectLongOperation
// -----------------------------------------------------------------------
SelectLongOperation::SelectLongOperation(RLMachine& machine,
                                         const SelectElement& commandElement)
    : machine_(machine),
      return_value_(-1) {
  const vector<SelectElement::Param>& params = commandElement.getRawParams();
  for (unsigned int i = 0; i < params.size(); ++i) {
    Option o;
    o.shown = true;
    o.enabled = true;
    o.use_colour = false;

    std::string evaluated_native =
        libReallive::evaluatePRINT(machine, params[i].text);
    o.str = cp932toUTF8(evaluated_native, machine.getTextEncoding());

    std::vector<SelectElement::Condition> conditions = params[i].cond_parsed;
    for (std::vector<SelectElement::Condition>::const_iterator it =
             conditions.begin(); it != conditions.end(); ++it) {
      switch (it->effect) {
        // TODO(erg): Someday, I might need to support the other options, but
        // for now, I've never seen anything other than hide.
        case SelectElement::OPTION_HIDE: {
          bool value = false;
          if (it->condition != "") {
            const char* location = it->condition.c_str();
            scoped_ptr<ExpressionPiece> condition(
              libReallive::get_expression(location));
            value = !condition->integerValue(machine);
          }

          o.shown = value;
          break;
        }
        case SelectElement::OPTION_TITLE: {
          bool enabled = false;
          if (it->condition != "") {
            const char* location = it->condition.c_str();
            scoped_ptr<ExpressionPiece> condition(
              libReallive::get_expression(location));
            enabled = !condition->integerValue(machine);
          }

          bool use_colour = false;
          int colour_index = 0;
          if (!enabled && it->effect_argument != "") {
            const char* location = it->effect_argument.c_str();
            scoped_ptr<ExpressionPiece> effect_argument(
              libReallive::get_expression(location));
            colour_index = !effect_argument->integerValue(machine);
            use_colour = true;
          }

          o.enabled = enabled;
          o.use_colour = use_colour;
          o.colour_index = colour_index;
          break;
        }
        default:
          cerr << "Unsupported option in select statement "
               << "(condition: "
               << libReallive::parsableToPrintableString(it->condition)
               << ", effect: " << it->effect << ", effect_argument: "
               << libReallive::parsableToPrintableString(it->effect_argument)
               << ")" << endl;
          break;
      }
    }

    options_.push_back(o);
  }
}

void SelectLongOperation::selected(int num) {
  if (machine_.system().sound().hasSe(1))
    machine_.system().sound().playSe(1);
  machine_.system().takeSelectionSnapshot(machine_);
  return_value_ = num;
}

bool SelectLongOperation::selectOption(const std::string& str) {
  std::vector<Option>::iterator it =
      find_if(options_.begin(), options_.end(), bind(&Option::str, _1) == str);

  if (it != options_.end() && it->shown) {
    selected(distance(options_.begin(), it));
    return true;
  }

  return false;
}

std::vector<std::string> SelectLongOperation::options() const {
  std::vector<std::string> opt;
  for (size_t i = 0; i < options_.size(); i++) {
    opt.push_back(options_[i].str);
  }

  return opt;
}

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
      text_window_(machine.system().text().currentWindow()) {
  machine.system().text().setInSelectionMode(true);
  text_window_->setVisible(true);
  text_window_->startSelectionMode();
  text_window_->setSelectionCallback(
    bind(&NormalSelectLongOperation::selected, this, _1));

  for (size_t i = 0; i < options_.size(); ++i) {
    // TODO(erg): Also deal with colour.
    if (options_[i].shown) {
      if (options_[i].use_colour == true || options_[i].enabled == false) {
        cerr << "We don't deal with color/enabled state in normal selections..."
             << endl;
      }

      text_window_->addSelectionItem(options_[i].str, i);
    }
  }

  machine.system().graphics().markScreenAsDirty(GUT_TEXTSYS);
}

NormalSelectLongOperation::~NormalSelectLongOperation() {
  text_window_->endSelectionMode();
  machine_.system().text().setInSelectionMode(false);
}

void NormalSelectLongOperation::mouseMotion(const Point& pos) {
  // Tell the text system about the move
  machine_.system().text().setMousePosition(pos);
}

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
      highlighted_item_(-1),
      normal_frame_(0),
      select_frame_(0),
      push_frame_(0),
      dontsel_frame_(0),
      mouse_down_(false) {
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
  if (select_colour_num_ == -1) // For little busters
    select_colour_num_ = default_colour_num_;

  GraphicsSystem& gs = machine.system().graphics();
  if (selbtn("NAME").exists() && selbtn("NAME").to_string() != "")
    name_surface_ = gs.getSurfaceNamed(selbtn("NAME"));
  if (selbtn("BACK").exists() && selbtn("BACK").to_string() != "")
    back_surface_ = gs.getSurfaceNamed(selbtn("BACK"));

  std::vector<int> tmp;
  if (selbtn("NORMAL").exists()) {
    tmp = selbtn("NORMAL");
    normal_frame_ = tmp.at(0);
    normal_frame_offset_ = Point(tmp.at(1), tmp.at(2));
  }
  if (selbtn("SELECT").exists()) {
    tmp = selbtn("SELECT");
    select_frame_ = tmp.at(0);
    select_frame_offset_ = Point(tmp.at(1), tmp.at(2));
  }
  if (selbtn("PUSH").exists()) {
    tmp = selbtn("PUSH");
    push_frame_ = tmp.at(0);
    push_frame_offset_ = Point(tmp.at(1), tmp.at(2));
  }
  if (selbtn("DONTSEL").exists()) {
    tmp = selbtn("DONTSEL");
    dontsel_frame_ = tmp.at(0);
    dontsel_frame_offset_ = Point(tmp.at(1), tmp.at(2));
  }

  // Pick the correct font colour
  vec = gexe("COLOR_TABLE", default_colour_num_);
  RGBColour default_colour(vec.at(0), vec.at(1), vec.at(2));
  vec = gexe("COLOR_TABLE", select_colour_num_);
  RGBColour select_colour(vec.at(0), vec.at(1), vec.at(2));
  vec = gexe("COLOR_TABLE", 255);
  RGBColour shadow_colour(vec.at(0), vec.at(1), vec.at(2));

  // Build graphic representations of the choices to display to the user.
  TextSystem& ts = machine.system().text();
  int shown_option_count = std::count_if(options_.begin(), options_.end(),
                                         bind(&Option::shown, _1));

  // Calculate out the bounding rectangles for all the options.
  Size screen_size = machine.system().graphics().screenSize();
  int baseposx = 0;
  if (center_x) {
    int totalwidth = ((shown_option_count - 1) * reppos_x_);
    if (back_surface_)
      totalwidth += back_surface_->size().width();
    else
      totalwidth += name_surface_->getPattern(normal_frame_).rect.width();
    baseposx = (screen_size.width() / 2) - (totalwidth / 2);
  } else {
    baseposx = basepos_x_;
  }

  int baseposy = 0;
  if (center_y) {
    int totalheight = ((shown_option_count - 1) * reppos_y_);
    if (back_surface_)
      totalheight += back_surface_->size().height();
    baseposy = (screen_size.height() / 2) - (totalheight / 2);
  } else {
    baseposy = basepos_y_;
  }

  for (size_t i = 0; i < options_.size(); i++) {
    if (options_[i].shown) {
      const std::string& text = options_[i].str;

      RGBColour text_colour = default_colour;
      RGBColour text_selection_colour = select_colour;

      if (options_[i].use_colour) {
        std::vector<int> vec = gexe("COLOR_TABLE", options_[i].colour_index);
        text_colour = RGBColour(vec.at(0), vec.at(1), vec.at(2));

        if (!options_[i].enabled)
          text_selection_colour = text_colour;
      }

      ButtonOption o;
      o.id = i;
      o.enabled = options_[i].enabled;
      o.default_surface = ts.renderText(
          text, moji_size_, 0, 0, text_colour, &shadow_colour, 0);
      o.select_surface = ts.renderText(
          text, moji_size_, 0, 0, text_selection_colour, &shadow_colour, 0);
      if (back_surface_) {
        o.bounding_rect = Rect(baseposx, baseposy, back_surface_->size());
      } else {
        o.bounding_rect = Rect(baseposx, baseposy,
                               name_surface_->getPattern(0).rect.size());
      }

      buttons_.push_back(o);

      baseposx += reppos_x_;
      baseposy += reppos_y_;
    }
  }

  machine.system().graphics().markScreenAsDirty(GUT_TEXTSYS);
}

ButtonSelectLongOperation::~ButtonSelectLongOperation() {
  machine_.system().graphics().removeRenderable(this);
}

void ButtonSelectLongOperation::mouseMotion(const Point& p) {
  for (size_t i = 0; i < buttons_.size(); i++) {
    if (buttons_[i].bounding_rect.contains(p)) {
      if (options_[i].enabled) {
        if (highlighted_item_ != i &&
            machine_.system().sound().hasSe(0)) {
          machine_.system().sound().playSe(0);
        }

        highlighted_item_ = i;
      }
      return;
    }
  }

  highlighted_item_ = -1;
}

bool ButtonSelectLongOperation::mouseButtonStateChanged(
    MouseButton mouseButton, bool pressed) {
  EventSystem& es = machine_.system().event();

  switch (mouseButton) {
    case MOUSE_LEFT: {
      mouse_down_ = pressed;
      if (!pressed) {
        Point pos = es.getCursorPos();
        for (size_t i = 0; i < buttons_.size(); i++) {
          if (buttons_[i].bounding_rect.contains(pos) &&
              options_[i].enabled) {
            selected(buttons_[i].id);
            break;
          }
        }

        return true;
      }
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

void ButtonSelectLongOperation::render(std::ostream* tree) {
  for (size_t i = 0; i < buttons_.size(); i++) {
    int frame = normal_frame_;
    Point offset = normal_frame_offset_;
    if (!buttons_[i].enabled) {
      offset = dontsel_frame_offset_;
      frame = dontsel_frame_;
    } else if (i == highlighted_item_) {
      if (mouse_down_) {
        offset = push_frame_offset_;
        frame = push_frame_;
      } else {
        offset = select_frame_offset_;
        frame = select_frame_;
      }
    }

    Rect bounding_rect = buttons_[i].bounding_rect;
    bounding_rect = Rect(bounding_rect.origin() + offset,
                         bounding_rect.size());

    if (back_surface_) {
      back_surface_->renderToScreenAsColorMask(
          back_surface_->rect(), bounding_rect, window_bg_colour_,
          window_filter_);
    }
    if (name_surface_) {
      name_surface_->renderToScreen(
          name_surface_->getPattern(frame).rect, bounding_rect);
    }

    if (i == highlighted_item_) {
      renderTextSurface(buttons_[i].select_surface, bounding_rect);
    } else {
      renderTextSurface(buttons_[i].default_surface, bounding_rect);
    }
  }
}

void ButtonSelectLongOperation::renderTextSurface(
    const boost::shared_ptr<Surface>& text_surface, const Rect& bounding_rect) {
  // Render the correct text in the correct place.
  Rect text_bounding_rect = text_surface->size().centeredIn(bounding_rect);
  text_surface->renderToScreen(text_surface->rect(), text_bounding_rect);
}
