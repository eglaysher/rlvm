// -*- Mode: C++; tab-width:2; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi:tw=80:et:ts=2:sts=2
//
// -----------------------------------------------------------------------
//
// This file is part of RLVM, a RealLive virtual machine clone.
//
// -----------------------------------------------------------------------
//
// Copyright (C) 2007 Elliot Glaysher
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

#include "NullTextWindow.hpp"

// -----------------------------------------------------------------------

NullTextWindow::NullTextWindow(System& system, int x)
    : TextWindow(system, x), text_window_log_("NullTextWindow") {
}

// -----------------------------------------------------------------------

NullTextWindow::~NullTextWindow() {}

// -----------------------------------------------------------------------

void NullTextWindow::clearWin()
{
  text_window_log_.recordFunction("clearWin");
  TextWindow::clearWin();
  current_contents_ = "";
}

// -----------------------------------------------------------------------

void NullTextWindow::setFontColor(const std::vector<int>& color_data) {
  text_window_log_.recordFunction("setFontColor");
  TextWindow::setFontColor(color_data);
}

// -----------------------------------------------------------------------

bool NullTextWindow::displayChar(const std::string& current,
                                 const std::string& next)
{
  text_window_log_.recordFunction("displayChar", current, next);

  current_contents_ += current;
  return true;
}

// -----------------------------------------------------------------------

void NullTextWindow::setName(const std::string& utf8name,
                             const std::string& next_char)
{
  text_window_log_.recordFunction("setName", utf8name, next_char);

  current_contents_ += "\\{" + utf8name + "}";
}

// -----------------------------------------------------------------------

void NullTextWindow::hardBrake()
{
  text_window_log_.recordFunction("hardBrake");
  current_contents_ += "\n";
  TextWindow::hardBrake();
}

// -----------------------------------------------------------------------

void NullTextWindow::resetIndentation() {
  text_window_log_.recordFunction("resetIndentation");
  TextWindow::resetIndentation();
}

// -----------------------------------------------------------------------

void NullTextWindow::markRubyBegin() {
  text_window_log_.recordFunction("markRubyBegin");
  TextWindow::markRubyBegin();
}

// -----------------------------------------------------------------------

void NullTextWindow::displayRubyText(const std::string& utf8str) {
  text_window_log_.recordFunction("displayRubyText", utf8str);
}
