// -*- Mode: C++; tab-width:2; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi:tw=80:et:ts=2:sts=2
//
// -----------------------------------------------------------------------
//
// This file is part of RLVM, a RealLive virtual machine clone.
//
// -----------------------------------------------------------------------
//
// Copyright (C) 2006, 2007 Elliot Glaysher
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

#include "Precompiled.hpp"

// -----------------------------------------------------------------------

#include "libReallive/defs.h"

#include "Systems/Base/TextWindow.hpp"
#include "Systems/Base/TextWindowButton.hpp"
#include "Systems/Base/TextSystem.hpp"
#include "MachineBase/RLMachine.hpp"
#include "Systems/Base/System.hpp"
#include "Systems/Base/SystemError.hpp"
#include "Systems/Base/GraphicsSystem.hpp"

#include "libReallive/gameexe.h"
#include "Utilities.h"

#include <vector>
#include <boost/bind.hpp>
#include <iomanip>

using std::cerr;
using std::endl;
using std::ostringstream;
using std::setfill;
using std::setw;
using std::vector;

// -----------------------------------------------------------------------
// TextWindow
// -----------------------------------------------------------------------

TextWindow::TextWindow(RLMachine& machine, int window_num)
  : window_num_(window_num), current_line_number_(0),
    current_indentation_in_pixels_(0), use_indentation_(0), colour_(),
    filter_(0), is_visible_(0), in_selection_mode_(0), next_id_(0)
{
  Gameexe& gexe = machine.system().gameexe();

  // POINT
  Size size = getScreenSize(gexe);
  screen_width_ = size.width();
  screen_height_ = size.height();

  // Base form for everything to follow.
  GameexeInterpretObject window(gexe("WINDOW", window_num));

  // Handle: #WINDOW.index.ATTR_MOD, #WINDOW_ATTR, #WINDOW.index.ATTR
  window_attr_mod_ = window("ATTR_MOD");
  if(window_attr_mod_ == 0)
  {
    setRGBAF(machine.system().text().windowAttr());
  }
  else
    setRGBAF(window("ATTR"));

  setFontSizeInPixels(window("MOJI_SIZE"));
  setWindowSizeInCharacters(window("MOJI_CNT"));
  setSpacingBetweenCharacters(window("MOJI_REP"));
  setRubyTextSize(window("LUBY_SIZE").to_int(0));
  setTextboxPadding(window("MOJI_POS"));

  setWindowPosition(window("POS"));

  setDefaultTextColor(gexe("COLOR_TABLE", 0));

  // INDENT_USE appears to default to on. See the first scene in the
  // game with Nagisa, paying attention to indentation; then check the
  // Gameexe.ini.
  setUseIndentation(window("INDENT_USE").to_int(1));

  setNameMod(window("NAME_MOD").to_int(0));

  setKeycurMod(window("KEYCUR_MOD"));
  setActionOnPause(window("R_COMMAND_MOD"));
}

// -----------------------------------------------------------------------

TextWindow::~TextWindow() {}

// -----------------------------------------------------------------------

void TextWindow::execute(RLMachine& machine)
{
  using namespace boost;

  if(isVisible() && ! machine.system().graphics().interfaceHidden())
  {
    for(ButtonMap::iterator it = button_map_.begin(); it != button_map_.end();
        ++it)
    {
      it->second->execute();
    }
  }
}

// -----------------------------------------------------------------------

void TextWindow::setTextboxPadding(const vector<int>& pos_data)
{
  upper_box_padding_ = pos_data.at(0);
  lower_box_padding_ = pos_data.at(1);
  left_box_padding_ = pos_data.at(2);
  right_box_padding_ = pos_data.at(3);
}

// -----------------------------------------------------------------------

void TextWindow::setDefaultTextColor(const vector<int>& color_data)
{
  default_color_ = RGBColour(color_data.at(0), color_data.at(1), color_data.at(2));
}

// -----------------------------------------------------------------------

void TextWindow::setFontColor(const vector<int>& color_data)
{
  font_colour_ = RGBColour(color_data.at(0), color_data.at(1), color_data.at(2));
}

// -----------------------------------------------------------------------

void TextWindow::setWindowSizeInCharacters(const vector<int>& pos_data)
{
  x_window_size_in_chars_ = pos_data.at(0);
  y_window_size_in_chars_ = pos_data.at(1);
}

// -----------------------------------------------------------------------

void TextWindow::setSpacingBetweenCharacters(const vector<int>& pos_data)
{
  x_spacing_ = pos_data.at(0);
  y_spacing_ = pos_data.at(1);
}

// -----------------------------------------------------------------------

void TextWindow::setWindowPosition(const vector<int>& pos_data)
{
  origin_ = pos_data.at(0);
  x_distance_from_origin_ = pos_data.at(1);
  y_distance_from_origin_ = pos_data.at(2);
}

// -----------------------------------------------------------------------

Size TextWindow::textWindowSize() const
{
  return Size((x_window_size_in_chars_ *
               (font_size_in_pixels_ + x_spacing_)) + right_box_padding_,
              (y_window_size_in_chars_ *
               (font_size_in_pixels_ + y_spacing_ + ruby_size_)) + lower_box_padding_);
}

// -----------------------------------------------------------------------

int TextWindow::boxX1() const
{
  switch(origin_)
  {
  case 0:
  case 2:
    return x_distance_from_origin_;
  case 1:
  case 3:
    return screen_width_ - x_distance_from_origin_ - textWindowSize().width() -
      left_box_padding_;
  default:
    throw SystemError("Invalid origin");
  };
}

// -----------------------------------------------------------------------

int TextWindow::boxY1() const
{
  switch(origin_)
  {
  case 0: // Top and left
  case 1: // Top and right
    return y_distance_from_origin_;
  case 2: // Bottom and left
  case 3: // Bottom and right
    return screen_height_ - y_distance_from_origin_ - textWindowSize().height() -
      upper_box_padding_;
  default:
    throw SystemError("Invalid origin");
  }
}

// -----------------------------------------------------------------------

int TextWindow::textX1() const
{
  switch(origin_)
  {
  case 0: // Top and left
  case 2: // Bottom and left
    return x_distance_from_origin_ + left_box_padding_;
  case 1: // Top and right
  case 3: // Bottom and right
    return screen_width_ - x_distance_from_origin_ - textWindowSize().width();
  default:
    throw SystemError("Invalid origin");
  };
}

// -----------------------------------------------------------------------

int TextWindow::textY1() const
{
  switch(origin_)
  {
  case 0: // Top and left
  case 1: // Top and right
    return y_distance_from_origin_ + upper_box_padding_;
  case 2: // Bottom and left
  case 3: // Bottom and right
    return screen_height_ - y_distance_from_origin_ - textWindowSize().height();
  default:
    throw SystemError("Invalid origin");
  }
}

// -----------------------------------------------------------------------

int TextWindow::textX2() const
{
  return textX1() + textWindowSize().width();
}

// -----------------------------------------------------------------------

int TextWindow::textY2() const
{
  return textY1() + textWindowSize().height();
}

// -----------------------------------------------------------------------

void TextWindow::setKeycurMod(const vector<int>& keycur)
{
  keycursor_type_ = keycur.at(0);
  keycursor_pos_ = Point(keycur.at(1), keycur.at(2));
}

// -----------------------------------------------------------------------

Point TextWindow::keycursorPosition() const
{
  switch(keycursor_type_)
  {
  case 0:
    return Point(textX2(), textY2());
  case 1:
    return Point(text_insertion_point_x_, text_insertion_point_y_);
  case 2:
    return Point(textX1(), textY1()) + keycursor_pos_;
  default:
    throw SystemError("Invalid keycursor type");
  }
}

// -----------------------------------------------------------------------

void readjumpTurnsOn()
{
  cerr << "Readjump (falsly) turn on!" << endl;
}

void readjumpTurnsOff()
{
  cerr << "Readjump (falsly) turn off!" << endl;
}

void automodeTurnsOn()
{
  cerr << "Automode (falsly) turn on!" << endl;
}

void automodeTurnsOff()
{
  cerr << "Automode (falsly) turn off!" << endl;
}

// -----------------------------------------------------------------------


void TextWindow::setWindowWaku(RLMachine& machine, Gameexe& gexe,
                               const int waku_no)
{
  using namespace boost;

  GameexeInterpretObject waku(gexe("WAKU", waku_no, 0));

  setWakuMain(machine, waku("NAME").to_string(""));
  setWakuBacking(machine, waku("BACK").to_string(""));
  setWakuButton(machine, waku("BTN").to_string(""));

  TextSystem& ts = machine.system().text();
  GraphicsSystem& gs = machine.system().graphics();
  button_map_.clear();
  // Translation: Boost ptr_map is retarded.
  string key = "MOVE_BOX";
  button_map_.insert(key,
                     new TextWindowButton(ts.windowMoveUse(), waku("MOVE_BOX")));
  key = string("CLEAR_BOX");
  button_map_.insert(key,
                     new ActionTextWindowButton(
                       ts.windowClearUse(), waku("CLEAR_BOX"),
                       bind(&GraphicsSystem::toggleInterfaceHidden, ref(gs))));
  key = string("READJUMP_BOX");
  button_map_.insert(key,
                     new ActivationTextWindowButton(
                       ts.windowReadJumpUse(), waku("READJUMP_BOX"),
                       &readjumpTurnsOn, &readjumpTurnsOff));
  key = string("AUTOMODE_BOX");
  button_map_.insert(key,
                     new ActivationTextWindowButton(
                       ts.windowAutomodeUse(), waku("AUTOMODE_BOX"),
                       bind(&TextSystem::setAutoMode, ref(ts), true),
                       bind(&TextSystem::setAutoMode, ref(ts), false)));

  key = string("MSGBK_BOX");
  button_map_.insert(key,
                     new TextWindowButton(ts.windowMsgbkUse(), waku("MSGBK_BOX")));
  key = string("MSGBKLEFT_BOX");
  button_map_.insert(key,
                     new RepeatActionWhileHoldingWindowButton(
                       ts.windowMsgbkleftUse(),
                       waku("MSGBKLEFT_BOX"),
                       machine,
                       bind(&TextSystem::backPage, ref(ts), ref(machine)),
                       250));
  key = string("MSGBKRIGHT_BOX");
  button_map_.insert(key,
                     new RepeatActionWhileHoldingWindowButton(
                       ts.windowMsgbkrightUse(),
                       waku("MSGBKRIGHT_BOX"),
                       machine,
                       bind(&TextSystem::forwardPage, ref(ts), ref(machine)),
                       250));

  // Read the seven EXBTNs.
  for(int i = 0; i < 7; ++i)
  {
    GameexeInterpretObject wbcall(gexe("WBCALL", i));
    ostringstream oss;
    oss << "EXBTN_" << setw(3) << setfill('0') << i << "_BOX";
    key = oss.str();
    button_map_.insert(key,
                       new ExbtnWindowButton(machine,
                         ts.windowExbtnUse(),
                         waku(oss.str()),
                         wbcall));
  }
}

// -----------------------------------------------------------------------

void TextWindow::setRGBAF(const vector<int>& attr)
{
  colour_ = RGBAColour(attr.at(0), attr.at(1), attr.at(2), attr.at(3));
  setFilter(attr.at(4));
}

// -----------------------------------------------------------------------

void TextWindow::setMousePosition(RLMachine& machine, const Point& pos)
{
  using namespace boost;

  for(ButtonMap::iterator it = button_map_.begin(); it != button_map_.end();
      ++it)
  {
    it->second->setMousePosition(machine, *this, pos);
  }
}

// -----------------------------------------------------------------------

bool TextWindow::handleMouseClick(RLMachine& machine, const Point& pos,
                                  bool pressed)
{
  using namespace boost;

  if(isVisible() && ! machine.system().graphics().interfaceHidden())
  {
    for(ButtonMap::iterator it = button_map_.begin(); it != button_map_.end();
        ++it)
    {
      if(it->second->handleMouseClick(machine, *this, pos, pressed))
        return true;
    }
  }

  return false;
}

// -----------------------------------------------------------------------

void TextWindow::startSelectionMode()
{
  in_selection_mode_ = true;
  next_id_ = 0;
}

// -----------------------------------------------------------------------

void TextWindow::setSelectionCallback(const boost::function<void(int)>& in)
{
  selection_callback_ = in;
}

// -----------------------------------------------------------------------

void TextWindow::endSelectionMode()
{
  in_selection_mode_ = false;
  selection_callback_.clear();
}

// -----------------------------------------------------------------------

const boost::function<void(int)>& TextWindow::selectionCallback()
{
  return selection_callback_;
}
